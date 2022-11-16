#pragma once

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

    private:
        std::shared_ptr<RHI> m_rhi;
    };
} // namespace Piccolo
