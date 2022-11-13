#pragma once

#include "function/ui/window_ui.h"

namespace Piccolo
{
    class EditorUI : public WindowUI
    {
    public:
        EditorUI();
        virtual void initialize(WindowUIInitInfo init_info) override final;
        // override: 当需要重写基类方法时，使用override关键字，这样可以将重写的检查工作交给编译器会去做
        // final: 重写函数到此为止，不能继续被重写
    };
} // namespace Piccolo
