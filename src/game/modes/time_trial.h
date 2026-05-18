#pragma once
#include "race_state.h"
#include <chrono>
#include <optional>
#include <vector>
#include <algorithm>

namespace mmm {

// Ghost replay frame — stores a lightweight snapshot of one kart
struct GhostFrame {
    float  time_s{0.f};
    Vec3   position{};
    float  heading{0.f};
    float  speed{0.f};
};

// Single-player time trial: records a ghost, compares laps
class TimeTrial {
public:
    explicit TimeTrial(uint32_t kart_id) : kart_id_(kart_id) {}

    void start() {
        current_frames_.clear();
        lap_start_s_ = 0.f;
        elapsed_s_   = 0.f;
        running_      = true;
    }

    // Call each physics tick with the kart's current state
    void tick(float dt, const KartState& state) {
        if (!running_) return;
        elapsed_s_ += dt;

        // Record at ~10 Hz (every 6th tick at 60 fps)
        frame_accum_ += dt;
        if (frame_accum_ >= 0.1f) {
            frame_accum_ = 0.f;
            current_frames_.push_back({elapsed_s_,
                                        state.body.position,
                                        state.body.heading,
                                        state.speed});
        }
    }

    // Trigger when kart crosses the start/finish line
    void on_lap_complete() {
        float lap_time = elapsed_s_ - lap_start_s_;
        lap_times_.push_back(lap_time);

        if (!best_ghost_.has_value() || lap_time < best_lap_time_) {
            best_lap_time_ = lap_time;
            best_ghost_    = current_frames_;
        }

        lap_start_s_ = elapsed_s_;
    }

    void stop() { running_ = false; }

    float elapsed_s()    const { return elapsed_s_;    }
    float best_lap()     const { return best_lap_time_; }
    int   lap_count()    const { return static_cast<int>(lap_times_.size()); }
    float lap_time(int i) const {
        return (i >= 0 && i < lap_count()) ? lap_times_[static_cast<size_t>(i)] : 0.f;
    }

    const std::optional<std::vector<GhostFrame>>& ghost() const { return best_ghost_; }

private:
    uint32_t kart_id_;
    float    elapsed_s_{0.f};
    float    lap_start_s_{0.f};
    float    frame_accum_{0.f};
    float    best_lap_time_{1e30f};
    bool     running_{false};

    std::vector<GhostFrame>                  current_frames_;
    std::vector<float>                       lap_times_;
    std::optional<std::vector<GhostFrame>>   best_ghost_;
};

} // namespace mmm
