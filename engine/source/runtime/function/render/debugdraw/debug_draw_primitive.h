#pragma once

#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "function/render/interface/rhi_struct.h"

#include <array>

namespace Piccolo
{
    struct DebugDrawVertex
    {
        Vector2 pos;
        Vector3 color;

        static std::array<RHIVertexInputBindingDescription, 1> getBindingDescriptions()
        {
            std::array<RHIVertexInputBindingDescription, 1> binding_descriptions;

            binding_descriptions[0].binding   = 0; // index of the binding vertices
            binding_descriptions[0].stride    = sizeof(DebugDrawVertex);
            binding_descriptions[0].inputRate = RHI_VERTEX_INPUT_RATE_VERTEX; // 移动到下个顶点而非实例

            return binding_descriptions;
        }

        static std::array<RHIVertexInputAttributeDescription, 2> getAttributeDescriptions()
        {
            std::array<RHIVertexInputAttributeDescription, 2> attribute_descriptions {};

            // position
            attribute_descriptions[0].binding  = 0;
            attribute_descriptions[0].location = 0;                        // layout(location)
            attribute_descriptions[0].format   = RHI_FORMAT_R32G32_SFLOAT; // 三个通道都是float
            attribute_descriptions[0].offset   = offsetof(DebugDrawVertex, pos);

            // color
            attribute_descriptions[1].binding  = 0;
            attribute_descriptions[1].location = 1;
            attribute_descriptions[1].format   = RHI_FORMAT_R32G32B32_SFLOAT; // 四个通道都是float
            attribute_descriptions[1].offset   = offsetof(DebugDrawVertex, color);

            return attribute_descriptions;
        }
    };

    enum class DebugDrawTimeType : uint8_t
    {
        infinity,
        oneFrame,
        common
    };

    enum class DebugDrawPrimitiveType : uint8_t
    {
        point = 0,
        line,
        triangle,
        quad,
        draw_box,
        cylinder,
        sphere,
        capsule,
        text,
        count
    };

    class DebugDrawPrimitive
    {
    public:
        // DebugDrawTimeType m_time_type {DebugDrawTimeType::infinity};
        // float             m_life_time {0.f};
        // FillMode          m_fill_mode {_FillMode_wireframe};
        bool m_no_depth_test = false;

        // bool isTimeOut(float delta_time);
        // void setTime(float in_life_time);

    private:
        // bool m_rendered = false; // for one frame object
    };

    class DebugDrawTriangle : public DebugDrawPrimitive
    {
    public:
        DebugDrawVertex                     m_vertex[3];
        static const DebugDrawPrimitiveType m_k_type_enum_value = DebugDrawPrimitiveType::triangle;
    };
} // namespace Piccolo