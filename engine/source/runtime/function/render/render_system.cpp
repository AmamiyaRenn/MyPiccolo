#include "function/render/render_system.h"
#include "function/render/interface/rhi.h"
#include "function/render/interface/vulkan/vulkan_rhi.h"
#include "function/render/render_pipeline.h"

namespace Piccolo
{
    void RenderSystem::initialize(RenderSystemInitInfo init_info)
    {
        RHIInitInfo rhi_init_info;
        rhi_init_info.m_window_system = init_info.m_window_system;

        m_rhi = std::make_shared<VulkanRHI>();
        m_rhi->initialize(rhi_init_info);

        // initialize render pipeline
        m_render_pipeline        = std::make_shared<RenderPipeline>();
        m_render_pipeline->m_rhi = m_rhi;
    }

    void RenderSystem::tick(float delta_time)
    {
        // prepare render command context
        m_rhi->prepareContext();

        switch (m_render_pipeline_type)
        {
            case RENDER_PIPELINE_TYPE::FORWARD_PIPELINE:
                m_render_pipeline->forwardRender(m_rhi, m_render_resource);
                break;

            case RENDER_PIPELINE_TYPE::DEFERRED_PIPELINE:
            case RENDER_PIPELINE_TYPE::PIPELINE_TYPE_COUNT:
                break;
        }
    }
} // namespace Piccolo
