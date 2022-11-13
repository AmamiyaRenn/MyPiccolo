#pragma once

#include <memory>

#include "engine.h"

#include "editor/include/editor_ui.h"

namespace Piccolo
{
    class PiccoloEditor
    {
    public:
        PiccoloEditor();
        void initialize(PiccoloEngine* engine_runtime);
        void run();
        void shutdown();

    protected:
        std::shared_ptr<EditorUI> editor_ui;
        PiccoloEngine*            engine_runtime = nullptr;
    };
} // namespace Piccolo