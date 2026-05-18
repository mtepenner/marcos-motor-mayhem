#pragma once
#include "entity.h"
#include <string>

namespace mmm {

enum class HazardType : uint8_t {
    OIL_SLICK  = 0,   // reduces grip for 1.5 s
    COW        = 1,   // solid obstacle on Bovine Byway
    POTHOLE    = 2,   // brief airtime
    BANANA     = 3,   // spin-out
};

class Hazard final : public Entity {
public:
    Hazard(uint32_t id, HazardType ht, const Vec3& pos)
        : Entity(id, EntityType::HAZARD)
        , hazard_type_(ht)
    {
        position_ = pos;
    }

    void tick(float /*dt*/) override {
        // Static hazards don't move; lifetime ticks handled externally
    }

    HazardType hazard_type() const { return hazard_type_; }

    // Effect applied to a kart on contact
    void apply_effect(KartState& kart) const {
        switch (hazard_type_) {
            case HazardType::OIL_SLICK:
                kart.stats.grip = std::max(0.05f, kart.stats.grip * 0.3f);
                break;
            case HazardType::COW:
                // Bounce kart away (caller handles collision response)
                kart.body.velocity = kart.body.velocity * -0.5f;
                kart.speed *= -0.5f;
                break;
            case HazardType::POTHOLE:
                kart.on_ground = false;
                kart.body.velocity.y += 4.f;
                break;
            case HazardType::BANANA:
                kart.slip_angle = (kart.slip_angle >= 0.f ? 1.f : -1.f) * 0.65f;
                kart.speed     *= 0.6f;
                break;
        }
    }

    AABB bounds() const override {
        float r = (hazard_type_ == HazardType::COW) ? 1.2f : 0.5f;
        return {position_ - Vec3{r,r,r}, position_ + Vec3{r,r,r}};
    }

private:
    HazardType hazard_type_;
};

} // namespace mmm
