#pragma once

#include "function/render/debugdraw/debug_draw_primitive.h"

namespace Piccolo
{
    class DebugDrawGroup
    {
    public:
        DebugDrawGroup();
        void writeTriangleData(std::vector<DebugDrawVertex>& vertexs, bool no_depth_test);

    private:
        std::list<DebugDrawTriangle> m_triangles;
        void                         addTriangle(const Vector2& point0,
                                                 const Vector2& point1,
                                                 const Vector2& point2,
                                                 const Vector3& color0,
                                                 const Vector3& color1,
                                                 const Vector3& color2);
        size_t                       getTriangleCount(bool no_depth_test) const;
    };
} // namespace Piccolo