#include "../core/game_loop.h"
#include "../render/renderer.h"
#include "../input/input_manager.h"
#include "../../game/modes/race_state.h"
#include <iostream>
#include <memory>

// Entry point for Marcos Motor Mayhem — wires the engine together.
int main(int /*argc*/, char** /*argv*/) {
    mmm::NullRenderer renderer; // swap for VulkanBackend at link time
    if (!renderer.init(1280, 720, "Marcos Motor Mayhem")) {
        std::cerr << "[main] renderer init failed\n";
        return 1;
    }

    mmm::InputManager input;
    mmm::RaceState    race;

    // Add a placeholder kart
    race.register_kart(1, "Marco Polo");

    mmm::GameLoop::Config config{};
    config.target_fps = 60.f;
    config.max_steps = 8;
    config.headless = false;
    config.time_scale = 1.f;

    mmm::GameLoop loop(config);

    int tick_limit = 60 * 30; // run for 30 simulated seconds then exit
    int ticks      = 0;

    loop.run(
        [&](float dt) {
            // Simulate idle acceleration input for demo
            mmm::RawInput raw;
            raw.trigger_r = 1.f; // full throttle
            input.poll(raw);

            race.tick(dt, input);
            ++ticks;
        },
        [&](float /*alpha*/) {
            renderer.begin_frame();
            // draw_mesh calls would go here
            renderer.end_frame();
        },
        [&]() { return ticks >= tick_limit; }
    );

    renderer.shutdown();
    std::cout << "[main] finished " << ticks << " ticks\n";
    return 0;
}
