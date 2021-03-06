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
#ifndef VULKANX_SETUP_H
#define VULKANX_SETUP_H

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

/**
 * \defgroup setup Setup
 *
 * `<vulkanx/setup.h>`
 */
/**\{*/

/** \brief Instance create info. */
typedef struct VkxInstanceCreateInfo_ {
    /** \brief The application name. */
    const char* pApplicationName;

    /** \brief The application version, from `VK_MAKE_VERSION`. */
    uint32_t applicationVersion;

    /** \brief The engine name. */
    const char* pEngineName;

    /** \brief The engine version, from `VK_MAKE_VERSION`. */
    uint32_t engineVersion;

    /** \brief The API version. */
    uint32_t apiVersion;

    /** \brief The requested instance layer count. */
    uint32_t requestedLayerCount;

    /** \brief The requested instance layer names. */
    const char* const* ppRequestedLayerNames;

    /** \brief The requested instance extension count. */
    uint32_t requestedExtensionCount;

    /** \brief The requested instance extension names. */
    const char* const* ppRequestedExtensionNames;
} VkxInstanceCreateInfo;

/**
 * \brief Create instance.
 *
 * \param[in] pCreateInfo
 * Create info.
 *
 * \param[in] pAllocator
 * _Optional_. Allocation callbacks.
 *
 * \param[out] pRequestedLayersEnabled
 * _Optional_. Booleans to indicate which requested extensions were enabled.
 *
 * \param[out] pRequestedExtensionsEnabled
 * _Optional_. Booleans to indicate which requested layers were enabled.
 *
 * \param[out] pInstance
 * Instance.
 */
VkResult vkxCreateInstance(
    const VkxInstanceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkBool32* pRequestedLayersEnabled,
    VkBool32* pRequestedExtensionsEnabled,
    VkInstance* pInstance);

/**
 * \brief Physical device select info.
 *
 * This struct can be used to control how physical device selection
 * is carried out by `vkxSelectPhysicalDevice`. By default, the implementation
 * considers every available device and tries to select the most performant
 * device with the most available features.
 */
typedef struct VkxPhysicalDeviceSelectInfo_ {
    /** \brief _Optional_. The requested name.
     *
     * If non-`NULL`, physical device selection looks for the device with
     * this specific name. If no such device exists, the implementation
     * returns `VK_NULL_HANDLE`.
     */
    const char* pRequestedName;

    /** \brief _Optional_. The requested features.
     *
     * If non-`NULL`, physical device selection only considers these
     * requested features. The implementation does not require that every
     * requested feature is supported by a device, but rather seeks the device
     * that supports the most requested features.
     */
    const VkPhysicalDeviceFeatures* pRequestedFeatures;

    /** \brief _Optional_. Is physical device ok for selection?
     *
     * If non-`NULL`, physical device selection only considers physical
     * devices where this callback returns `VK_TRUE`. If every candidate
     * device is rejected by this callback, the implementation returns
     * `VK_NULL_HANDLE`.
     */
    VkBool32 (*pIsPhysicalDeviceOk)(VkPhysicalDevice, void*);

    /** \brief _Optional_. The user data for callback. */
    void* pUserData;
} VkxPhysicalDeviceSelectInfo;

/**
 * \brief Select physical device.
 *
 * \param[in] instance
 * Instance.
 *
 * \param[in] pSelectInfo
 * _Optional_. Select info.
 */
VkPhysicalDevice vkxSelectPhysicalDevice(
    VkInstance instance, const VkxPhysicalDeviceSelectInfo* pSelectInfo);

/**
 * \brief Select format.
 *
 * \param[in] physicalDevice
 * Physical device.
 *
 * \param[in] requestedTiling
 * Requested tiling.
 *
 * \param[in] requestedFeatures
 * Requested features.
 *
 * \param[in] candidateFormatCount
 * Candidate format count.
 *
 * \param[in] pCandidateFormats
 * Candidate formats.
 *
 * \pre
 * - `physicalDevice` is valid
 * - `pCandidateFormats` points to `candidateFormatCount` values
 *
 * \return
 * Format in `pCandidateFormats` with `requestedFeatures`, or
 * `VK_FORMAT_UNDEFINED` if no such format exists.
 */
VkFormat vkxSelectFormat(
    VkPhysicalDevice physicalDevice,
    VkImageTiling requestedTiling,
    VkFormatFeatureFlags requestedFeatures,
    uint32_t candidateFormatCount,
    const VkFormat* pCandidateFormats);

/**
 * \brief Get supported depth format.
 *
 * \param[in] physicalDevice
 * Physical device.
 */
VkFormat vkxGetDepthFormat(VkPhysicalDevice physicalDevice);

/**
 * \brief Get supported depth/stencil format.
 *
 * \param[in] physicalDevice
 * Physical device.
 */
VkFormat vkxGetDepthStencilFormat(VkPhysicalDevice physicalDevice);

/**
 * \brief Get format texel size in bytes.
 *
 * \param[in] format
 * Format.
 *
 * \note
 * For the time being, the implementation handles uncompressed color
 * and depth/stencil formats only. If `format` is a compressed format,
 * plane format, or extension format, the implementation returns `0`.
 */
uint32_t vkxGetFormatTexelSize(VkFormat format);

/** \brief Device queue family. */
typedef struct VkxDeviceQueueFamily_ {
    /** \brief The queue flags. */
    VkQueueFlags queueFlags;

    /** \brief The queue family properties. */
    VkQueueFamilyProperties queueFamilyProperties;

    /** \brief The queue family index. */
    uint32_t queueFamilyIndex;

    /** \brief The queue count. */
    uint32_t queueCount;

    /** \brief The queues. */
    VkQueue* pQueues;

    /** \brief The queue priorities. */
    float* pQueuePriorities;

    /** \brief The command pool count. */
    uint32_t commandPoolCount;

    /** \brief The command pools. */
    VkCommandPool* pCommandPools;

    /** \brief The command pool flags for each command pool. */
    VkCommandPoolCreateFlags* pCommandPoolCreateFlags;
} VkxDeviceQueueFamily;

/**
 * \brief Device queue family create info.
 *
 * This struct specifies the requirements for a queue family in
 * `VkxDevice`.
 */
typedef struct VkxDeviceQueueFamilyCreateInfo_ {
    /** \brief The queue flags. */
    VkQueueFlags queueFlags;

    /** \brief The queue count. */
    uint32_t queueCount;

    /** \brief The minimum queue count. */
    uint32_t minQueueCount;

    /** \brief If creating many queues, use equal or inequal priority? */
    VkBool32 useEqualPriority;

    /** \brief The surface for present operations or `VK_NULL_HANDLE`. */
    VkSurfaceKHR presentSurface;

    /** \brief The command pool count. */
    uint32_t commandPoolCount;

    /** \brief The command pool create flags. */
    const VkCommandPoolCreateFlags* pCommandPoolCreateFlags;
} VkxDeviceQueueFamilyCreateInfo;

/** \brief Device. */
typedef struct VkxDevice_ {
    /** \brief The physical device. */
    VkPhysicalDevice physicalDevice;

    /** \brief The physical device features. */
    VkPhysicalDeviceFeatures* pPhysicalDeviceFeatures;

    /** \brief The logical device. */
    VkDevice device;

    /** \brief The queue family count. */
    uint32_t queueFamilyCount;

    /** \brief The queue families. */
    VkxDeviceQueueFamily* pQueueFamilies;
} VkxDevice;

/** \brief Device create info. */
typedef struct VkxDeviceCreateInfo_ {
    /** \brief _Optional_. The physical device select info. */
    const VkxPhysicalDeviceSelectInfo* pSelectInfo;

    /** \brief The queue family create info count. */
    uint32_t queueFamilyCreateInfoCount;

    /** \brief The queue family create infos. */
    const VkxDeviceQueueFamilyCreateInfo* pQueueFamilyCreateInfos;

    /** \brief _Optional_. The enabled extension count. */
    uint32_t enabledExtensionCount;

    /** \brief _Optional_. The enabled extension names. */
    const char* const* ppEnabledExtensionNames;
} VkxDeviceCreateInfo;

/**
 * \brief Create device.
 *
 * \param[in] instance
 * Instance.
 *
 * \param[in] pCreateInfo
 * Create info.
 *
 * \param[in] pAllocator
 * _Optional_. Allocation callbacks.
 *
 * \param[out] pDevice
 * Device.
 */
VkResult vkxCreateDevice(
    VkInstance instance,
    const VkxDeviceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkxDevice* pDevice);

/**
 * \brief Destroy device.
 *
 * \param[inout] pDevice
 * Device.
 *
 * \param[in] pAllocator
 * _Optional_. Allocation callbacks.
 */
void vkxDestroyDevice(
    VkxDevice* pDevice, const VkAllocationCallbacks* pAllocator);

/**\}*/

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus

#endif // #ifndef VULKANX_SETUP_H
