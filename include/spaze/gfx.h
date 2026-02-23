#pragma once

#include <stdbool.h>
#include <vulkan/vulkan.h>

enum gfx_error_e {
  gfx_error_ok,
  gfx_error_instance_create_failed,
};

struct gfx_s {
  VkInstance instance;
  bool alive;
};

enum gfx_error_e gfx_init(struct gfx_s *gfx);
void gfx_deinit(struct gfx_s *gfx);
