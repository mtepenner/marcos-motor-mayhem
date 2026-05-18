#pragma once
#include "../entities/entity.h"
#include "item_manager.h"

namespace mmm {

class Projectile final : public Entity {
public:
    static constexpr float kSpeed        = 20.f; // m/s
    static constexpr float kHomingForce  = 8.f;  // rad/s max turn toward target

    Projectile(uint32_t id, ItemType type,
               const Vec3& pos, const Vec3& dir,
               uint32_t owner_id)
        : Entity(id, EntityType::PROJECTILE)
        , proj_type_(type)
        , owner_id_(owner_id)
    {
        position_ = pos;
        velocity_ = dir * kSpeed;
    }

    void tick(float dt) override {
        if (!alive_) return;

        if (proj_type_ == ItemType::HOMING_SHELL && target_pos_.has_value()) {
            Vec3 to_target = (*target_pos_ - position_).normalized();
            velocity_ = velocity_ + to_target * (kHomingForce * dt * kSpeed);
            // Renormalise to constant speed
            float v = velocity_.norm();
            if (v > 1e-6f) velocity_ = velocity_ / v * kSpeed;
        }

        position_ += velocity_ * dt;
        lifetime_  -= dt;
        if (lifetime_ <= 0.f) alive_ = false;
    }

    void set_target(const Vec3& pos) { target_pos_ = pos; }
    uint32_t owner_id()  const { return owner_id_; }
    ItemType proj_type() const { return proj_type_; }

private:
    ItemType             proj_type_;
    uint32_t             owner_id_;
    float                lifetime_{3.0f}; // seconds
    std::optional<Vec3>  target_pos_{};
};

} // namespace mmm
