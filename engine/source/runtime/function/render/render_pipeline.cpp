#include "function/render/render_pipeline.h"
#include "function/global/global_context.h"
#include "function/render/interface/vulkan/vulkan_rhi.h"

namespace Piccolo
{
    void RenderPipeline::forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource)
    {
        VulkanRHI* vulkan_rhi = static_cast<VulkanRHI*>(rhi.get());

        vulkan_rhi->waitForFences(); // fences wait for signal otherwise thread will be stopped

        if (!vulkan_rhi->prepareBeforePass())
            return;

        m_rhi->cmdSetViewportPFN(m_rhi->getCurrentCommandBuffer(), 0, 1, m_rhi->getSwapchainInfo().viewport);
        m_rhi->cmdSetScissorPFN(m_rhi->getCurrentCommandBuffer(), 0, 1, m_rhi->getSwapchainInfo().scissor);

        g_runtime_global_context.m_debugdraw_manager->draw(vulkan_rhi->m_current_swapchain_image_index);
        vulkan_rhi->submitRendering();
    }
} // namespace Piccolo