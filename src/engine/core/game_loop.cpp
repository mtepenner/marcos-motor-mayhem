#include "game_loop.h"
#include <thread>
#include <algorithm>

namespace mmm {

void GameLoop::run(UpdateFn update, RenderFn render, QuitFn should_quit) {
    using Clock = std::chrono::steady_clock;
    const float dt = fixed_dt() * cfg_.time_scale;

    auto prev = Clock::now();
    accumulator_ = 0.f;

    while (!should_quit()) {
        auto   now   = Clock::now();
        float  frame = std::chrono::duration<float>(now - prev).count();
        prev = now;

        // Clamp frame time to avoid spiral-of-death
        frame = std::min(frame, dt * cfg_.max_steps);
        accumulator_ += frame;

        while (accumulator_ >= dt) {
            update(dt);
            accumulator_ -= dt;
            ++ticks_;
        }

        if (!cfg_.headless) {
            float alpha = accumulator_ / dt;
            render(alpha);
        }

        ++frames_;

        // Yield to avoid 100% CPU on headless loops
        if (cfg_.headless)
            std::this_thread::yield();
    }
}

int GameLoop::step(UpdateFn update, float wall_dt) {
    const float dt = fixed_dt() * cfg_.time_scale;
    accumulator_ += wall_dt;

    int steps = 0;
    while (accumulator_ >= dt && steps < cfg_.max_steps) {
        update(dt);
        accumulator_ -= dt;
        ++ticks_;
        ++steps;
    }
    return steps;
}

} // namespace mmm
