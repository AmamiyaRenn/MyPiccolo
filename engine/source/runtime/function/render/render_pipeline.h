#pragma once

#include "function/render/interface/render_pipeline_base.h"

namespace Piccolo
{
    class RenderPipeline : public RenderPipelineBase
    {
    public:
        void forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource);
    };
} // namespace Piccolo
