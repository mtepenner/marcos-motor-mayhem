#pragma once
#include "rigidbody.h"
#include <cmath>
#include <algorithm>

namespace mmm {

// Kart input snapshot (normalised to [-1,1] or [0,1])
struct KartInput {
    float throttle{0.f};    // 0..1
    float brake{0.f};       // 0..1
    float steer{0.f};       // -1 (left) .. +1 (right)
    bool  drift{false};
    bool  boost{false};
};

// Tuning parameters per kart archetype
struct KartStats {
    float max_speed{18.f};          // m/s (~65 km/h)
    float acceleration{12.f};       // m/s²
    float brake_decel{18.f};        // m/s²
    float steer_speed{2.2f};        // rad/s max turning rate
    float grip{0.85f};              // lateral friction coefficient [0,1]
    float drift_factor{0.35f};      // how much lateral slip during drift
    float boost_speed_add{8.f};     // m/s bonus while boosting
    float boost_duration{1.5f};     // seconds
    float weight{420.f};            // kg (affects bump reactions)
};

// All mutable kart simulation state
struct KartState {
    RigidBody   body{};
    KartStats   stats{};

    float speed{0.f};           // scalar forward speed (may be negative for reverse)
    float slip_angle{0.f};      // lateral drift angle (radians)
    float boost_remaining{0.f}; // seconds

    bool  on_ground{true};
    bool  is_drifting{false};

    // Generic bitmask used by game logic (e.g., item effects, status ailments)
    uint32_t status_effect{0};
};

// Apply one physics tick to a kart given its input
inline void kart_tick(KartState& k, const KartInput& input, float dt) {
    KartStats& s  = k.stats;
    RigidBody& rb = k.body;

    // ── Throttle / Brake ───────────────────────────────────────────────
    float target_speed = input.throttle * s.max_speed;
    if (k.boost_remaining > 0.f) {
        target_speed += s.boost_speed_add;
        k.boost_remaining -= dt;
    }

    if (input.brake > 0.1f) {
        // Braking decelerates regardless of speed direction
        float brake_force = s.brake_decel * input.brake;
        if (k.speed > 0.f)
            k.speed -= brake_force * dt;
        else if (k.speed < 0.f)
            k.speed += brake_force * dt;
    } else {
        // Accelerate toward target speed
        float diff  = target_speed - k.speed;
        float accel = std::min(s.acceleration * dt, std::abs(diff));
        k.speed    += (diff >= 0.f ? 1.f : -1.f) * accel;
    }

    // ── Steering ──────────────────────────────────────────────────────
    // Steering rate scales down at high speed (understeer effect)
    float speed_factor  = 1.f - std::min(std::abs(k.speed) / (s.max_speed * 1.5f), 0.6f);
    float steer_delta   = input.steer * s.steer_speed * speed_factor * dt;

    // ── Drift ─────────────────────────────────────────────────────────
    if (input.drift && std::abs(k.speed) > 4.f) {
        k.is_drifting = true;
        // Exaggerate heading turn, reduce lateral grip
        rb.heading   += steer_delta * 1.6f;
        k.slip_angle  = std::clamp(k.slip_angle + input.steer * 0.05f, -0.7f, 0.7f);
    } else {
        k.is_drifting = false;
        rb.heading   += steer_delta;
        // Recover slip angle toward zero
        k.slip_angle  *= (1.f - s.grip * dt * 8.f);
    }

    // ── Build velocity from forward direction + lateral slip ──────────
    Vec3 fwd  = rb.forward();
    Vec3 rgt  = rb.right();

    float effective_speed = k.speed;
    // Lateral slip reduces effective forward speed slightly
    float lateral_speed   = k.slip_angle * std::abs(k.speed);

    rb.velocity = fwd * effective_speed + rgt * lateral_speed;

    // ── Ground friction drag ───────────────────────────────────────────
    float surface_drag = k.on_ground ? 0.01f : 0.002f;
    k.speed *= (1.f - surface_drag * dt * 60.f);

    // Integrate position
    rb.position += rb.velocity * dt;

    // ── Boost activation ──────────────────────────────────────────────
    if (input.boost && k.boost_remaining <= 0.f)
        k.boost_remaining = s.boost_duration;
}

} // namespace mmm
