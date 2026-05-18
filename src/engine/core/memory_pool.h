#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <array>
#include <bitset>
#include <new>

namespace mmm {

// Fixed-size pool allocator.
// Manages a contiguous block; O(1) alloc/free via a free-list.
// Not thread-safe — intended for single-threaded game logic.
template<typename T, std::size_t Capacity>
class PoolAllocator {
public:
    static constexpr std::size_t kCapacity = Capacity;

    PoolAllocator() {
        // Build free list: each slot's first bytes hold the index of the next free slot
        for (std::size_t i = 0; i < Capacity - 1; ++i)
            free_list_[i] = static_cast<uint32_t>(i + 1);
        free_list_[Capacity - 1] = kNone;
        head_ = 0;
        used_ = 0;
    }

    // Construct an object in the pool, returning a pointer.
    // Returns nullptr if the pool is full.
    template<typename... Args>
    T* allocate(Args&&... args) {
        if (head_ == kNone) return nullptr;
        std::size_t idx = head_;
        head_ = free_list_[idx];
        live_[idx] = true;
        ++used_;
        return ::new (slots_[idx].data()) T(std::forward<Args>(args)...);
    }

    // Destroy and release an object back to the pool.
    void free(T* ptr) {
        std::size_t idx = slot_index(ptr);
        assert(idx < Capacity && live_[idx] && "double-free or invalid pointer");
        ptr->~T();
        live_[idx] = false;
        free_list_[idx] = head_;
        head_ = static_cast<uint32_t>(idx);
        --used_;
    }

    bool  is_live(std::size_t idx) const { return idx < Capacity && live_[idx]; }
    T*    at(std::size_t idx)            { return reinterpret_cast<T*>(slots_[idx].data()); }
    const T* at(std::size_t idx)   const { return reinterpret_cast<const T*>(slots_[idx].data()); }
    std::size_t used()             const { return used_; }
    std::size_t capacity()         const { return Capacity; }
    bool  full()                   const { return head_ == kNone; }
    bool  empty()                  const { return used_ == 0; }

private:
    static constexpr uint32_t kNone = UINT32_MAX;

    // Raw aligned storage for each T
    using Slot = std::array<std::byte, sizeof(T)>;
    alignas(alignof(T)) std::array<Slot, Capacity> slots_{};

    std::array<uint32_t, Capacity> free_list_{};
    std::bitset<Capacity>          live_{};
    uint32_t                       head_{0};
    std::size_t                    used_{0};

    std::size_t slot_index(const T* ptr) const {
        auto base = reinterpret_cast<const std::byte*>(slots_.data());
        auto raw  = reinterpret_cast<const std::byte*>(ptr);
        std::ptrdiff_t diff = raw - base;
        return static_cast<std::size_t>(diff) / sizeof(Slot);
    }
};

// ── Linear (frame) allocator ────────────────────────────────────────────────
// Bump-pointer; resets entirely at the end of each frame.
// Zero overhead allocation cost — ideal for per-frame scratch data.
template<std::size_t Bytes>
class LinearAllocator {
public:
    void* alloc(std::size_t size, std::size_t align = alignof(std::max_align_t)) {
        std::size_t ptr = (offset_ + align - 1) & ~(align - 1); // align up
        if (ptr + size > Bytes) return nullptr;
        offset_ = ptr + size;
        return buffer_.data() + ptr;
    }

    template<typename T, typename... Args>
    T* make(Args&&... args) {
        void* p = alloc(sizeof(T), alignof(T));
        if (!p) return nullptr;
        return ::new (p) T(std::forward<Args>(args)...);
    }

    void reset() { offset_ = 0; }
    std::size_t used()      const { return offset_; }
    std::size_t capacity()  const { return Bytes; }

private:
    alignas(std::max_align_t) std::array<std::byte, Bytes> buffer_{};
    std::size_t offset_{0};
};

} // namespace mmm
