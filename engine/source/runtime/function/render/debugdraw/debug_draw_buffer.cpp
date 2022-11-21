#include "function/render/debugdraw/debug_draw_buffer.h"
#include "function/global/global_context.h"

namespace Piccolo
{
    DebugDrawAllocator::DebugDrawAllocator() { m_rhi = g_runtime_global_context.m_render_system->getRHI(); }

    void DebugDrawAllocator::allocator()
    {
        uint64_t    vertex_buffer_size = static_cast<uint64_t>(m_vertex_cache.size() * sizeof(DebugDrawVertex));
        static bool created            = false;
        if (vertex_buffer_size > 0 && !created)
        {
            created = true;
            m_rhi->createBuffer(vertex_buffer_size,
                                RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                m_vertex_resource.buffer,
                                m_vertex_resource.memory);

            void* data;
            m_rhi->mapMemory(m_vertex_resource.memory, 0, vertex_buffer_size, 0, &data);
            memcpy(data, m_vertex_cache.data(), vertex_buffer_size);
            m_rhi->unmapMemory(m_vertex_resource.memory);
        }
    }

    size_t DebugDrawAllocator::cacheVertexs(const std::vector<DebugDrawVertex>& vertexs)
    {
        size_t offset = m_vertex_cache.size();
        m_vertex_cache.resize(offset + vertexs.size());
        size_t v_size = vertexs.size();
        for (size_t i = 0; i < v_size; i++)
            m_vertex_cache[i + offset] = vertexs[i];
        return offset;
    }

    void DebugDrawAllocator::clear()
    {
        // clearBuffer();
        m_vertex_cache.clear();
    }

    void DebugDrawAllocator::clearBuffer()
    {
        if (m_vertex_resource.buffer)
        {
            m_vertex_resource.buffer = nullptr;
            m_vertex_resource.memory = nullptr;
        }
    }
} // namespace Piccolo