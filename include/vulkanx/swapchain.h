/* Copyright (c) 2019-21 M. Grady Saunders
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*-*-*-*-*-*-*/
#pragma once
#ifndef VULKANX_SWAPCHAIN_H
#define VULKANX_SWAPCHAIN_H

#include <vulkan/vulkan.h>
#include <vulkanx/image.h>

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

/**
 * \defgroup swapchain Swapchain
 *
 * `<vulkanx/swapchain.h>`
 */
/**\{*/

#if 0
/**
 * \brief Swapchain color attachment description.
 */
typedef struct VkxSwapchainColorAttachmentDescription_
{
    /** \brief Format. */
    VkFormat format;

    /** \brief Usage. */
    VkImageUsageFlags usage;

    /** \brief _Optional_. Extent override, given swapchain extent. */
    VkExtent2D (*pExtentOverride)(VkExtent2D, void*);

    /** \brief _Optional_. Extent override user data. */
    void* pExtentOverrideUserData;

    /** \brief Array layers. */
    uint32_t arrayLayers;
}
VkxSwapchainColorAttachmentDescription;

typedef struct VkxSwapchainAttachments_
{
    VkxImageGroup images;

    VkImageView* pImageViews;
}
VkxSwapchainAttachments;
#endif

/**
 * \brief Swapchain.
 */
typedef struct VkxSwapchain_ {
    /**
     * \name Swapchain configuration (immutable)
     */
    /**\{*/

    /** \brief The associated physical device. */
    VkPhysicalDevice physicalDevice;

    /** \brief The associated device. */
    VkDevice device;

    /** \brief The present mode. */
    VkPresentModeKHR presentMode;

    /** \brief The surface format. */
    VkSurfaceFormatKHR surfaceFormat;

    /** \brief The pre-transform. */
    VkSurfaceTransformFlagBitsKHR preTransform;

    /** \brief The composite alpha. */
    VkCompositeAlphaFlagBitsKHR compositeAlpha;

    /** \brief The image usage flags. */
    VkImageUsageFlags imageUsage;

    /** \brief The queue family index count. */
    uint32_t queueFamilyIndexCount;

    /** \brief The queue family indices. */
    uint32_t queueFamilyIndices[2];

    /** \brief The image sharing mode. */
    VkSharingMode imageSharingMode;

    /** \brief The graphics queue. */
    VkQueue graphicsQueue;

    /** \brief The present queue. */
    VkQueue presentQueue;

    /**\}*/

    /**
     * \name Swapchain
     */
    /**\{*/

    /** \brief The swapchain. */
    VkSwapchainKHR swapchain;

    /** \brief The image extent. */
    VkExtent2D imageExtent;

    /** \brief The image count. */
    uint32_t imageCount;

    /** \brief The images. */
    VkImage* pImages;

    /** \brief The image views. */
    VkImageView* pImageViews;

    /** \brief The indices for each image. */
    uint32_t* pIndices;

    /** \brief The acquired semaphores for each image. */
    VkSemaphore* pAcquiredSemaphores;

    /** \brief THe released semaphores for each image. */
    VkSemaphore* pReleasedSemaphores;

    /** \brief The next acquired semaphore. */
    VkSemaphore nextAcquiredSemaphore;

    /** \brief The next released semaphore. */
    VkSemaphore nextReleasedSemaphore;

    /** \brief The fences for each image. */
    VkFence* pFences;

    /** \brief The command pool. */
    VkCommandPool commandPool;

    /** \brief The command buffers for each image. */
    VkCommandBuffer* pCommandBuffers;

    /**\}*/

    /**
     * \name Swapchain latent state
     */
    /**\{*/

    /** \brief The render pass. */
    VkRenderPass renderPass;

    /** \brief The framebuffers for each image. */
    VkFramebuffer* pFramebuffers;

    /**\}*/

    /**
     * \name Swapchain active state
     */
    /**\{*/

    /** \brief The active image index. */
    uint32_t activeIndex;

    /** \brief The active acquried semaphore. */
    VkSemaphore activeAcquiredSemaphore;

    /** \brief The active released semaphore. */
    VkSemaphore activeReleasedSemaphore;

    /** \brief The active fence. */
    VkFence activeFence;

    /** \brief The active command buffer. */
    VkCommandBuffer activeCommandBuffer;

    /** \brief The active framebuffer. */
    VkFramebuffer activeFramebuffer;

    /**\}*/
} VkxSwapchain;

/**
 * \brief Create swapchain.
 *
 * \param[in] physicalDevice
 * Physical device.
 *
 * \param[in] device
 * Device.
 *
 * \param[in] graphicsQueueFamilyIndex
 * Graphics queue family index.
 *
 * \param[in] presentQueueFamilyIndex
 * Present queue family index.
 *
 * \param[in] surface
 * Surface.
 *
 * \param[in] surfaceExtent
 * Surface extent.
 *
 * \param[in] pAllocator
 * _Optional_. Allocation callbacks.
 *
 * \param[out] pSwapchain
 * Swapchain.
 *
 * \pre
 * - `physicalDevice` is valid
 * - `device` is valid
 * - `graphicsQueueFamilyIndex` is valid and supports graphics commands
 * - `presentQueueFamilyIndex` is valid and supports present commands
 * - `surface` is valid
 * - `pSwapchain` is non-`NULL`
 * - `pSwapchain` is uninitialized
 *
 * \post
 * - on success, `pSwapchain` is properly initialized
 * - on failure, `pSwapchain` is nullified
 */
VkResult vkxCreateSwapchain(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    uint32_t graphicsQueueFamilyIndex,
    uint32_t presentQueueFamilyIndex,
    VkSurfaceKHR surface,
    VkExtent2D surfaceExtent,
    const VkAllocationCallbacks* pAllocator,
    VkxSwapchain* pSwapchain);

/**
 * \brief Recreate swapchain.
 *
 * \param[in] surface
 * Surface.
 *
 * \param[in] surfaceExtent
 * Surface extent.
 *
 * \param[in] pAllocator
 * _Optional_. Allocation callbacks.
 *
 * \param[inout] pSwapchain
 * Swapchain.
 *
 * \pre
 * - `surface` was used to create `pSwapchain`
 * - `pAllocator` was used to create `pSwapchain`
 * - `pSwapchain` is non-`NULL`
 * - `pSwapchain` was previously created by `vkxCreateSwapchain`
 */
VkResult vkxRecreateSwapchain(
    VkSurfaceKHR surface,
    VkExtent2D surfaceExtent,
    const VkAllocationCallbacks* pAllocator,
    VkxSwapchain* pSwapchain);

/**
 * \brief Destroy swapchain.
 *
 * \param[inout] pSwapchain
 * Swapchain.
 *
 * \param[in] pAllocator
 * _Optional_. Allocation callbacks.
 *
 * \pre
 * - `pAllocator` was used to create `pSwapchain`
 * - `pSwapchain` was previously created by `vkxCreateSwapchain`
 *
 * \post
 * - `pSwapchain` is nullified
 *
 * \note
 * Does nothing if `pSwapchain` is `NULL`.
 */
void vkxDestroySwapchain(
    VkxSwapchain* pSwapchain, const VkAllocationCallbacks* pAllocator);

/**
 * \brief Setup render pass.
 */
VkResult vkxSwapchainSetupRenderPass(
    VkxSwapchain* pSwapchain,
    const VkRenderPassCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator);

#if 0
/**
 * \brief Get render pass begin info.
 */
void vkxSwapchainGetRenderPassBeginInfo(
            const VkxSwapchain* pSwapchain,
            uint32_t clearValueCount,
            const VkClearValue* pClearValues,
            VkRenderPassBeginInfo* pBeginInfo);
#endif

/**
 * \brief Acquire next image.
 *
 * \param[in] device
 * Device.
 *
 * \param[inout] pSwapchain
 * Swapchain.
 *
 * \param[in] timeout
 * Timeout.
 */
VkResult vkxSwapchainAcquireNextImage(
    VkxSwapchain* pSwapchain, uint64_t timeout);

VkResult vkxSwapchainSubmit(VkxSwapchain* pSwapchain);

VkResult vkxSwapchainPresent(
    VkxSwapchain* pSwapchain,
    uint32_t moreWaitSemaphoreCount,
    const VkSemaphore* pMoreWaitSemaphores);

/**\}*/

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus

#endif // #ifndef VULKANX_SWAPCHAIN_H
