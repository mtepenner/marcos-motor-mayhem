#pragma once
#include "../entities/kart.h"
#include <random>
#include <cstdint>

namespace mmm {

enum class ItemType : uint8_t {
    NONE        = 0,
    HOMING_SHELL= 1,   // tracks nearest kart ahead
    BANANA      = 2,   // drops a banana hazard
    BOOST_STAR  = 3,   // speed boost
    GREEN_SHELL = 4,   // straight projectile
    MEGA_HORN   = 5,   // clears nearby hazards
};

// Probability tables keyed by race placement (1-based).
// Lower placement (leading) → weaker items.
inline ItemType roll_item(int placement, uint32_t seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> d(0.f, 1.f);
    float r = d(rng);

    if (placement == 1) {
        // Leader gets weak items
        if (r < 0.50f) return ItemType::BANANA;
        if (r < 0.80f) return ItemType::GREEN_SHELL;
        return ItemType::BOOST_STAR;
    } else if (placement <= 4) {
        if (r < 0.30f) return ItemType::GREEN_SHELL;
        if (r < 0.60f) return ItemType::HOMING_SHELL;
        if (r < 0.80f) return ItemType::BOOST_STAR;
        return ItemType::MEGA_HORN;
    } else {
        // Tail-end karts get powerful items
        if (r < 0.20f) return ItemType::BOOST_STAR;
        if (r < 0.50f) return ItemType::HOMING_SHELL;
        if (r < 0.75f) return ItemType::MEGA_HORN;
        return ItemType::HOMING_SHELL;
    }
}

struct ItemSlot {
    ItemType item{ItemType::NONE};
    bool     held{false};
};

class ItemManager {
public:
    // Grant a random item to a kart based on placement
    void grant(Kart& kart, uint32_t seed = 42) {
        if (kart.state().status_effect != 0) return; // already has item effect
        ItemSlot& slot = slots_[kart.id() % kMaxKarts];
        if (!slot.held) {
            slot.item = roll_item(kart.placement(), seed + kart.id());
            slot.held = true;
        }
    }

    // Use the held item; returns item type (NONE if empty)
    ItemType use(uint32_t kart_id) {
        ItemSlot& slot = slots_[kart_id % kMaxKarts];
        if (!slot.held) return ItemType::NONE;
        ItemType it = slot.item;
        slot.item   = ItemType::NONE;
        slot.held   = false;
        return it;
    }

    bool has_item(uint32_t kart_id) const {
        return slots_[kart_id % kMaxKarts].held;
    }

private:
    static constexpr std::size_t kMaxKarts = 16;
    std::array<ItemSlot, kMaxKarts> slots_{};
};

} // namespace mmm
