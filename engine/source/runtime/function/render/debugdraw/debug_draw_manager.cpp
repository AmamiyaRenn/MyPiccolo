#include "function/render/debugdraw/debug_draw_manager.h"
#include "function/global/global_context.h"
#include "function/render/debugdraw/debug_draw_pipeline.h"

namespace Piccolo
{
    void DebugDrawManager::initialize()
    {
        m_rhi = g_runtime_global_context.m_render_system->getRHI();
        setupPipelines();
    }

    void DebugDrawManager::setupPipelines()
    {
        for (uint8_t i = 0; static_cast<DebugDrawPipelineType>(i) < DebugDrawPipelineType::count; i++)
        {
            m_debug_draw_pipeline[i] = new DebugDrawPipeline(static_cast<DebugDrawPipelineType>(i));
            m_debug_draw_pipeline[i]->initialize();
        }
    }
} // namespace Piccolo