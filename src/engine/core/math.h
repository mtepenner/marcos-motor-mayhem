#pragma once
#include <cmath>
#include <algorithm>

namespace mmm {

struct Vec3 {
    float x{0.f}, y{0.f}, z{0.f};

    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float s)       const { return {x*s,   y*s,   z*s};   }
    Vec3 operator/(float s)       const { return {x/s,   y/s,   z/s};   }
    Vec3& operator+=(const Vec3& o) { x+=o.x; y+=o.y; z+=o.z; return *this; }
    Vec3& operator*=(float s)       { x*=s;   y*=s;   z*=s;   return *this; }
    bool  operator==(const Vec3& o) const { return x==o.x && y==o.y && z==o.z; }

    float dot(const Vec3& o)  const { return x*o.x + y*o.y + z*o.z; }
    Vec3  cross(const Vec3& o) const {
        return {y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x};
    }
    float norm2()  const { return dot(*this); }
    float norm()   const { return std::sqrt(norm2()); }
    Vec3  normalized() const {
        float n = norm();
        return n > 1e-6f ? *this / n : Vec3{};
    }
};

struct Vec2 {
    float x{0.f}, y{0.f};
    Vec2 operator+(const Vec2& o) const { return {x+o.x, y+o.y}; }
    Vec2 operator*(float s)       const { return {x*s,   y*s};   }
    float dot(const Vec2& o)      const { return x*o.x + y*o.y;  }
    float norm()                  const { return std::sqrt(x*x + y*y); }
};

// Axis-aligned bounding box (3D)
struct AABB {
    Vec3 min{}, max{};

    Vec3 center()  const { return (min + max) * 0.5f; }
    Vec3 extents() const { return (max - min) * 0.5f; }

    bool contains(const Vec3& p) const {
        return p.x >= min.x && p.x <= max.x &&
               p.y >= min.y && p.y <= max.y &&
               p.z >= min.z && p.z <= max.z;
    }

    bool intersects(const AABB& o) const {
        return min.x <= o.max.x && max.x >= o.min.x &&
               min.y <= o.max.y && max.y >= o.min.y &&
               min.z <= o.max.z && max.z >= o.min.z;
    }
};

// Sphere
struct Sphere {
    Vec3  center{};
    float radius{1.f};

    bool intersects(const Sphere& o) const {
        Vec3 d = center - o.center;
        float dist2 = d.norm2();
        float radSum = radius + o.radius;
        return dist2 <= radSum * radSum;
    }

    bool intersects(const AABB& box) const {
        // Closest point on AABB to sphere center
        Vec3 closest = {
            std::clamp(center.x, box.min.x, box.max.x),
            std::clamp(center.y, box.min.y, box.max.y),
            std::clamp(center.z, box.min.z, box.max.z)
        };
        Vec3 d = closest - center;
        return d.norm2() <= radius * radius;
    }
};

} // namespace mmm
