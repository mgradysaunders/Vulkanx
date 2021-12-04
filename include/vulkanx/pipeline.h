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
#ifndef VULKANX_PIPELINE_H
#define VULKANX_PIPELINE_H

#include <vulkanx/memory.h>

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

/**
 * \defgroup pipeline Pipeline
 *
 * `<vulkanx/pipeline.h>`
 */
/**\{*/

/**
 * \brief Graphics pipeline input state.
 *
 * This struct combines
 * - `VkPipelineVertexInputStateCreateInfo`,
 * - `VkPipelineInputAssemblyStateCreateInfo`,
 * - `VkPipelineTessellationStateCreateInfo`, and
 * - partially `VkPipelineRasterizationStateCreateInfo`.
 *
 * This aims to capture the entirety of how graphics primitives are
 * input to and rasterized by Vulkan. Hence, this includes the vertex
 * bindings and attributes, the primitive topology and culling options,
 * and the number of patch control points if tessellation is used.
 */
typedef struct VkxGraphicsPipelineInputState_ {
    /** \brief The binding count. */
    uint32_t bindingCount;

    /** \brief The bindings. */
    const VkVertexInputBindingDescription* pBindings;

    /** \brief The attribute count. */
    uint32_t attributeCount;

    /** \brief The attributes. */
    const VkVertexInputAttributeDescription* pAttributes;

    /** \brief The topology. */
    VkPrimitiveTopology topology;

    /** \brief The front face setting. */
    VkFrontFace frontFace;

    /** \brief The cull mode. */
    VkCullModeFlags cullMode;

    /** \brief The polygon mode. */
    VkPolygonMode polygonMode;

    /** \brief Primitive restart enable? */
    VkBool32 primitiveRestartEnable;

    /** \brief Rasterizer discard enable? */
    VkBool32 rasterizerDiscardEnable;

    /** \brief Control points per patch, or 0 if no tessellation. */
    uint32_t patchControlPoints;

    /** \brief The line width, if rasterizing lines. */
    float lineWidth;
} VkxGraphicsPipelineInputState;

/**
 * \brief Graphics pipeline depth state.
 *
 * This struct combines
 * - partially `VkPipelineDepthStencilStateCreateInfo`, and
 * - partially `VkPipelineRasterizationStateCreateInfo`.
 */
typedef struct VkxGraphicsPipelineDepthState_ {
    /** \brief Depth clamp enable? */
    VkBool32 depthClampEnable;

    /** \brief Depth write enable? */
    VkBool32 depthWriteEnable;

    /** \brief The depth compare op. */
    VkCompareOp depthCompareOp;

    /** \brief Depth bounds test enable? */
    VkBool32 depthBoundsTestEnable;

    /** \brief The minimum depth bounds. */
    float minDepthBounds;

    /** \brief The maximum depth bounds. */
    float maxDepthBounds;

    /** \brief Depth bias enable? */
    VkBool32 depthBiasEnable;

    /** \brief The depth bias constant factor. */
    float depthBiasConstantFactor;

    /** \brief The depth bias clamp. */
    float depthBiasClamp;

    /** \brief The depth bias slope factor. */
    float depthBiasSlopeFactor;
} VkxGraphicsPipelineDepthState;

/** \brief Graphics pipeline stencil state. */
typedef struct VkxGraphicsPipelineStencilState_ {
    /** \brief The front state. */
    VkStencilOpState front;

    /** \brief The back state. */
    VkStencilOpState back;
} VkxGraphicsPipelineStencilState;

/**
 * \brief Graphics pipeline multisample state.
 *
 * This struct wraps `VkPipelineMultisampleStateCreateInfo`. It is
 * nearly identical, but inlines `pSampleMask` as `sampleMask` since it
 * should never be a pointer to more than two values. Also, this may be
 * left out of `VkxGraphicsPipelineCreateInfo`, in which case an effective
 * _no multisampling_ default struct is used.
 */
typedef struct VkxGraphicsPipelineMultisampleState_ {
    /** \brief The samples. */
    VkSampleCountFlagBits samples;

    /** \brief Sample shading enable? */
    VkBool32 sampleShadingEnable;

    /** \brief The minimum sample shading fraction. */
    float minSampleShading;

    /** \brief Sample mask enable? */
    VkBool32 sampleMaskEnable;

    /** \brief The sample mask. */
    VkSampleMask sampleMask[2];

    /** \brief Alpha-to-coverable enable? */
    VkBool32 alphaToCoverageEnable;

    /** \brief Alpha-to-one enable? */
    VkBool32 alphaToOneEnable;
} VkxGraphicsPipelineMultisampleState;

/**
 * \brief Graphics pipeline create info.
 *
 * This struct replaces `VkGraphicsPipelineCreateInfo`. This uses
 * - `VkxGraphicsPipelineInputState`,
 * - `VkxGraphicsPipelineDepthState`,
 * - `VkxGraphicsPipelineStencilState`, and
 * - `VkxGraphicsPipelineMultisampleState`
 *
 * to replace
 * - `VkPipelineVertexInputStateCreateInfo`,
 * - `VkPipelineInputAssemblyStateCreateInfo`,
 * - `VkPipelineTessellationStateCreateInfo`,
 * - `VkPipelineDepthStencilStateCreateInfo`,
 * - `VkPipelineRasterizationStateCreateInfo`, and
 * - `VkPipelineMultisampleStateCreateInfo`.
 *
 * This further inlines
 * `VkPipelineViewportStateCreateInfo`,
 * `VkPipelineColorBlendStateCreateInfo`, and
 * `VkPipelineDynamicStateCreateInfo`, thus reducing the number of
 * intermediate structs from 9 to 4.
 */
typedef struct VkxGraphicsPipelineCreateInfo_ {
    /** \brief The stage count. */
    uint32_t stageCount;

    /** \brief The stages. */
    const VkPipelineShaderStageCreateInfo* pStages;

    /** \brief The input state. */
    const VkxGraphicsPipelineInputState* pInputState;

    /** \brief _Optional_. The depth state. */
    const VkxGraphicsPipelineDepthState* pDepthState;

    /** \brief _Optional_. The stencil state. */
    const VkxGraphicsPipelineStencilState* pStencilState;

    /** \brief _Optional_. The multisample state. */
    const VkxGraphicsPipelineMultisampleState* pMultisampleState;

    /**
     * \name Viewport state
     *
     * Note that `VkPipelineViewportStateCreateInfo` has another field
     * `scissorCount` to specify the number of scissors `pScissors`, which
     * is required to be equivalent to `viewportCount`. This struct makes
     * this requirement implicit by removing `scissorCount` and renaming
     * `pScissors` to `pViewportScissors`.
     *
     * For convenience, `pViewportScissors` is optional. If `NULL`, the
     * implementation initializes scissors to match each viewport in
     * `pViewports`.
     */
    /**\{*/

    /** \brief The viewport count. */
    uint32_t viewportCount;

    /** \brief The viewports. */
    const VkViewport* pViewports;

    /** \brief _Optional_. The viewport scissors. */
    const VkRect2D* pViewportScissors;

    /**\}*/

    /**
     * \name Color blend state
     */
    /**\{*/

    /** \brief Logic op enable? */
    VkBool32 logicOpEnable;

    /** \brief The logic op. */
    VkLogicOp logicOp;

    /** \brief The blend attachment count. */
    uint32_t blendAttachmentCount;

    /** \brief The blend attachments. */
    const VkPipelineColorBlendAttachmentState* pBlendAttachments;

    /** \brief The blend constants. */
    float blendConstants[4];

    /**\}*/

    /**
     * \name Dynamic state
     */
    /**\{*/

    /** \brief The dynamic state count. */
    uint32_t dynamicStateCount;

    /** \brief The dynamic states. */
    const VkDynamicState* pDynamicStates;

    /**\}*/

    /** \brief The layout. */
    VkPipelineLayout layout;

    /** \brief The render pass. */
    VkRenderPass renderPass;

    /** \brief The subpass index. */
    uint32_t subpass;

    /** \brief The base pipeline index, or -1 if no parent. */
    int32_t basePipeline;
} VkxGraphicsPipelineCreateInfo;

/**
 * \brief Create graphics pipelines.
 *
 * \param[in] device
 * Device.
 *
 * \param[in] createInfoCount
 * Create info count.
 *
 * \param[in] pCreateInfos
 * Create infos.
 *
 * \param[in] pAllocator
 * _Optional_. Allocation callbacks.
 *
 * \param[out] pPipelines
 * Pipelines.
 *
 * \pre
 * - `device` is valid
 * - `pCreateInfos` points to `createInfoCount` values
 * - `pPipelines` points to `createInfoCount` values
 * - `pPipelines` is `NULL` only if `createInfoCount` is zero, in which
 * case the implementation immediately returns `VK_SUCCESS`
 */
VkResult vkxCreateGraphicsPipelines(
    VkDevice device,
    uint32_t createInfoCount,
    const VkxGraphicsPipelineCreateInfo* pCreateInfos,
    const VkAllocationCallbacks* pAllocator,
    VkPipeline* pPipelines);

/**\}*/

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus

#endif // #ifndef VULKANX_PIPELINE_H
