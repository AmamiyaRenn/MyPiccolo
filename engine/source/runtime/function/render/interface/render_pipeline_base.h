#pragma once

#include "function/render/interface/rhi.h"
#include "function/render/render_resouce_base.h"

#include <memory>
#include <vector>

namespace Piccolo
{
    class RenderPipelineBase
    {
        friend class RenderSystem;

    public:
        virtual void forwardRender(std::shared_ptr<RHI> rhi, std::shared_ptr<RenderResourceBase> render_resource);

    protected:
        std::shared_ptr<RHI> m_rhi;
    };
} // namespace Piccolo
