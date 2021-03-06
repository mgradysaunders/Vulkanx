#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <vulkanx/command_buffer.h>
#include <vulkanx/result.h>
#include <vulkanx/buffer.h>

// Create buffer.
VkResult vkxCreateBuffer(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    const VkBufferCreateInfo* pBufferCreateInfo,
    const VkMemoryPropertyFlags memoryPropertyFlags,
    const VkAllocationCallbacks* pAllocator,
    VkxBuffer* pBuffer) {
    assert(pBufferCreateInfo);
    assert(pBuffer);

    // Nullify.
    pBuffer->buffer = VK_NULL_HANDLE;
    pBuffer->memory = VK_NULL_HANDLE;

    {
        // Create buffer.
        VkResult result = vkCreateBuffer(
            device, pBufferCreateInfo, pAllocator, &pBuffer->buffer);
        if (VKX_IS_ERROR(result)) {
            pBuffer->buffer = VK_NULL_HANDLE;
            return result;
        }
    }

    {
        // Get memory requirements.
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(
            device, pBuffer->buffer, &memoryRequirements);
        pBuffer->memorySize = memoryRequirements.size;

        // Find memory type index.
        uint32_t memoryTypeIndex = vkxFindMemoryTypeIndex(
            physicalDevice, memoryPropertyFlags,
            memoryRequirements.memoryTypeBits);
        if (memoryTypeIndex == UINT32_MAX) {
            // Destroy buffer.
            vkxDestroyBuffer(device, pBuffer, pAllocator);
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        // Memory allocate info.
        VkMemoryAllocateInfo memoryAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = NULL,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = memoryTypeIndex};

        // Allocate memory.
        VkResult result = vkAllocateMemory(
            device, &memoryAllocateInfo, pAllocator, &pBuffer->memory);
        if (VKX_IS_ERROR(result)) {
            // Destroy buffer.
            pBuffer->memory = VK_NULL_HANDLE;
            vkxDestroyBuffer(device, pBuffer, pAllocator);
            return result;
        }
    }

    // Bind memory.
    VkResult result =
        vkBindBufferMemory(device, pBuffer->buffer, pBuffer->memory, 0);
    if (VKX_IS_ERROR(result)) {
        // Destroy buffer.
        vkxDestroyBuffer(device, pBuffer, pAllocator);
        // Fall through.
    }
    return result;
}

// Destroy buffer.
void vkxDestroyBuffer(
    VkDevice device,
    VkxBuffer* pBuffer,
    const VkAllocationCallbacks* pAllocator) {
    if (pBuffer) {
        // Destroy buffer.
        vkDestroyBuffer(device, pBuffer->buffer, pAllocator);

        // Free memory.
        vkFreeMemory(device, pBuffer->memory, pAllocator);

        // Nullify.
        pBuffer->buffer = VK_NULL_HANDLE;
        pBuffer->memory = VK_NULL_HANDLE;
    }
}

// Create buffer group.
VkResult vkxCreateBufferGroup(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    uint32_t bufferCount,
    const VkBufferCreateInfo* pBufferCreateInfos,
    const VkMemoryPropertyFlags* pMemoryPropertyFlags,
    const VkAllocationCallbacks* pAllocator,
    VkxBufferGroup* pBufferGroup) {
    assert(pBufferGroup);
    memset(pBufferGroup, 0, sizeof(VkxBufferGroup));
    if (bufferCount == 0) {
        return VK_SUCCESS;
    }

    // Sanity check.
    assert(pBufferCreateInfos);
    assert(pMemoryPropertyFlags);

    // Allocate buffers.
    pBufferGroup->bufferCount = bufferCount;
    pBufferGroup->pBuffers = (VkBuffer*)malloc(sizeof(VkBuffer) * bufferCount);
    VkBuffer* pBuffers = pBufferGroup->pBuffers;
    for (uint32_t bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++) {
        pBuffers[bufferIndex] = VK_NULL_HANDLE;
    }

    // Allocate memory requirements.
    VkMemoryRequirements* pMemoryRequirements =
        (VkMemoryRequirements*)VKX_LOCAL_MALLOC(
            sizeof(VkMemoryRequirements) * bufferCount);

    // Iterate buffers.
    for (uint32_t bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++) {
        // Create buffer.
        VkResult result = vkCreateBuffer(
            device, &pBufferCreateInfos[bufferIndex], pAllocator,
            &pBuffers[bufferIndex]);
        if (VKX_IS_ERROR(result)) {
            // Free memory requirements.
            VKX_LOCAL_FREE(pMemoryRequirements);
            // Destroy buffer group.
            pBuffers[bufferIndex] = VK_NULL_HANDLE;
            vkxDestroyBufferGroup(device, pBufferGroup, pAllocator);
            return result;
        }

        // Get buffer memory requirements.
        vkGetBufferMemoryRequirements(
            device, pBuffers[bufferIndex], &pMemoryRequirements[bufferIndex]);
    }

    {
        // Allocate shared memory.
        VkResult result = vkxAllocateSharedMemory(
            physicalDevice, device, bufferCount, pMemoryRequirements,
            pMemoryPropertyFlags, pAllocator, &pBufferGroup->sharedMemory);
        if (VKX_IS_ERROR(result)) {
            // Free memory requirements.
            VKX_LOCAL_FREE(pMemoryRequirements);
            // Destroy buffer group.
            vkxDestroyBufferGroup(device, pBufferGroup, pAllocator);
            return result;
        }
    }

    // Bind memory.
    VkxDeviceMemoryView* pMemoryViews =
        pBufferGroup->sharedMemory.pMemoryViews;
    for (uint32_t bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++) {
        VkResult result = vkBindBufferMemory(
            device, pBuffers[bufferIndex], pMemoryViews[bufferIndex].memory,
            pMemoryViews[bufferIndex].offset);
        if (VKX_IS_ERROR(result)) {
            // Free memory requirements.
            VKX_LOCAL_FREE(pMemoryRequirements);
            // Destroy buffer group.
            vkxDestroyBufferGroup(device, pBufferGroup, pAllocator);
            return result;
        }
    }

    // Free memory requirements.
    VKX_LOCAL_FREE(pMemoryRequirements);

    return VK_SUCCESS;
}

// Destroy buffer group.
void vkxDestroyBufferGroup(
    VkDevice device,
    VkxBufferGroup* pBufferGroup,
    const VkAllocationCallbacks* pAllocator) {
    if (pBufferGroup) {
        for (uint32_t bufferIndex = 0; bufferIndex < pBufferGroup->bufferCount;
             bufferIndex++) {
            // Destroy buffer.
            vkDestroyBuffer(
                device, pBufferGroup->pBuffers[bufferIndex], pAllocator);
        }
        // Free buffers.
        free(pBufferGroup->pBuffers);

        // Nullify.
        pBufferGroup->pBuffers = NULL;
        pBufferGroup->bufferCount = 0;

        // Delegate.
        vkxFreeSharedMemory(device, &pBufferGroup->sharedMemory, pAllocator);
    }
}

// Copy buffer.
VkResult vkxCopyBuffer(
    VkDevice device,
    VkQueue queue,
    VkCommandPool commandPool,
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    uint32_t regionCount,
    const VkBufferCopy* pRegions,
    const VkAllocationCallbacks* pAllocator) {
    if (srcBuffer == dstBuffer || regionCount == 0) {
        return VK_SUCCESS;
    }
    // Sanity check.
    assert(pRegions);

    // Command buffer allocate info.
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = commandPool,
        .commandBufferCount = 1};

    // Command buffer begin info.
    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = NULL};

    // Command buffer.
    VkCommandBuffer commandBuffer;
    {
        // Allocate and begin command buffer.
        VkResult result = vkxAllocateAndBeginCommandBuffers(
            device, &commandBufferAllocateInfo, &commandBufferBeginInfo,
            &commandBuffer);
        // Allocate and begin command buffer error?
        if (VKX_IS_ERROR(result)) {
            return result;
        }
    }

    // Copy buffer.
    vkCmdCopyBuffer(
        commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);

    // End, flush, and free command buffers.
    return vkxEndFlushAndFreeCommandBuffers(
        device, queue, commandPool, 1, &commandBuffer, pAllocator);
}

// Get buffer data.
VkResult vkxGetBufferData(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkQueue queue,
    VkCommandPool commandPool,
    VkBuffer buffer,
    const VkxBufferDataAccess* pBufferDataAccess,
    const VkAllocationCallbacks* pAllocator,
    void* pData) {
    assert(pBufferDataAccess);
    assert(pData || pBufferDataAccess->size == 0);
    if (pBufferDataAccess->size == 0) {
        return VK_SUCCESS;
    }

    VkxBuffer stagingBuffer;
    {
        // Staging buffer create info.
        VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .size = pBufferDataAccess->size,
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = NULL};
        // Create staging buffer.
        VkResult result = vkxCreateBuffer(
            physicalDevice, device, &bufferCreateInfo,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            pAllocator, &stagingBuffer);
        // Create staging buffer error?
        if (VKX_IS_ERROR(result)) {
            return result;
        }
    }

    {
        // Buffer copy region.
        VkBufferCopy region = {
            .srcOffset = pBufferDataAccess->offset,
            .dstOffset = 0,
            .size = pBufferDataAccess->size};
        // Copy.
        VkResult result = vkxCopyBuffer(
            device, queue, commandPool, buffer, stagingBuffer.buffer, 1,
            &region, pAllocator);
        // Copy error?
        if (VKX_IS_ERROR(result)) {
            // Destroy staging buffer.
            vkxDestroyBuffer(device, &stagingBuffer, pAllocator);
            return result;
        }
    }

    // Map staging buffer data.
    void* pStagingBufferData = NULL;
    VkResult result = vkMapMemory(
        device, stagingBuffer.memory, 0, pBufferDataAccess->size, 0,
        &pStagingBufferData);
    // Map staging buffer data ok?
    if (VKX_IS_OK(result)) {
        // Read staging buffer data.
        memcpy(pData, pStagingBufferData, pBufferDataAccess->size);
        // Unmap staging buffer data.
        vkUnmapMemory(device, stagingBuffer.memory);
    }

    // Destroy staging buffer.
    vkxDestroyBuffer(device, &stagingBuffer, pAllocator);

    return result;
}

// Set buffer data.
VkResult vkxSetBufferData(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkQueue queue,
    VkCommandPool commandPool,
    VkBuffer buffer,
    const VkxBufferDataAccess* pBufferDataAccess,
    const void* pData,
    const VkAllocationCallbacks* pAllocator) {
    assert(pBufferDataAccess);
    assert(pData || pBufferDataAccess->size == 0);
    if (pBufferDataAccess->size == 0) {
        return VK_SUCCESS;
    }

    // Staging buffer.
    VkxBuffer stagingBuffer;
    {
        // Staging buffer create info.
        VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .size = pBufferDataAccess->size,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = NULL};
        // Create staging buffer.
        VkResult result = vkxCreateBuffer(
            physicalDevice, device, &bufferCreateInfo,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            pAllocator, &stagingBuffer);
        // Create staging buffer error?
        if (VKX_IS_ERROR(result)) {
            return result;
        }
    }

    {
        // Map staging buffer data.
        void* pStagingBufferData = NULL;
        VkResult result = vkMapMemory(
            device, stagingBuffer.memory, 0, pBufferDataAccess->size, 0,
            &pStagingBufferData);
        // Map staging buffer data error?
        if (VKX_IS_ERROR(result)) {
            // Destroy staging buffer.
            vkxDestroyBuffer(device, &stagingBuffer, pAllocator);
            return result;
        }

        // Write staging buffer data.
        memcpy(pStagingBufferData, pData, pBufferDataAccess->size);

        // Unmap staging buffer data.
        vkUnmapMemory(device, stagingBuffer.memory);
    }

    // Copy.
    VkBufferCopy region = {
        .srcOffset = 0,
        .dstOffset = pBufferDataAccess->offset,
        .size = pBufferDataAccess->size};
    VkResult result = vkxCopyBuffer(
        device, queue, commandPool, stagingBuffer.buffer, buffer, 1, &region,
        pAllocator);

    // Destroy staging buffer.
    vkxDestroyBuffer(device, &stagingBuffer, pAllocator);

    return result;
}
