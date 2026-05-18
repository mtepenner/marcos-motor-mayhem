#pragma once
#include "../engine/core/math.h"
#include <cstdint>
#include <string>

namespace mmm {

enum class EntityType : uint8_t {
    KART    = 0,
    HAZARD  = 1,
    ITEM_BOX= 2,
    PROJECTILE = 3
};

// Base class for every object in the game world
class Entity {
public:
    explicit Entity(uint32_t id, EntityType type)
        : id_(id), type_(type) {}

    virtual ~Entity() = default;

    virtual void tick(float dt) = 0;

    uint32_t    id()   const { return id_;   }
    EntityType  type() const { return type_; }

    Vec3& position()        { return position_; }
    const Vec3& position()  const { return position_; }
    const Vec3& velocity()  const { return velocity_; }
    Vec3& velocity()              { return velocity_; }

    bool is_alive()         const { return alive_; }
    void destroy()                { alive_ = false; }

    // Broad-phase AABB for collision queries
    virtual AABB bounds() const {
        float half = 0.5f;
        return {position_ - Vec3{half,half,half},
                position_ + Vec3{half,half,half}};
    }

protected:
    uint32_t   id_;
    EntityType type_;
    Vec3       position_{};
    Vec3       velocity_{};
    bool       alive_{true};
};

} // namespace mmm
