#pragma once
#include "entity.h"
#include "../engine/physics/kart_dynamics.h"
#include <string>
#include <cstdint>

namespace mmm {

// Kart characters
enum class Character : uint8_t {
    MARCO   = 0,
    LINGUINI= 1,
    CARBONARA = 2,
    GELATO  = 3
};

// Each character has slightly different stats
inline KartStats stats_for(Character c) {
    switch (c) {
        case Character::MARCO:    return {18.f, 12.f, 18.f, 2.2f, 0.85f, 0.35f, 8.f, 1.5f, 420.f};
        case Character::LINGUINI: return {20.f, 10.f, 16.f, 2.4f, 0.80f, 0.45f, 7.f, 1.2f, 380.f};
        case Character::CARBONARA:return {16.f, 14.f, 20.f, 2.0f, 0.90f, 0.25f, 9.f, 1.8f, 480.f};
        case Character::GELATO:   return {22.f,  9.f, 14.f, 2.6f, 0.75f, 0.50f, 10.f,1.0f, 350.f};
        default: return {};
    }
}

class Kart final : public Entity {
public:
    Kart(uint32_t id, Character ch, const std::string& name)
        : Entity(id, EntityType::KART)
        , name_(name)
        , character_(ch)
    {
        state_.stats = stats_for(ch);
        state_.body.set_mass(state_.stats.weight);
    }

    void tick(float dt) override {
        kart_tick(state_, input_, dt);
        position_ = state_.body.position;
        velocity_ = state_.body.velocity;
    }

    void set_input(const KartInput& i)   { input_ = i; }
    KartState&       state()             { return state_; }
    const KartState& state()       const { return state_; }
    const std::string& name()      const { return name_; }
    Character          character() const { return character_; }

    // Race placement
    int  placement()          const { return placement_; }
    int  lap()                const { return lap_; }
    int  checkpoint()         const { return checkpoint_; }
    void set_placement(int p)       { placement_ = p; }
    void advance_checkpoint(int cp) { checkpoint_ = cp; }
    void advance_lap()              { ++lap_; checkpoint_ = 0; }

    AABB bounds() const override {
        return {position_ - Vec3{0.7f, 0.4f, 0.7f},
                position_ + Vec3{0.7f, 1.0f, 0.7f}};
    }

private:
    std::string name_;
    Character   character_;
    KartState   state_{};
    KartInput   input_{};
    int         lap_{0};
    int         checkpoint_{0};
    int         placement_{1};
};

} // namespace mmm
