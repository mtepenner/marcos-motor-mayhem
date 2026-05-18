#include "vulkan_backend.h"
#ifdef MMM_VULKAN
// Full Vulkan implementation would live here.
// Stub — implementation gated by MMM_VULKAN define.
namespace mmm {
bool   VulkanBackend::init(uint32_t w, uint32_t h, const std::string&)     { width_=w; height_=h; return true; }
void   VulkanBackend::shutdown()                                             {}
void   VulkanBackend::begin_frame()                                          { draw_calls_=0; }
void   VulkanBackend::end_frame()                                            {}
void   VulkanBackend::draw_mesh(uint32_t,const Vec3&,float,float)            { ++draw_calls_; }
void   VulkanBackend::draw_ui_quad(float,float,float,float,uint32_t)         { ++draw_calls_; }
uint32_t VulkanBackend::upload_mesh(const float*,uint32_t,const uint32_t*,uint32_t) { return 1; }
} // namespace mmm
#endif
