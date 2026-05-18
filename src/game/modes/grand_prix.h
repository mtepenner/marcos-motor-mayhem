#pragma once
#include "race_state.h"
#include <array>
#include <string>

namespace mmm {

// Four-race Grand Prix cup
class GrandPrix {
public:
    static constexpr int kRacesPerCup = 4;

    // Points awarded for each finishing position (1st..8th)
    static constexpr std::array<int, 8> kPoints = {15, 12, 10, 8, 6, 4, 2, 1};

    struct Standing {
        uint32_t    kart_id{0};
        std::string kart_name;
        int         total_points{0};
    };

    // Called when a race finishes; records points
    void record_race(const std::vector<RaceResult>& results) {
        ++races_completed_;
        for (auto& r : results) {
            int pts = (r.placement > 0 && r.placement <= 8)
                      ? kPoints[static_cast<size_t>(r.placement - 1)] : 0;
            auto it = std::find_if(standings_.begin(), standings_.end(),
                                   [&](const Standing& s) { return s.kart_id == r.kart_id; });
            if (it == standings_.end()) {
                standings_.push_back({r.kart_id, r.kart_name, pts});
            } else {
                it->total_points += pts;
                it->kart_name     = r.kart_name;
            }
        }
        sort_standings();
    }

    bool is_complete() const { return races_completed_ >= kRacesPerCup; }
    int  races_completed() const { return races_completed_; }

    const std::vector<Standing>& standings() const { return standings_; }

    Standing* leader() {
        return standings_.empty() ? nullptr : &standings_[0];
    }

private:
    void sort_standings() {
        std::sort(standings_.begin(), standings_.end(),
                  [](const Standing& a, const Standing& b) {
                      return a.total_points > b.total_points;
                  });
    }

    std::vector<Standing> standings_;
    int                   races_completed_{0};
};

} // namespace mmm
