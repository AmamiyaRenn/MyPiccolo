#pragma once

#include "function/render/render_system.h"
#include "function/render/window_system.h"
#include <memory>

namespace Piccolo
{
    // 窗口界面初始化信息
    struct WindowUIInitInfo
    {
        std::shared_ptr<WindowSystem> m_window_system;
        std::shared_ptr<RenderSystem> m_render_system;
    };

    class WindowUI
    {
    public:
        virtual void initialize(WindowUIInitInfo init_info) = 0;
        // 纯虚函数：使派生类仅仅只是继承函数的接口(本类作为抽象类而存在)
    };
} // namespace Piccolo
