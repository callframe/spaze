#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

enum gfx_device_error_e {
  gfx_device_error_ok,
  gfx_device_error_adapter_not_found,
  gfx_device_error_queue_not_found,
  gfx_device_error_device_create_failed,
};

struct gfx_device_s {
  VkPhysicalDevice adapter;
  VkDevice device;
  VkQueue queue;
  uint32_t family;
  bool alive;
};

#define assert_vkhandle(handle) assert((handle) != VK_NULL_HANDLE)

enum gfx_device_error_e gfx_device_init(struct gfx_device_s *device,
                                        VkInstance instance);
void gfx_device_deinit(struct gfx_device_s *device);

enum gfx_error_e {
  gfx_error_ok,
  gfx_error_instance_create_failed,
  gfx_error_device_get_failed,
};

struct gfx_s {
  VkInstance instance;
  struct gfx_device_s device;
  bool alive;
};

enum gfx_error_e gfx_init(struct gfx_s *gfx);
void gfx_deinit(struct gfx_s *gfx);

struct gfx_renderer_s {
    
};