#pragma once
#include "../entities/kart.h"
#include <chrono>
#include <cstdint>

namespace mmm {

// Temporary speed boost state modifier
struct BoostEffect {
    float   bonus_speed{0.f};   // m/s added to max_speed
    float   remaining{0.f};     // seconds left

    bool active() const { return remaining > 0.f; }

    void apply(float dt, KartState& kart) {
        if (!active()) return;
        remaining -= dt;
        if (remaining <= 0.f) {
            // Expire: undo speed bonus (stats were modified on start)
            kart.stats.max_speed -= bonus_speed;
            remaining  = 0.f;
            bonus_speed = 0.f;
        }
    }

    // Activate a boost on a kart
    void start(KartState& kart, float speed_add, float duration) {
        if (active()) {
            // Extend rather than reset
            remaining  += duration;
        } else {
            bonus_speed       = speed_add;
            remaining         = duration;
            kart.stats.max_speed += speed_add;
        }
        kart.boost_remaining = duration;
    }
};

} // namespace mmm
