#include "function/render/debugdraw/debug_draw_pipeline.h"
#include "core/base/macro.h"
#include "function/global/global_context.h"

#include <debugdraw_frag.h>
#include <debugdraw_vert.h>

namespace Piccolo
{
    void DebugDrawPipeline::initialize()
    {
        m_rhi = g_runtime_global_context.m_render_system->getRHI();
        setupPipelines();
    }

    void DebugDrawPipeline::setupRenderPass()
    {
        RHIAttachmentDescription color_attachment_description {};
        color_attachment_description.format = m_rhi->getSwapchainInfo().image_format;
    }

    void DebugDrawPipeline::setupPipelines()
    {
        RHIShader* vert_shader_module = m_rhi->createShaderModule(DEBUGDRAW_VERT);
        RHIShader* frag_shader_module = m_rhi->createShaderModule(DEBUGDRAW_FRAG);

        // create vertex stage pipeline information
        RHIPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info {};
        vert_pipeline_shader_stage_create_info.sType  = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_pipeline_shader_stage_create_info.stage  = RHI_SHADER_STAGE_VERTEX_BIT; // using in vertex stage
        vert_pipeline_shader_stage_create_info.module = vert_shader_module;          // set module(SPIR-V code)
        vert_pipeline_shader_stage_create_info.pName  = "main";                      // SPIR-V program entry

        // create fragment stage pipeline information
        RHIPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info {};
        frag_pipeline_shader_stage_create_info.sType  = RHI_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_pipeline_shader_stage_create_info.stage  = RHI_SHADER_STAGE_FRAGMENT_BIT;
        frag_pipeline_shader_stage_create_info.module = frag_shader_module;
        frag_pipeline_shader_stage_create_info.pName  = "main";

        RHIPipelineShaderStageCreateInfo shader_stages[] = {vert_pipeline_shader_stage_create_info,
                                                            frag_pipeline_shader_stage_create_info};

        // set vertex input information
        VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
        vertex_input_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount   = 0;
        vertex_input_info.pVertexBindingDescriptions      = nullptr; // Optional
        vertex_input_info.vertexAttributeDescriptionCount = 0;
        vertex_input_info.pVertexAttributeDescriptions    = nullptr; // Optional

        // set vertex input assembly rule
        VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // 在花第一个三角形时，直接选这个选项
        input_assembly.primitiveRestartEnable = VK_FALSE; // 不进行图元重启（就画个三角形没必要）

        // set viewport & scissor change stage information
        RHIPipelineViewportStateCreateInfo viewport_state_create_info {};
        viewport_state_create_info.sType         = RHI_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.pViewports    = m_rhi->getSwapchainInfo().viewport;
        viewport_state_create_info.scissorCount  = 1;
        viewport_state_create_info.pScissors     = m_rhi->getSwapchainInfo().scissor;

        // set rasterization stage
        RHIPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
        rasterization_state_create_info.sType            = RHI_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_create_info.depthClampEnable = RHI_FALSE;        // discard the far & near fragment
        rasterization_state_create_info.rasterizerDiscardEnable = RHI_FALSE; // not discard the rasterization stage
        rasterization_state_create_info.polygonMode             = RHI_POLYGON_MODE_FILL; // fill the vertices to polygon
        rasterization_state_create_info.lineWidth = 1.0f; // when the line width is greater than 1, GPU will be used
        rasterization_state_create_info.cullMode  = RHI_CULL_MODE_NONE;       // cull nothing
        rasterization_state_create_info.frontFace = RHI_FRONT_FACE_CLOCKWISE; // specify the vertices join order
        rasterization_state_create_info.depthBiasEnable         = RHI_FALSE;
        rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
        rasterization_state_create_info.depthBiasClamp          = 0.0f;
        rasterization_state_create_info.depthBiasSlopeFactor    = 0.0f;

        // set MSAA information
        RHIPipelineMultisampleStateCreateInfo multisample_state_create_info {};
        multisample_state_create_info.sType                = RHI_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_create_info.sampleShadingEnable  = RHI_FALSE;
        multisample_state_create_info.rasterizationSamples = RHI_SAMPLE_COUNT_1_BIT;

        // // set depth & stencil test information
        // RHIPipelineDepthStencilStateCreateInfo depth_stencil_create_info {};
        // depth_stencil_create_info.sType                 =
        // RHI_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO; depth_stencil_create_info.depthTestEnable =
        // RHI_TRUE; depth_stencil_create_info.depthWriteEnable      = RHI_TRUE;
        // depth_stencil_create_info.depthCompareOp        = RHI_COMPARE_OP_LESS;
        // depth_stencil_create_info.depthBoundsTestEnable = RHI_FALSE;
        // depth_stencil_create_info.stencilTestEnable     = RHI_FALSE;

        // set color blend rule in every buffer frame
        RHIPipelineColorBlendAttachmentState color_blend_attachment_state {}; // used in every buffer frame
        color_blend_attachment_state.colorWriteMask = RHI_COLOR_COMPONENT_R_BIT | RHI_COLOR_COMPONENT_G_BIT |
                                                      RHI_COLOR_COMPONENT_B_BIT | RHI_COLOR_COMPONENT_A_BIT;
        color_blend_attachment_state.blendEnable         = RHI_FALSE; // will be true later
        color_blend_attachment_state.srcColorBlendFactor = RHI_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment_state.dstColorBlendFactor = RHI_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment_state.colorBlendOp        = RHI_BLEND_OP_ADD;
        color_blend_attachment_state.srcAlphaBlendFactor = RHI_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstAlphaBlendFactor = RHI_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.alphaBlendOp        = RHI_BLEND_OP_ADD;

        // set global color blend methods
        RHIPipelineColorBlendStateCreateInfo color_blend_state_create_info {};
        color_blend_state_create_info.sType             = RHI_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state_create_info.logicOpEnable     = RHI_FALSE; // will be true maybe later
        color_blend_state_create_info.logicOp           = RHI_LOGIC_OP_COPY;
        color_blend_state_create_info.attachmentCount   = 1;
        color_blend_state_create_info.pAttachments      = &color_blend_attachment_state;
        color_blend_state_create_info.blendConstants[0] = 0.0f;
        color_blend_state_create_info.blendConstants[1] = 0.0f;
        color_blend_state_create_info.blendConstants[2] = 0.0f;
        color_blend_state_create_info.blendConstants[3] = 0.0f;

        // some settings can be dynamic(it will not rebulild the pipeline however)
        RHIDynamicState                   dynamic_states[] = {RHI_DYNAMIC_STATE_VIEWPORT, RHI_DYNAMIC_STATE_SCISSOR};
        RHIPipelineDynamicStateCreateInfo dynamic_state_create_info {};
        dynamic_state_create_info.sType             = RHI_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = 2;
        dynamic_state_create_info.pDynamicStates    = dynamic_states;

        // some glsl uniform will be specified in the Pipeline layout
        RHIPipelineLayoutCreateInfo pipeline_layout_create_info {};
        pipeline_layout_create_info.sType                  = RHI_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount         = 0; // TODO: layout descriptor
        pipeline_layout_create_info.pSetLayouts            = nullptr;
        pipeline_layout_create_info.pushConstantRangeCount = 0;
        pipeline_layout_create_info.pPushConstantRanges    = nullptr;

        m_render_pipelines.resize(1);
        if (m_rhi->createPipelineLayout(&pipeline_layout_create_info, m_render_pipelines[0].layout) != RHI_SUCCESS)
            LOG_ERROR("Failed to create RHI pipeline layout");
    }
} // namespace Piccolo