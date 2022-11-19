#include "function/render/debugdraw/debug_draw_manager.h"
#include "function/global/global_context.h"
#include "function/render/debugdraw/debug_draw_pipeline.h"
#include <stdint.h>

namespace Piccolo
{
    void DebugDrawManager::initialize()
    {
        m_rhi = g_runtime_global_context.m_render_system->getRHI();
        setupPipelines();
    }

    void DebugDrawManager::setupPipelines()
    {
        for (uint8_t i = 0; static_cast<DebugDrawPipelineType>(i) < DebugDrawPipelineType::count; i++)
        {
            m_debug_draw_pipeline[i] = new DebugDrawPipeline(static_cast<DebugDrawPipelineType>(i));
            m_debug_draw_pipeline[i]->initialize();
        }
    }

    void DebugDrawManager::drawPointLineTriangleBox(uint32_t current_swapchain_image_index)
    {
        std::vector<DebugDrawPipeline*> vc_pipelines {
            m_debug_draw_pipeline[static_cast<uint32_t>(DebugDrawPipelineType::triangle)],
        };
        std::vector<size_t> vc_start_offsets {m_triangle_start_offset = 0};
        std::vector<size_t> vc_end_offsets {m_triangle_end_offset = 3};

        RHIClearValue clear_values[1];                    // 2
        clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f}; // use black color as clear value
        // clear_values[1].depthStencil = {1.0f, 0};
        RHIRenderPassBeginInfo renderpass_begin_info {};
        renderpass_begin_info.sType             = RHI_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_begin_info.renderArea.offset = {0, 0};
        renderpass_begin_info.renderArea.extent = m_rhi->getSwapchainInfo().extent;
        renderpass_begin_info.clearValueCount   = (sizeof(clear_values) / sizeof(clear_values[0]));
        renderpass_begin_info.pClearValues      = clear_values;

        for (size_t i = 0; i < vc_pipelines.size(); i++)
        {
            renderpass_begin_info.renderPass = vc_pipelines[i]->getFramebuffer().render_pass;
            renderpass_begin_info.framebuffer =
                vc_pipelines[i]->getFramebuffer().framebuffers[current_swapchain_image_index];
            m_rhi->cmdBeginRenderPassPFN(
                m_rhi->getCurrentCommandBuffer(), &renderpass_begin_info, RHI_SUBPASS_CONTENTS_INLINE);
            m_rhi->cmdBindPipelinePFN(m_rhi->getCurrentCommandBuffer(),
                                      RHI_PIPELINE_BIND_POINT_GRAPHICS, // graphics pipeline
                                      vc_pipelines[i]->getPipeline().pipeline);
            m_rhi->cmdDraw(
                m_rhi->getCurrentCommandBuffer(), vc_end_offsets[i] - vc_start_offsets[i], 1, vc_start_offsets[i], 0);
            m_rhi->cmdEndRenderPassPFN(m_rhi->getCurrentCommandBuffer());
        }
    }

    void DebugDrawManager::drawDebugObject(uint32_t current_swapchain_image_index)
    {
        drawPointLineTriangleBox(current_swapchain_image_index);
    }

    void DebugDrawManager::draw(uint32_t current_swapchain_image_index)
    {
        drawDebugObject(current_swapchain_image_index);
    }

} // namespace Piccolo