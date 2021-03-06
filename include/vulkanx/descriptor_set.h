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
#ifndef VULKANX_DESCRIPTOR_SET_H
#define VULKANX_DESCRIPTOR_SET_H

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

/**
 * \defgroup descriptor_set Descriptor set
 *
 * `<vulkanx/descriptor_set.h>`
 */
/**\{*/

/** \brief Descriptor set group. */
typedef struct VkxDescriptorSetGroup_ {
    /** \brief The descriptor set layout. */
    VkDescriptorSetLayout setLayout;

    /** \brief The descriptor set count. */
    uint32_t setCount;

    /** \brief The descriptor sets. */
    VkDescriptorSet* pSets;

    /** \brief The descriptor pool. */
    VkDescriptorPool pool;
} VkxDescriptorSetGroup;

/**
 * \brief Create descriptor set group.
 *
 * \param[in] device
 * Device.
 *
 * \param[in] pSetLayoutCreateInfo
 * Descriptor set layout create info.
 *
 * \param[in] setCount
 * Descriptor set count.
 *
 * \param[in] pAllocator
 * _Optional_. Allocation callbacks.
 *
 * \param[out] pSetGroup
 * Descriptor set group.
 */
VkResult vkxCreateDescriptorSetGroup(
    VkDevice device,
    const VkDescriptorSetLayoutCreateInfo* pSetLayoutCreateInfo,
    uint32_t setCount,
    const VkAllocationCallbacks* pAllocator,
    VkxDescriptorSetGroup* pSetGroup);

/**
 * \brief Destroy descriptor set group.
 *
 * \param[in] device
 * Device.
 *
 * \param[inout] pSetGroup
 * Descriptor set group.
 *
 * \param[in] pAllocator
 * _Optional_. Allocation callbacks.
 */
void vkxDestroyDescriptorSetGroup(
    VkDevice device,
    VkxDescriptorSetGroup* pSetGroup,
    const VkAllocationCallbacks* pAllocator);

/** \brief Dynamic descriptor pool. */
typedef struct VkxDynamicDescriptorPool_ {
    /** \brief The descriptor pool create info. */
    VkDescriptorPoolCreateInfo poolCreateInfo;

    /** \brief The descriptor pool count. */
    uint32_t poolCount;

    /** \brief The descriptor pool capacity. */
    uint32_t poolCapacity;

    /** \brief The descriptor pools. */
    VkDescriptorPool* pPools;

    /** \brief The descriptor pool full flags. */
    VkBool32* pFullFlags;
} VkxDynamicDescriptorPool;

/** \brief Dynamic descriptor set. */
typedef struct VkxDynamicDescriptorSet_ {
    /** \brief The descriptor set. */
    VkDescriptorSet set;

    /** \brief The associated descriptor pool index. */
    uint32_t associatedPoolIndex;
} VkxDynamicDescriptorSet;

/**
 * \brief Create dynamic descriptor pool.
 *
 * \param[in] device
 * Device.
 *
 * \param[in] pPoolCreateInfo
 * Descriptor pool create info.
 *
 * \param[in] pAllocator
 * _Optional_. Allocation callbacks.
 *
 * \param[out] pDynamicPool
 * Dynamic descriptor pool.
 */
VkResult vkxCreateDynamicDescriptorPool(
    VkDevice device,
    const VkDescriptorPoolCreateInfo* pPoolCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkxDynamicDescriptorPool* pDynamicPool);

/**
 * \brief Allocate dynamic descriptor sets.
 *
 * \param[in] device
 * Device.
 *
 * \param[inout] pDynamicPool
 * Dynamic descriptor pool.
 *
 * \param[in] setCount
 * Descriptor set count.
 *
 * \param[in] pSetLayouts
 * Descriptor set layouts.
 *
 * \param[in] pAllocator
 * _Optional_. Allocation callbacks.
 *
 * \param[out] pDynamicSets
 * Dynamic descriptor sets.
 */
VkResult vkxAllocateDynamicDescriptorSets(
    VkDevice device,
    VkxDynamicDescriptorPool* pDynamicPool,
    uint32_t setCount,
    const VkDescriptorSetLayout* pSetLayouts,
    const VkAllocationCallbacks* pAllocator,
    VkxDynamicDescriptorSet* pDynamicSets);

/**
 * \brief Free dynamic descriptor sets.
 *
 * \param[in] device
 * Device.
 *
 * \param[inout] pDynamicPool
 * Dynamic descriptor pool.
 *
 * \param[in] setCount
 * Descriptor set count.
 *
 * \param[inout] pDynamicSets
 * Dynamic descriptor sets.
 */
VkResult vkxFreeDynamicDescriptorSets(
    VkDevice device,
    VkxDynamicDescriptorPool* pDynamicPool,
    uint32_t setCount,
    VkxDynamicDescriptorSet* pDynamicSets);

/**
 * \brief Bind dynamic descriptor sets.
 *
 * \param[in] commandBuffer
 * Command buffer.
 *
 * \param[in] pipelineBindPoint
 * Pipeline bind point.
 *
 * \param[in] pipelineLayout
 * Pipeline layout.
 *
 * \param[in] firstSet
 * First descriptor set.
 *
 * \param[in] setCount
 * Descriptor set count.
 *
 * \param[in] pDynamicSets
 * Dynamic descriptor sets.
 *
 * \param[in] dynamicOffsetCount
 * Dynamic offset count.
 *
 * \param[in] pDynamicOffsets
 * Dynamic offsets.
 */
void vkxCmdBindDynamicDescriptorSets(
    VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipelineLayout pipelineLayout,
    uint32_t firstSet,
    uint32_t setCount,
    const VkxDynamicDescriptorSet* pDynamicSets,
    uint32_t dynamicOffsetCount,
    const uint32_t* pDynamicOffsets);

/**
 * \brief Destroy dynamic descriptor pool.
 *
 * \param[in] device
 * Device.
 *
 * \param[inout] pDynamicPool
 * Dynamic descriptor pool.
 *
 * \param[in] pAllocator
 * _Optional_. Allocation callbacks.
 */
void vkxDestroyDynamicDescriptorPool(
    VkDevice device,
    VkxDynamicDescriptorPool* pDynamicPool,
    const VkAllocationCallbacks* pAllocator);

/**\}*/

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus

#endif // #ifndef VULKANX_DESCRIPTOR_SET_H
