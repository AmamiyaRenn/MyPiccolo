#include "function/render/debugdraw/debug_draw_group.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"

namespace Piccolo
{
    DebugDrawGroup::DebugDrawGroup()
    {
        Vector2 point0(0.0f, -0.5f);
        Vector2 point1(0.5f, 0.5f);
        Vector2 point2(-0.5f, 0.5f);
        Vector3 color0(1.0f, 1.0f, 1.0f);
        Vector3 color1(0.0f, 1.0f, 0.0f);
        Vector3 color2(0.0f, 0.0f, 1.0f);
        addTriangle(point0, point1, point2, color0, color1, color2);
    }

    void DebugDrawGroup::addTriangle(const Vector2& point0,
                                     const Vector2& point1,
                                     const Vector2& point2,
                                     const Vector3& color0,
                                     const Vector3& color1,
                                     const Vector3& color2)
    {
        DebugDrawTriangle triangle;
        // triangle.setTime(life_time);
        // triangle.m_fill_mode     = fillmod;
        // triangle.m_no_depth_test = no_depth_test;

        triangle.m_vertex[0].pos   = point0;
        triangle.m_vertex[0].color = color0;

        triangle.m_vertex[1].pos   = point1;
        triangle.m_vertex[1].color = color1;

        triangle.m_vertex[2].pos   = point2;
        triangle.m_vertex[2].color = color2;

        m_triangles.push_back(triangle);
    }

    size_t DebugDrawGroup::getTriangleCount(bool no_depth_test) const
    {
        size_t triangle_count = 0;
        for (const DebugDrawTriangle triangle : m_triangles)
            // if (triangle.m_fill_mode == FillMode::_FillMode_solid && triangle.m_no_depth_test == no_depth_test)
            triangle_count++;
        return triangle_count;
    }

    void DebugDrawGroup::writeTriangleData(std::vector<DebugDrawVertex>& vertexs, bool no_depth_test)
    {
        size_t vertexs_count = getTriangleCount(no_depth_test) * 3;
        vertexs.resize(vertexs_count);

        size_t current_index = 0;
        for (DebugDrawTriangle triangle : m_triangles)
        {
            // if (triangle.m_fill_mode == FillMode::_FillMode_solid && triangle.m_no_depth_test == no_depth_test)
            // {
            vertexs[current_index++] = triangle.m_vertex[0];
            vertexs[current_index++] = triangle.m_vertex[1];
            vertexs[current_index++] = triangle.m_vertex[2];
            // }
        }
    }
} // namespace Piccolo