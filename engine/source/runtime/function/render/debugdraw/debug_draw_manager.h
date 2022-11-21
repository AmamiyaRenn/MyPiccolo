#pragma once

#include "function/render/debugdraw/debug_draw_buffer.h"
#include "function/render/debugdraw/debug_draw_group.h"
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
        void updateAfterRecreateSwapchain();

        void draw(uint32_t current_swapchain_image_index);

    private:
        void prepareDrawBuffer();
        void drawDebugObject(uint32_t current_swapchain_image_index);
        void drawPointLineTriangleBox(uint32_t current_swapchain_image_index);

        std::shared_ptr<RHI> m_rhi {nullptr};

        DebugDrawPipeline*  m_debug_draw_pipeline[static_cast<uint32_t>(DebugDrawPipelineType::count)] = {};
        DebugDrawAllocator* m_buffer_allocator = nullptr; // buffer allocator
        DebugDrawGroup      m_debug_draw_group_for_render;

        size_t m_triangle_start_offset;
        size_t m_triangle_end_offset;
    };
} // namespace Piccolo