#pragma once
#include <cstdint>
#include <array>
#include <string>

namespace mmm {

enum class GameAction : uint8_t {
    THROTTLE = 0,
    BRAKE,
    STEER_LEFT,
    STEER_RIGHT,
    DRIFT,
    ITEM_USE,
    PAUSE,
    COUNT
};

static constexpr int kActionCount = static_cast<int>(GameAction::COUNT);

struct ActionState {
    bool  pressed{false};   // true on first frame down
    bool  held{false};      // true while held
    bool  released{false};  // true on first frame up
    float axis{0.f};        // analog value [-1, 1] (keyboard = ±1)
};

// Raw hardware input snapshot (platform-agnostic)
struct RawInput {
    // Digital buttons (keyboard / controller face buttons)
    uint32_t buttons_down{0};  // bitmask of currently-held buttons
    uint32_t buttons_edge{0};  // bitmask of buttons that changed this frame

    // Analog axes
    float left_x{0.f};   // left stick or A/D keys
    float left_y{0.f};   // left stick or W/S keys (not heavily used in kart)
    float trigger_l{0.f};
    float trigger_r{0.f};
};

} // namespace mmm
