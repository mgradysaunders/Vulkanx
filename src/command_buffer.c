#include <assert.h>
#include <string.h>
#include <vulkanx/result.h>
#include <vulkanx/command_buffer.h>

VkResult vkxCreateFences(
    VkDevice device,
    uint32_t fenceCount,
    VkBool32 fenceSignaled,
    const VkAllocationCallbacks* pAllocator,
    VkFence* pFences) {
    if (fenceCount == 0) return VK_SUCCESS;

    assert(pFences);
    memset(pFences, 0, sizeof(VkFence) * fenceCount);

    // Fence create info.
    VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = NULL,
        .flags = fenceSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0};

    VkResult result = VK_SUCCESS;
    for (uint32_t fenceIndex = 0; fenceIndex < fenceCount; fenceIndex++) {
        // Create fence.
        result = vkCreateFence(
            device, &fenceCreateInfo, pAllocator, &pFences[fenceIndex]);
        if (VKX_IS_ERROR(result)) {
            pFences[fenceIndex] = VK_NULL_HANDLE;
            vkxDestroyFences(device, fenceCount, pFences, pAllocator);
            break;
        }
    }

    return result;
}

void vkxDestroyFences(
    VkDevice device,
    uint32_t fenceCount,
    VkFence* pFences,
    const VkAllocationCallbacks* pAllocator) {
    if (pFences) {
        // Destroy.
        for (uint32_t fenceIndex = 0; fenceIndex < fenceCount; fenceIndex++)
            vkDestroyFence(device, pFences[fenceIndex], pAllocator);
        // Nullify.
        memset(pFences, 0, sizeof(VkFence) * fenceCount);
    }
}

VkResult vkxCreateSemaphores(
    VkDevice device,
    uint32_t semaphoreCount,
    const VkAllocationCallbacks* pAllocator,
    VkSemaphore* pSemaphores) {
    if (semaphoreCount == 0) return VK_SUCCESS;

    assert(pSemaphores);
    memset(pSemaphores, 0, sizeof(VkSemaphore) * semaphoreCount);

    // Semaphore create info.
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0};

    VkResult result = VK_SUCCESS;
    for (uint32_t semaphoreIndex = 0; semaphoreIndex < semaphoreCount;
         semaphoreIndex++) {
        // Create semaphore.
        result = vkCreateSemaphore(
            device, &semaphoreCreateInfo, pAllocator,
            &pSemaphores[semaphoreIndex]);
        if (VKX_IS_ERROR(result)) {
            pSemaphores[semaphoreIndex] = VK_NULL_HANDLE;
            vkxDestroySemaphores(
                device, semaphoreCount, pSemaphores, pAllocator);
            break;
        }
    }
    return result;
}

void vkxDestroySemaphores(
    VkDevice device,
    uint32_t semaphoreCount,
    VkSemaphore* pSemaphores,
    const VkAllocationCallbacks* pAllocator) {
    if (pSemaphores) {
        // Destroy.
        for (uint32_t semaphoreIndex = 0; semaphoreIndex < semaphoreCount;
             semaphoreIndex++)
            vkDestroySemaphore(
                device, pSemaphores[semaphoreIndex], pAllocator);
        // Nullify.
        memset(pSemaphores, 0, sizeof(VkSemaphore) * semaphoreCount);
    }
}

VkResult vkxAllocateAndBeginCommandBuffers(
    VkDevice device,
    const VkCommandBufferAllocateInfo* pAllocateInfo,
    const VkCommandBufferBeginInfo* pBeginInfos,
    VkCommandBuffer* pCommandBuffers) {
    assert(pAllocateInfo);
    if (pAllocateInfo->commandBufferCount == 0) {
        return VK_SUCCESS;
    }

    // Sanity check.
    assert(pBeginInfos && pCommandBuffers);

    VkResult result;
    // Allocate command buffers.
    result = vkAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    // Allocate command buffers error?
    if (VKX_IS_ERROR(result)) {
        // Failure.
        goto FAILURE;
    }

    for (uint32_t commandBufferIndex = 0;
         commandBufferIndex < pAllocateInfo->commandBufferCount;
         commandBufferIndex++) {
        // Begin command buffer.
        result = vkBeginCommandBuffer(
            pCommandBuffers[commandBufferIndex],
            &pBeginInfos[commandBufferIndex]);
        // Begin command buffer error?
        if (VKX_IS_ERROR(result)) {
            // Free command buffers.
            vkFreeCommandBuffers(
                device, pAllocateInfo->commandPool,
                pAllocateInfo->commandBufferCount, pCommandBuffers);
            // Failure.
            goto FAILURE;
        }
    }

    // Success.
    return VK_SUCCESS;

FAILURE:

    // Nullify.
    for (uint32_t commandBufferIndex = 0;
         commandBufferIndex < pAllocateInfo->commandBufferCount;
         commandBufferIndex++) {
        pCommandBuffers[commandBufferIndex] = VK_NULL_HANDLE;
    }
    return result;
}

VkResult vkxFlushCommandBuffers(
    VkDevice device,
    VkQueue queue,
    uint32_t commandBufferCount,
    const VkCommandBuffer* pCommandBuffers,
    const VkAllocationCallbacks* pAllocator) {
    VkFence fence = VK_NULL_HANDLE;

    {
        // Create fence.
        VkFenceCreateInfo fenceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0};
        VkResult result =
            vkCreateFence(device, &fenceCreateInfo, pAllocator, &fence);
        if (VKX_IS_ERROR(result)) {
            return result;
        }
    }

    {
        // Submit.
        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = NULL,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = NULL,
            .pWaitDstStageMask = NULL,
            .commandBufferCount = commandBufferCount,
            .pCommandBuffers = pCommandBuffers,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = NULL};
        VkResult result = vkQueueSubmit(queue, 1, &submitInfo, fence);
        if (VKX_IS_ERROR(result)) {
            // Destroy fence.
            vkDestroyFence(device, fence, pAllocator);
            return result;
        }
    }

    // Wait.
    VkResult result = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

    // Destroy fence.
    vkDestroyFence(device, fence, pAllocator);

    return result;
}

VkResult vkxEndFlushAndFreeCommandBuffers(
    VkDevice device,
    VkQueue queue,
    VkCommandPool commandPool,
    uint32_t commandBufferCount,
    const VkCommandBuffer* pCommandBuffers,
    const VkAllocationCallbacks* pAllocator) {
    if (commandBufferCount == 0) return VK_SUCCESS;

    assert(pCommandBuffers);

    // End.
    for (uint32_t commandBufferIndex = 0;
         commandBufferIndex < commandBufferCount; commandBufferIndex++)
        vkEndCommandBuffer(pCommandBuffers[commandBufferIndex]);

    // Flush.
    VkResult result = vkxFlushCommandBuffers(
        device, queue, commandBufferCount, pCommandBuffers, pAllocator);

    // Free.
    vkFreeCommandBuffers(
        device, commandPool, commandBufferCount, pCommandBuffers);

    return result;
}

void vkxBeginCommandBuffer(
    VkCommandBuffer commandBuffer,
    VkCommandBufferUsageFlags flags,
    const VkCommandBufferInheritanceInfo* pInheritanceInfo) {
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = flags,
        .pInheritanceInfo = pInheritanceInfo};
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
}
