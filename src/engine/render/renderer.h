#pragma once
#include "../core/math.h"
#include <string>
#include <cstdint>

namespace mmm {

// Abstract rendering interface — swappable for Vulkan, OpenGL, or null (headless).
class Renderer {
public:
    virtual ~Renderer() = default;

    // Lifecycle
    virtual bool init(uint32_t width, uint32_t height, const std::string& title) = 0;
    virtual void shutdown() = 0;

    // Per-frame
    virtual void begin_frame()  = 0;
    virtual void end_frame()    = 0;

    // Primitive submission (simplified API for gameplay systems to use)
    virtual void draw_mesh(uint32_t mesh_id, const Vec3& pos,
                           float yaw, float scale) = 0;
    virtual void draw_ui_quad(float x, float y, float w, float h,
                              uint32_t color) = 0;

    // Resource helpers
    virtual uint32_t upload_mesh(const float* verts, uint32_t vert_count,
                                 const uint32_t* indices, uint32_t idx_count) = 0;

    // Stats
    virtual uint32_t draw_calls_last_frame() const { return 0; }
};

// Null renderer — used in headless mode and unit tests
class NullRenderer final : public Renderer {
public:
    bool init(uint32_t, uint32_t, const std::string&) override { return true; }
    void shutdown() override {}
    void begin_frame() override { ++frames_; draw_calls_ = 0; }
    void end_frame()   override {}
    void draw_mesh(uint32_t, const Vec3&, float, float) override { ++draw_calls_; }
    void draw_ui_quad(float, float, float, float, uint32_t) override { ++draw_calls_; }
    uint32_t upload_mesh(const float*, uint32_t, const uint32_t*, uint32_t) override {
        return ++next_id_;
    }
    uint32_t draw_calls_last_frame() const override { return draw_calls_; }

private:
    uint32_t frames_{0}, draw_calls_{0}, next_id_{0};
};

} // namespace mmm
