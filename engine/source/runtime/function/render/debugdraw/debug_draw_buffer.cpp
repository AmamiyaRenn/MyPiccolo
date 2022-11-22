#include "function/render/debugdraw/debug_draw_buffer.h"
#include "function/global/global_context.h"

namespace Piccolo
{
    DebugDrawAllocator::DebugDrawAllocator() { m_rhi = g_runtime_global_context.m_render_system->getRHI(); }

    void DebugDrawAllocator::allocator()
    { // FIXME: 还原
        uint64_t    vertex_buffer_size = static_cast<uint64_t>(m_vertex_cache.size() * sizeof(DebugDrawVertex));
        static bool created            = false;
        if (vertex_buffer_size > 0 && !created)
        {
            created = true;
            // m_rhi->createBuffer(vertex_buffer_size,
            //                     RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            //
            //                     m_vertex_resource.buffer,
            //                     m_vertex_resource.memory);

            // void* data;
            // m_rhi->mapMemory(m_vertex_resource.memory, 0, vertex_buffer_size, 0, &data);
            // memcpy(data, m_vertex_cache.data(), vertex_buffer_size);
            // m_rhi->unmapMemory(m_vertex_resource.memory);

            RHIBuffer*       staging_buffer;
            RHIDeviceMemory* staging_buffer_memory;

            m_rhi->createBuffer(vertex_buffer_size,
                                RHI_BUFFER_USAGE_TRANSFER_SRC_BIT, // 传输的源
                                RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT, // 使用RAM
                                staging_buffer,
                                staging_buffer_memory);

            void* data;
            m_rhi->mapMemory(staging_buffer_memory, 0, vertex_buffer_size, 0, &data);
            memcpy(data, m_vertex_cache.data(), vertex_buffer_size);
            m_rhi->unmapMemory(staging_buffer_memory);

            m_rhi->createBuffer(vertex_buffer_size,
                                RHI_BUFFER_USAGE_TRANSFER_DST_BIT |
                                    RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT, // 传输的目的地|用做顶点缓冲
                                RHI_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,   // 使用设备RAM
                                m_vertex_resource.buffer,
                                m_vertex_resource.memory);

            m_rhi->copyBuffer(staging_buffer, m_vertex_resource.buffer, 0, 0, vertex_buffer_size);

            m_rhi->destroyBuffer(staging_buffer);
            m_rhi->freeMemory(staging_buffer_memory);
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