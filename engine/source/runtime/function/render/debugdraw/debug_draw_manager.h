#pragma once

#include "function/render/debugdraw/debug_draw_pipeline.h"
#include "function/render/interface/rhi.h"
#include <stdint.h>

namespace Piccolo
{
    class DebugDrawManager
    {
    public:
        void initialize();
        void setupPipelines();

    private:
        std::shared_ptr<RHI> m_rhi {nullptr};

        DebugDrawPipeline* m_debug_draw_pipeline[static_cast<uint32_t>(DebugDrawPipelineType::count)] = {};
    };
} // namespace Piccolo