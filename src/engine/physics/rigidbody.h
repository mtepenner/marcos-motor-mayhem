#pragma once
#include "../core/math.h"

namespace mmm {

struct RigidBody {
    // Physical properties
    float mass{1.0f};       // kg
    float inv_mass{1.0f};   // 1/mass, 0 = infinite mass (static)
    float drag{0.02f};      // linear drag coefficient
    float angular_drag{0.1f};

    // State
    Vec3  position{};
    Vec3  velocity{};
    Vec3  acceleration{};   // accumulated this frame
    float heading{0.f};     // yaw in radians (Y-up world)
    float angular_vel{0.f}; // rad/s around Y axis

    // Apply a force (accumulates until integrate())
    void apply_force(const Vec3& force) {
        if (inv_mass <= 0.f) return;
        acceleration += force * inv_mass;
    }

    void apply_impulse(const Vec3& impulse) {
        if (inv_mass <= 0.f) return;
        velocity += impulse * inv_mass;
    }

    // Euler integration step
    void integrate(float dt) {
        if (inv_mass <= 0.f) return;

        // Semi-implicit Euler
        velocity     += acceleration * dt;
        velocity     *= (1.f - drag * dt);  // linear damping
        position     += velocity * dt;

        heading      += angular_vel * dt;
        angular_vel  *= (1.f - angular_drag * dt);

        // Clear accumulated forces
        acceleration = {};
    }

    void set_mass(float m) {
        mass     = m;
        inv_mass = (m > 0.f) ? 1.f / m : 0.f;
    }

    // Forward direction from heading
    Vec3 forward() const { return {std::sin(heading), 0.f, std::cos(heading)}; }
    Vec3 right()   const { return {std::cos(heading), 0.f, -std::sin(heading)}; }
};

} // namespace mmm
