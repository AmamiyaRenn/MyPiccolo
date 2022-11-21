#pragma once

#include <cassert>

namespace Piccolo
{
    class Vector3
    {
    public:
        float x {0.f};
        float y {0.f};
        float z {0.f};

        Vector3() = default;
        Vector3(float x_, float y_, float z_) : x {x_}, y {y_}, z {z_} {};
        explicit Vector3(const float coords[3]) : x {coords[0]}, y {coords[1]}, z {coords[2]} {}

        float operator[](size_t i) const
        {
            assert(i < 3);
            return *(&x + i);
        }
        float& operator[](size_t i)
        {
            assert(i < 3);
            return *(&x + i);
        }

        bool operator==(const Vector3& rhs) const { return (x == rhs.x && y == rhs.y && z == rhs.z); }
        bool operator!=(const Vector3& rhs) const { return x != rhs.x || y != rhs.y || z != rhs.z; }

        // arithmetic operations
        Vector3 operator+(const Vector3& rhs) const { return Vector3(x + rhs.x, y + rhs.y, z + rhs.z); }
        Vector3 operator-(const Vector3& rhs) const { return Vector3(x - rhs.x, y - rhs.y, z - rhs.z); }
        Vector3 operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
        Vector3 operator*(const Vector3& rhs) const { return Vector3(x * rhs.x, y * rhs.y, z * rhs.z); }
        Vector3 operator/(float scalar) const
        {
            assert(scalar != 0.0);
            return Vector3(x / scalar, y / scalar, z / scalar);
        }
        Vector3 operator/(const Vector3& rhs) const
        {
            assert((rhs.x != 0 && rhs.y != 0 && rhs.z != 0));
            return Vector3(x / rhs.x, y / rhs.y, z / rhs.z);
        }
        const Vector3& operator+() const { return *this; }
        Vector3        operator-() const { return Vector3(-x, -y, -z); }

        // overloaded operators to help Vector3
        friend Vector3 operator*(float scalar, const Vector3& rhs)
        {
            return Vector3(scalar * rhs.x, scalar * rhs.y, scalar * rhs.z);
        }
        friend Vector3 operator/(float scalar, const Vector3& rhs)
        {
            assert(rhs.x != 0 && rhs.y != 0 && rhs.z != 0);
            return Vector3(scalar / rhs.x, scalar / rhs.y, scalar / rhs.z);
        }
        friend Vector3 operator+(const Vector3& lhs, float rhs)
        {
            return Vector3(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs);
        }
        friend Vector3 operator+(float lhs, const Vector3& rhs)
        {
            return Vector3(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z);
        }
        friend Vector3 operator-(const Vector3& lhs, float rhs)
        {
            return Vector3(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs);
        }
        friend Vector3 operator-(float lhs, const Vector3& rhs)
        {
            return Vector3(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z);
        }

        // arithmetic updates
        Vector3& operator+=(const Vector3& rhs)
        {
            x += rhs.x, y += rhs.y, z += rhs.z;
            return *this;
        }
        Vector3& operator+=(float scalar)
        {
            x += scalar, y += scalar, z += scalar;
            return *this;
        }
        Vector3& operator-=(const Vector3& rhs)
        {
            x -= rhs.x, y -= rhs.y, z -= rhs.z;
            return *this;
        }
        Vector3& operator-=(float scalar)
        {
            x -= scalar, y -= scalar, z -= scalar;
            return *this;
        }
        Vector3& operator*=(float scalar)
        {
            x *= scalar, y *= scalar, z *= scalar;
            return *this;
        }
        Vector3& operator*=(const Vector3& rhs)
        {
            x *= rhs.x, y *= rhs.y, z *= rhs.z;
            return *this;
        }
        Vector3& operator/=(float scalar)
        {
            assert(scalar != 0.0);
            x /= scalar, y /= scalar, z /= scalar;
            return *this;
        }
        Vector3& operator/=(const Vector3& rhs)
        {
            assert(rhs.x != 0 && rhs.y != 0 && rhs.z != 0);
            x /= rhs.x, y /= rhs.y, z /= rhs.z;
            return *this;
        }
    };
} // namespace Piccolo
