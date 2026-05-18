#pragma once
#include "renderer.h"
#include <memory>
#include <string>

namespace mmm {

// VulkanBackend is only compiled when MMM_VULKAN=1 is set.
// In CI (no Vulkan SDK) we conditionally compile this as a stub so the
// translation unit exists but the class body is empty.

#ifdef MMM_VULKAN
#include <vulkan/vulkan.h>

class VulkanBackend final : public Renderer {
public:
    bool init(uint32_t width, uint32_t height, const std::string& title) override;
    void shutdown() override;
    void begin_frame() override;
    void end_frame()   override;
    void draw_mesh(uint32_t mesh_id, const Vec3& pos, float yaw, float scale) override;
    void draw_ui_quad(float x, float y, float w, float h, uint32_t color) override;
    uint32_t upload_mesh(const float* verts, uint32_t vert_count,
                         const uint32_t* indices, uint32_t idx_count) override;

private:
    VkInstance       instance_{VK_NULL_HANDLE};
    VkDevice         device_{VK_NULL_HANDLE};
    VkQueue          gfx_queue_{VK_NULL_HANDLE};
    uint32_t         gfx_family_{0};
    uint32_t         width_{0}, height_{0};
    uint32_t         draw_calls_{0};
};

#else // MMM_VULKAN not defined — forward as NullRenderer alias for this TU

using VulkanBackend = NullRenderer;

#endif // MMM_VULKAN

} // namespace mmm
