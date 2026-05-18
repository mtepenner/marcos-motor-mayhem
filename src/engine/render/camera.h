#pragma once
#include "../core/math.h"

namespace mmm {

// Third-person follow camera.
// Call update() each frame with the kart's position and heading.
class Camera {
public:
    struct Config {
        float distance{8.f};     // behind kart
        float height{3.f};       // above kart
        float lag{6.f};          // position lag (higher = slower)
        float pitch_deg{15.f};   // fixed downward tilt
    };

    explicit Camera(Config cfg = {}) : cfg_(cfg) {}

    void update(const Vec3& kart_pos, float kart_yaw, float dt) {
        // Ideal camera position: behind and above kart
        Vec3 offset = {
            -std::sin(kart_yaw) * cfg_.distance,
             cfg_.height,
            -std::cos(kart_yaw) * cfg_.distance
        };
        Vec3 target = kart_pos + offset;

        // Exponential lag toward target
        float alpha = 1.f - std::exp(-cfg_.lag * dt);
        position_ = position_ + (target - position_) * alpha;
        look_at_  = kart_pos;
    }

    const Vec3& position() const { return position_; }
    const Vec3& look_at()  const { return look_at_;  }

private:
    Config cfg_;
    Vec3   position_{0.f, 5.f, -10.f};
    Vec3   look_at_{};
};

} // namespace mmm
