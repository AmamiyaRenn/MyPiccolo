#include "function/render/debugdraw/debug_draw_manager.h"
#include "function/global/global_context.h"
#include "function/render/debugdraw/debug_draw_pipeline.h"
#include "function/render/render_type.h"
#include <stddef.h>
#include <stdint.h>

namespace Piccolo
{
    void DebugDrawManager::initialize()
    {
        m_rhi = g_runtime_global_context.m_render_system->getRHI();
        setupPipelines();
        m_buffer_allocator = new DebugDrawAllocator();
    }

    void DebugDrawManager::setupPipelines()
    {
        for (uint8_t i = 0; static_cast<DebugDrawPipelineType>(i) < DebugDrawPipelineType::count; i++)
        {
            m_debug_draw_pipeline[i] = new DebugDrawPipeline(static_cast<DebugDrawPipelineType>(i));
            m_debug_draw_pipeline[i]->initialize();
        }
    }

    // rebuild all pipelines' framebuffers
    void DebugDrawManager::updateAfterRecreateSwapchain()
    {
        for (auto& i : m_debug_draw_pipeline)
            i->recreateAfterSwapchain();
    }

    void DebugDrawManager::prepareDrawBuffer()
    {
        m_buffer_allocator->clear();

        std::vector<DebugDrawVertex> vertexs;

        // m_debug_draw_group_for_render.writeTriangleData(vertexs, true);
        // m_triangle_start_offset = m_buffer_allocator->cacheVertexs(vertexs);
        // m_triangle_end_offset   = m_buffer_allocator->getVertexCacheOffset();

        // FIXME：删除
        vertexs = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                   {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                   {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                   {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

        m_triangle_start_offset = m_buffer_allocator->cacheVertexs(vertexs);
        m_triangle_end_offset   = 4;

        m_buffer_allocator->allocator();
    }

    void DebugDrawManager::drawPointLineTriangleBox(uint32_t current_swapchain_image_index)
    {
        RHIBuffer* vertex_buffers[] = {m_buffer_allocator->getVertexBuffer()};
        if (vertex_buffers[0] == nullptr)
            return;

        RHIDeviceSize offsets[] = {0};
        // 绑定顶点缓冲
        m_rhi->cmdBindVertexBuffersPFN(m_rhi->getCurrentCommandBuffer(), 0, 1, vertex_buffers, offsets);
        // 绑定索引缓冲
        m_rhi->cmdBindIndexBufferPFN(
            m_rhi->getCurrentCommandBuffer(), m_buffer_allocator->getIndexBuffer(), 0, RHI_INDEX_TYPE_UINT16);

        std::vector<DebugDrawPipeline*> vc_pipelines {
            m_debug_draw_pipeline[static_cast<uint32_t>(DebugDrawPipelineType::triangle)],
        };
        std::vector<size_t> vc_start_offsets {m_triangle_start_offset};
        std::vector<size_t> vc_end_offsets {m_triangle_end_offset};

        RHIClearValue clear_values[1];                    // 2
        clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f}; // use black color as clear value
        // clear_values[1].depthStencil = {1.0f, 0};
        RHIRenderPassBeginInfo renderpass_begin_info {};
        renderpass_begin_info.sType             = RHI_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_begin_info.renderArea.offset = {0, 0};
        renderpass_begin_info.renderArea.extent = m_rhi->getSwapchainInfo().extent;
        renderpass_begin_info.clearValueCount   = (sizeof(clear_values) / sizeof(clear_values[0]));
        renderpass_begin_info.pClearValues      = clear_values;

        size_t vc_pipelines_size = vc_pipelines.size();
        for (size_t i = 0; i < vc_pipelines_size; i++)
        {
            if (vc_end_offsets[i] - vc_start_offsets[i] == 0)
                continue;
            renderpass_begin_info.renderPass = vc_pipelines[i]->getFramebuffer().render_pass;
            renderpass_begin_info.framebuffer =
                vc_pipelines[i]->getFramebuffer().framebuffers[current_swapchain_image_index];
            m_rhi->cmdBeginRenderPassPFN(
                m_rhi->getCurrentCommandBuffer(), &renderpass_begin_info, RHI_SUBPASS_CONTENTS_INLINE);
            m_rhi->cmdBindPipelinePFN(m_rhi->getCurrentCommandBuffer(),
                                      RHI_PIPELINE_BIND_POINT_GRAPHICS, // graphics pipeline
                                      vc_pipelines[i]->getPipeline().pipeline);
            // m_rhi->cmdDraw(
            //     m_rhi->getCurrentCommandBuffer(), vc_end_offsets[i] - vc_start_offsets[i], 1, vc_start_offsets[i],
            //     0);
            m_rhi->cmdDrawIndexedPFN(m_rhi->getCurrentCommandBuffer(),
                                     static_cast<uint32_t>(m_buffer_allocator->getIndexCacheOffset()),
                                     1,
                                     0,
                                     0,
                                     0);
            m_rhi->cmdEndRenderPassPFN(m_rhi->getCurrentCommandBuffer());
        }
    }

    void DebugDrawManager::drawDebugObject(uint32_t current_swapchain_image_index)
    {
        prepareDrawBuffer();
        drawPointLineTriangleBox(current_swapchain_image_index);
    }

    void DebugDrawManager::draw(uint32_t current_swapchain_image_index)
    {
        drawDebugObject(current_swapchain_image_index);
    }

} // namespace Piccolo