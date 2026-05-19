#include "../core/game_loop.h"
#include "../input/input_manager.h"
#include "../../game/modes/race_state.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

constexpr int kTrackCols = 68;
constexpr int kTrackRows = 24;
constexpr float kWorldHalfWidth = 32.f;
constexpr float kWorldHalfHeight = 18.f;
constexpr float kOuterRadiusX = 26.f;
constexpr float kOuterRadiusZ = 13.f;
constexpr float kInnerRadiusX = 15.f;
constexpr float kInnerRadiusZ = 6.f;
constexpr float kCenterlineRadiusX = 20.5f;
constexpr float kCenterlineRadiusZ = 9.5f;

float clamp01(float value) {
    return std::max(0.f, std::min(1.f, value));
}

void set_button(mmm::RawInput& raw, mmm::GameAction action) {
    raw.buttons_down |= (1u << static_cast<unsigned>(action));
}

#ifdef _WIN32
bool key_down(int key) {
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

void enable_ansi_console() {
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output == INVALID_HANDLE_VALUE) {
        return;
    }

    DWORD mode = 0;
    if (!GetConsoleMode(output, &mode)) {
        return;
    }

    SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
#else
bool key_down(int) {
    return false;
}

void enable_ansi_console() {}
#endif

class ConsoleSession {
public:
    ConsoleSession() {
        enable_ansi_console();
        std::cout << "\x1b[2J\x1b[H\x1b[?25l";
        std::cout.flush();
#ifdef _WIN32
        SetConsoleTitleA("Marcos Motor Mayhem - Terminal Edition");
#endif
    }

    ~ConsoleSession() {
        std::cout << "\x1b[?25h\n";
        std::cout.flush();
    }
};

mmm::RawInput sample_input(bool& quit_requested) {
    mmm::RawInput raw;

    bool throttle = key_down('W') || key_down(VK_UP);
    bool brake = key_down('S') || key_down(VK_DOWN);
    bool steer_left = key_down('A') || key_down(VK_LEFT);
    bool steer_right = key_down('D') || key_down(VK_RIGHT);
    bool drift = key_down(VK_SPACE);
    bool quit = key_down('Q') || key_down(VK_ESCAPE);

    raw.trigger_r = throttle ? 1.f : 0.f;
    raw.trigger_l = brake ? 1.f : 0.f;
    if (steer_left && !steer_right) {
        raw.left_x = -1.f;
    } else if (steer_right && !steer_left) {
        raw.left_x = 1.f;
    }

    if (throttle) {
        set_button(raw, mmm::GameAction::THROTTLE);
    }
    if (brake) {
        set_button(raw, mmm::GameAction::BRAKE);
    }
    if (steer_left) {
        set_button(raw, mmm::GameAction::STEER_LEFT);
    }
    if (steer_right) {
        set_button(raw, mmm::GameAction::STEER_RIGHT);
    }
    if (drift) {
        set_button(raw, mmm::GameAction::DRIFT);
    }

    quit_requested = quit_requested || quit;
    return raw;
}

bool on_track(float world_x, float world_z) {
    float outer = (world_x * world_x) / (kOuterRadiusX * kOuterRadiusX) +
                  (world_z * world_z) / (kOuterRadiusZ * kOuterRadiusZ);
    float inner = (world_x * world_x) / (kInnerRadiusX * kInnerRadiusX) +
                  (world_z * world_z) / (kInnerRadiusZ * kInnerRadiusZ);
    return outer <= 1.f && inner >= 1.f;
}

std::pair<int, int> project_to_grid(const mmm::Vec3& pos) {
    float u = clamp01((pos.x + kWorldHalfWidth) / (kWorldHalfWidth * 2.f));
    float v = clamp01((pos.z + kWorldHalfHeight) / (kWorldHalfHeight * 2.f));
    int col = static_cast<int>(u * static_cast<float>(kTrackCols - 1));
    int row = static_cast<int>((1.f - v) * static_cast<float>(kTrackRows - 1));
    return {row, col};
}

void seed_race(mmm::RaceState& race) {
    constexpr std::array<mmm::Vec3, 4> kStartPositions{{
        {-1.5f, 0.f, 8.5f},
        { 1.5f, 0.f, 7.5f},
        {-4.0f, 0.f, 6.0f},
        { 4.0f, 0.f, 5.0f}
    }};
    constexpr std::array<const char*, 4> kNames{{
        "Marco Polo",
        "Linguini Lightning",
        "Carbonara Comet",
        "Gelato Glide"
    }};
    constexpr std::array<mmm::Character, 4> kCharacters{{
        mmm::Character::MARCO,
        mmm::Character::LINGUINI,
        mmm::Character::CARBONARA,
        mmm::Character::GELATO
    }};

    for (std::size_t i = 0; i < kNames.size(); ++i) {
        race.register_kart(static_cast<uint32_t>(i + 1), kNames[i], kCharacters[i], kStartPositions[i]);
    }

    for (int i = 0; i < 8; ++i) {
        float angle = (static_cast<float>(i) / 8.f) * 6.2831853f;
        race.add_checkpoint({
            static_cast<uint32_t>(i),
            {std::sin(angle) * kCenterlineRadiusX, 0.f, std::cos(angle) * kCenterlineRadiusZ},
            4.5f
        });
    }
}

std::string render_frame(const mmm::RaceState& race) {
    std::vector<std::string> grid(kTrackRows, std::string(kTrackCols, ' '));

    for (int row = 0; row < kTrackRows; ++row) {
        for (int col = 0; col < kTrackCols; ++col) {
            float u = (static_cast<float>(col) + 0.5f) / static_cast<float>(kTrackCols);
            float v = 1.f - (static_cast<float>(row) + 0.5f) / static_cast<float>(kTrackRows);
            float world_x = -kWorldHalfWidth + u * (kWorldHalfWidth * 2.f);
            float world_z = -kWorldHalfHeight + v * (kWorldHalfHeight * 2.f);

            if (on_track(world_x, world_z)) {
                grid[row][col] = '.';
            }
        }
    }

    for (int i = 0; i < race.kart_count(); ++i) {
        const mmm::Kart* kart = race.kart(i);
        if (!kart) {
            continue;
        }

        auto [row, col] = project_to_grid(kart->position());
        char glyph = (i == 0) ? 'P' : static_cast<char>('1' + (i - 1));
        grid[row][col] = glyph;
    }

    const mmm::Kart* player = race.kart(0);
    std::ostringstream out;
    out << "\x1b[H";
    out << "MARCOS MOTOR MAYHEM - TERMINAL EDITION\n";
    out << "Controls: W throttle | S brake | A/D steer | Space drift | Q or Esc quit\n";
    out << "Legend: P=you, 1-3=AI, .=track\n\n";

    if (player) {
        out << "Lap " << std::min(player->lap() + 1, mmm::RaceState::kLapCount)
            << "/" << mmm::RaceState::kLapCount
            << "   Place " << player->placement() << "/" << race.kart_count()
            << "   Speed " << std::abs(player->state().speed)
            << " m/s   Time " << race.elapsed_s() << " s\n";
        out << "Position x=" << player->position().x
            << " z=" << player->position().z
            << "   Heading " << player->state().body.heading << " rad\n\n";
    }

    for (const auto& line : grid) {
        out << line << '\n';
    }

    out << '\n';
    out << "Standings:\n";
    for (int i = 0; i < race.kart_count(); ++i) {
        const mmm::Kart* kart = race.kart(i);
        if (!kart) {
            continue;
        }
        out << "  " << kart->placement() << ". " << kart->name()
            << "  lap " << std::min(kart->lap() + 1, mmm::RaceState::kLapCount)
            << "  cp " << kart->checkpoint()
            << "  speed " << std::abs(kart->state().speed) << "\n";
    }

    if (race.is_finished()) {
        out << "\nRace finished! Press Q or Esc to close.\n";
    }

    return out.str();
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
    ConsoleSession console;

    mmm::InputManager input;
    mmm::RaceState race;
    seed_race(race);

    mmm::GameLoop::Config config{};
    config.target_fps = 60.f;
    config.max_steps = 8;
    config.headless = false;
    config.time_scale = 1.f;

    mmm::GameLoop loop(config);
    bool quit_requested = false;

    loop.run(
        [&](float dt) {
            mmm::RawInput raw = sample_input(quit_requested);
            input.poll(raw);

            if (!race.is_finished()) {
                race.tick(dt, input);
            }
        },
        [&](float /*alpha*/) {
            std::cout << render_frame(race);
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        },
        [&]() { return quit_requested; }
    );

    std::cout << "\n[main] thanks for playing\n";
    return 0;
}
