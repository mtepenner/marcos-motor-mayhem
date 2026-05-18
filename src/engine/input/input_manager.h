#pragma once
#include "controller_map.h"
#include <array>
#include <functional>

namespace mmm {

// InputManager translates RawInput into ActionState for each GameAction.
// Call poll() each frame with the latest hardware snapshot.
class InputManager {
public:
    // Translate raw input into per-action states
    void poll(const RawInput& raw) {
        prev_buttons_ = cur_buttons_;
        cur_buttons_  = raw.buttons_down;

        uint32_t just_down = cur_buttons_  & ~prev_buttons_;
        uint32_t just_up   = ~cur_buttons_ & prev_buttons_;

        for (int i = 0; i < kActionCount; ++i) {
            auto& a = actions_[i];
            auto  bit = 1u << i;
            a.held     = (cur_buttons_  & bit) != 0;
            a.pressed  = (just_down     & bit) != 0;
            a.released = (just_up       & bit) != 0;
        }

        // Map analog axes
        actions_[int(GameAction::THROTTLE)].axis   = raw.trigger_r;
        actions_[int(GameAction::BRAKE)].axis      = raw.trigger_l;
        actions_[int(GameAction::STEER_LEFT)].axis = (raw.left_x < 0.f) ? -raw.left_x : 0.f;
        actions_[int(GameAction::STEER_RIGHT)].axis= (raw.left_x > 0.f) ?  raw.left_x : 0.f;
    }

    const ActionState& action(GameAction a) const {
        return actions_[static_cast<int>(a)];
    }

    bool held(GameAction a)     const { return actions_[int(a)].held;     }
    bool pressed(GameAction a)  const { return actions_[int(a)].pressed;  }
    bool released(GameAction a) const { return actions_[int(a)].released; }
    float axis(GameAction a)    const { return actions_[int(a)].axis;     }

private:
    std::array<ActionState, kActionCount> actions_{};
    uint32_t cur_buttons_{0};
    uint32_t prev_buttons_{0};
};

} // namespace mmm
