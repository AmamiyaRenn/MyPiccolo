#pragma once

#include "function/render/interface/render_pipeline_base.h"
#include "function/render/interface/rhi.h"
#include "function/render/window_system.h"

#include <memory>

namespace Piccolo
{
    struct RenderSystemInitInfo
    {
        std::shared_ptr<WindowSystem> m_window_system;
    };

    class RenderSystem
    {
    public:
        void initialize(RenderSystemInitInfo init_info);
        void tick(float delta_time);

        std::shared_ptr<RHI> getRHI() const { return m_rhi; }

    private:
        RENDER_PIPELINE_TYPE m_render_pipeline_type {RENDER_PIPELINE_TYPE::FORWARD_PIPELINE};

        std::shared_ptr<RHI>                m_rhi;
        std::shared_ptr<RenderResourceBase> m_render_resource;
        std::shared_ptr<RenderPipelineBase> m_render_pipeline;
    };
} // namespace Piccolo
