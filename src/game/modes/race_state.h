#pragma once
#include "../entities/kart.h"
#include "../../engine/input/input_manager.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include <algorithm>
#include <memory>

namespace mmm {

struct CheckpointZone {
    uint32_t index{0};
    Vec3     center{};
    float    radius{8.f};
};

struct RaceResult {
    uint32_t    kart_id{0};
    std::string kart_name;
    int         placement{0};
    float       total_time_s{0.f};
};

class RaceState {
public:
    static constexpr int kLapCount = 3;

    // Register a kart before calling tick()
    void register_kart(uint32_t id, const std::string& name,
                       Character ch = Character::MARCO,
                       const Vec3& start_pos = {}) {
        auto kart = std::make_unique<Kart>(id, ch, name);
        kart->state().body.position = start_pos;
        karts_.emplace_back(std::move(kart));
    }

    void add_checkpoint(CheckpointZone cp) {
        checkpoints_.push_back(cp);
    }

    // Main update; input is applied to kart[0] (local player)
    void tick(float dt, const InputManager& input) {
        if (finished_) return;

        elapsed_s_ += dt;

        // Convert input to kart commands for player kart
        if (!karts_.empty()) {
            KartInput ki;
            ki.throttle = input.axis(GameAction::THROTTLE);
            if (ki.throttle < 0.05f && input.held(GameAction::THROTTLE))
                ki.throttle = 1.f;  // digital fallback
            ki.brake     = input.held(GameAction::BRAKE)      ? 1.f : 0.f;
            ki.steer     = input.axis(GameAction::STEER_RIGHT) -
                           input.axis(GameAction::STEER_LEFT);
            if (ki.steer == 0.f) {
                if (input.held(GameAction::STEER_RIGHT)) ki.steer =  1.f;
                if (input.held(GameAction::STEER_LEFT))  ki.steer = -1.f;
            }
            ki.drift = input.held(GameAction::DRIFT);
            karts_[0]->set_input(ki);
        }

        // AI karts: simple full-throttle along current checkpoint direction
        for (std::size_t i = 1; i < karts_.size(); ++i) {
            KartInput ai;
            ai.throttle = 0.85f;
            if (!checkpoints_.empty()) {
                auto& cp   = checkpoints_[karts_[i]->checkpoint() % checkpoints_.size()];
                Vec3 dir   = (cp.center - karts_[i]->position()).normalized();
                Vec3 fwd   = karts_[i]->state().body.forward();
                float dot  = fwd.dot(dir);
                Vec3 cross = fwd.cross(dir);
                ai.steer   = std::clamp(-cross.y, -1.f, 1.f);
                if (dot < 0.f) ai.throttle = 0.2f; // overshot — slow down
            }
            karts_[i]->set_input(ai);
        }

        // Update all karts
        for (auto& k : karts_) {
            k->tick(dt);
            check_checkpoints(*k);
        }

        update_placements();
    }

    bool  is_finished()    const { return finished_; }
    float elapsed_s()      const { return elapsed_s_; }
    int   kart_count()     const { return static_cast<int>(karts_.size()); }

    Kart* kart(int i) { return i < kart_count() ? karts_[i].get() : nullptr; }
    const Kart* kart(int i) const { return i < kart_count() ? karts_[i].get() : nullptr; }

    const std::vector<RaceResult>& results() const { return results_; }

private:
    void check_checkpoints(Kart& k) {
        if (checkpoints_.empty()) return;
        std::size_t next = static_cast<std::size_t>(k.checkpoint()) % checkpoints_.size();
        auto& cp = checkpoints_[next];
        Vec3 diff = k.position() - cp.center;
        if (diff.norm() < cp.radius) {
            int new_cp = static_cast<int>(next) + 1;
            if (new_cp >= static_cast<int>(checkpoints_.size())) {
                k.advance_lap();
                if (k.lap() >= kLapCount && !finished_) {
                    RaceResult res{k.id(), k.name(), k.placement(), elapsed_s_};
                    results_.push_back(res);
                    if (results_.size() >= karts_.size()) finished_ = true;
                }
            } else {
                k.advance_checkpoint(new_cp);
            }
        }
    }

    void update_placements() {
        // Sort by lap desc, then checkpoint desc, then reverse distance to next cp
        std::vector<Kart*> sorted;
        for (auto& k : karts_) sorted.push_back(k.get());
        std::sort(sorted.begin(), sorted.end(), [this](const Kart* a, const Kart* b) {
            if (a->lap() != b->lap()) return a->lap() > b->lap();
            if (a->checkpoint() != b->checkpoint()) return a->checkpoint() > b->checkpoint();
            if (checkpoints_.empty()) return false;
            auto& cp = checkpoints_[a->checkpoint() % checkpoints_.size()];
            float da = (a->position() - cp.center).norm();
            float db = (b->position() - cp.center).norm();
            return da < db;
        });
        for (int i = 0; i < static_cast<int>(sorted.size()); ++i)
            sorted[i]->set_placement(i + 1);
    }

    std::vector<std::unique_ptr<Kart>> karts_;
    std::vector<CheckpointZone>        checkpoints_;
    std::vector<RaceResult>            results_;
    float                              elapsed_s_{0.f};
    bool                               finished_{false};
};

} // namespace mmm
