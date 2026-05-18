#pragma once
#include "../core/math.h"
#include <optional>

namespace mmm {

// Collision contact data returned when two shapes intersect
struct Contact {
    Vec3  normal{};          // points from b toward a (unit vector)
    float penetration{0.f};  // overlap depth
    Vec3  point{};           // approximate contact point
};

// AABB vs AABB — separating axis test
// Returns contact data if overlapping, else std::nullopt
inline std::optional<Contact> aabb_aabb(const AABB& a, const AABB& b) {
    if (!a.intersects(b)) return std::nullopt;

    // Find axis of minimum penetration
    float dx1 = b.max.x - a.min.x;
    float dx2 = a.max.x - b.min.x;
    float dy1 = b.max.y - a.min.y;
    float dy2 = a.max.y - b.min.y;
    float dz1 = b.max.z - a.min.z;
    float dz2 = a.max.z - b.min.z;

    float px = std::min(dx1, dx2);
    float py = std::min(dy1, dy2);
    float pz = std::min(dz1, dz2);

    Contact c;
    if (px <= py && px <= pz) {
        c.penetration = px;
        c.normal = (dx1 < dx2) ? Vec3{-1,0,0} : Vec3{1,0,0};
    } else if (py <= px && py <= pz) {
        c.penetration = py;
        c.normal = (dy1 < dy2) ? Vec3{0,-1,0} : Vec3{0,1,0};
    } else {
        c.penetration = pz;
        c.normal = (dz1 < dz2) ? Vec3{0,0,-1} : Vec3{0,0,1};
    }
    c.point = a.center() + c.normal * (c.penetration * 0.5f);
    return c;
}

// Sphere vs Sphere
inline std::optional<Contact> sphere_sphere(const Sphere& a, const Sphere& b) {
    Vec3 d = a.center - b.center;
    float dist2 = d.norm2();
    float radSum = a.radius + b.radius;

    if (dist2 > radSum * radSum) return std::nullopt;

    float dist = std::sqrt(dist2);
    Contact c;
    c.normal      = dist > 1e-6f ? d / dist : Vec3{0, 1, 0};
    c.penetration = radSum - dist;
    c.point       = b.center + c.normal * b.radius;
    return c;
}

// Sphere vs AABB
inline std::optional<Contact> sphere_aabb(const Sphere& s, const AABB& box) {
    if (!s.intersects(box)) return std::nullopt;

    Vec3 closest = {
        std::clamp(s.center.x, box.min.x, box.max.x),
        std::clamp(s.center.y, box.min.y, box.max.y),
        std::clamp(s.center.z, box.min.z, box.max.z)
    };

    Vec3  d    = s.center - closest;
    float dist = d.norm();

    Contact c;
    c.normal      = dist > 1e-6f ? d / dist : Vec3{0, 1, 0};
    c.penetration = s.radius - dist;
    c.point       = closest;
    return c;
}

// Ray-AABB slab test; returns t (distance along ray) or -1 on miss
inline float ray_aabb(const Vec3& origin, const Vec3& dir, const AABB& box) {
    float tmin = 0.f, tmax = 1e30f;
    const float* o = &origin.x;
    const float* d = &dir.x;
    const float* bmin = &box.min.x;
    const float* bmax = &box.max.x;

    for (int i = 0; i < 3; ++i) {
        if (std::abs(d[i]) < 1e-6f) {
            if (o[i] < bmin[i] || o[i] > bmax[i]) return -1.f;
        } else {
            float t1 = (bmin[i] - o[i]) / d[i];
            float t2 = (bmax[i] - o[i]) / d[i];
            if (t1 > t2) std::swap(t1, t2);
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            if (tmin > tmax) return -1.f;
        }
    }
    return tmin;
}

} // namespace mmm
