#pragma once

#include "runtime/core/meta/reflection/reflection.h"

#include <cassert>
#include <cmath>

namespace Piccolo
{
    REFLECTION_TYPE(Vector2)
    CLASS(Vector2, Fields)
    {
        REFLECTION_BODY(Vector2);

    public:
        float x {0.f}, y {0.f};

        Vector2() = default;
        Vector2(float x_, float y_) : x(x_), y(y_) {}
        explicit Vector2(float scaler) : x(scaler), y(scaler) {}
        explicit Vector2(const float v[2]) : x(v[0]), y(v[1]) {}
        explicit Vector2(float* const r) : x(r[0]), y(r[1]) {}

        float operator[](size_t i) const
        {
            assert(i < 2);
            return (i == 0 ? x : y);
        }
        float& operator[](size_t i)
        {
            assert(i < 2);
            return (i == 0 ? x : y);
        }

        bool operator==(const Vector2& rhs) const { return (x == rhs.x && y == rhs.y); }
        bool operator!=(const Vector2& rhs) const { return (x != rhs.x || y != rhs.y); }

        // arithmetic operations
        Vector2 operator+(const Vector2& rhs) const { return Vector2(x + rhs.x, y + rhs.y); }
        Vector2 operator-(const Vector2& rhs) const { return Vector2(x - rhs.x, y - rhs.y); }
        Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
        Vector2 operator*(const Vector2& rhs) const { return Vector2(x * rhs.x, y * rhs.y); }
        Vector2 operator/(float scale) const
        {
            assert(scale != 0.0);
            float inv = 1.0f / scale;
            return Vector2(x * inv, y * inv);
        }
        Vector2        operator/(const Vector2& rhs) const { return Vector2(x / rhs.x, y / rhs.y); }
        const Vector2& operator+() const { return *this; }
        Vector2        operator-() const { return Vector2(-x, -y); }

        // overloaded operators to help Vector2
        friend Vector2 operator*(float scalar, const Vector2& rhs) { return Vector2(scalar * rhs.x, scalar * rhs.y); }
        friend Vector2 operator/(float fScalar, const Vector2& rhs)
        {
            return Vector2(fScalar / rhs.x, fScalar / rhs.y);
        }
        friend Vector2 operator+(const Vector2& lhs, float rhs) { return Vector2(lhs.x + rhs, lhs.y + rhs); }
        friend Vector2 operator+(float lhs, const Vector2& rhs) { return Vector2(lhs + rhs.x, lhs + rhs.y); }
        friend Vector2 operator-(const Vector2& lhs, float rhs) { return Vector2(lhs.x - rhs, lhs.y - rhs); }
        friend Vector2 operator-(float lhs, const Vector2& rhs) { return Vector2(lhs - rhs.x, lhs - rhs.y); }

        // arithmetic updates
        Vector2& operator+=(const Vector2& rhs)
        {
            x += rhs.x;
            y += rhs.y;

            return *this;
        }

        Vector2& operator+=(float scalar)
        {
            x += scalar, y += scalar;
            return *this;
        }
        Vector2& operator-=(const Vector2& rhs)
        {
            x -= rhs.x, y -= rhs.y;
            return *this;
        }
        Vector2& operator-=(float scalar)
        {
            x -= scalar, y -= scalar;
            return *this;
        }
        Vector2& operator*=(float scalar)
        {
            x *= scalar, y *= scalar;
            return *this;
        }
        Vector2& operator*=(const Vector2& rhs)
        {
            x *= rhs.x, y *= rhs.y;
            return *this;
        }
        Vector2& operator/=(float scalar)
        {
            assert(scalar != 0.0);
            float inv = 1.0f / scalar;
            x *= inv, y *= inv;
            return *this;
        }
        Vector2& operator/=(const Vector2& rhs)
        {
            x /= rhs.x, y /= rhs.y;
            return *this;
        }
    };
} // namespace Piccolo