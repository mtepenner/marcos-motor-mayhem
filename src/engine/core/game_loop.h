#pragma once
#include <functional>
#include <chrono>
#include <cstdint>
#include <string>

namespace mmm {

// Fixed-timestep game loop.
// The update() callback receives a fixed dt; render() receives the
// interpolation alpha between the previous and current physics states.
class GameLoop {
public:
    using UpdateFn = std::function<void(float dt)>;
    using RenderFn = std::function<void(float alpha)>;
    using QuitFn   = std::function<bool()>;

    struct Config {
        float  target_fps{60.f};    // fixed physics tick rate
        int    max_steps{8};        // cap on how many updates per frame
        bool   headless{false};     // skip render callback (unit tests / servers)
        float  time_scale{1.f};     // slow-motion / fast-forward
    };

    GameLoop() = default;
    explicit GameLoop(const Config& cfg) : cfg_(cfg) {}

    // Runs until should_quit() returns true.
    void run(UpdateFn update, RenderFn render, QuitFn should_quit);

    // Single-step the fixed-timestep loop externally (useful in tests).
    // Returns number of update() calls performed.
    int step(UpdateFn update, float wall_dt);

    uint64_t tick_count()   const { return ticks_; }
    uint64_t frame_count()  const { return frames_; }
    float    fixed_dt()     const { return 1.f / cfg_.target_fps; }

private:
    Config   cfg_;
    float    accumulator_{0.f};
    uint64_t ticks_{0};
    uint64_t frames_{0};
};

} // namespace mmm
