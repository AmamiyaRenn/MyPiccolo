#pragma once

#include "function/render/debugdraw/debug_draw_primitive.h"
#include "function/render/interface/rhi.h"

namespace Piccolo
{
    class DebugDrawAllocator
    {
    public:
        DebugDrawAllocator();

        void allocator();
        void clear();
        void clearBuffer();

        size_t cacheVertexs(const std::vector<DebugDrawVertex>& vertexs);

        size_t     getVertexCacheOffset() const { return m_vertex_cache.size(); }
        RHIBuffer* getVertexBuffer() const { return m_vertex_resource.buffer; }

    private:
        std::shared_ptr<RHI> m_rhi;

        struct Resource
        {
            RHIBuffer*       buffer = nullptr;
            RHIDeviceMemory* memory = nullptr;
        };
        // changeable resource
        Resource                     m_vertex_resource; // vk顶点资源指针
        std::vector<DebugDrawVertex> m_vertex_cache;    // 顶点数据cpu暂存
    };
} // namespace Piccolo