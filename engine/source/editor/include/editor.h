#pragma once

#include "engine.h"

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
        PiccoloEngine* engine_runtime = nullptr;
    };
} // namespace Piccolo