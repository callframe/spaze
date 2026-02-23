#include "spaze/gfx.h"
#include "spaze/common.h"
#include <vulkan/vulkan_core.h>

static struct VkInstanceCreateInfo INSTANCE_CREATE_INFO = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = NULL,
    .flags = 0,
    .pApplicationInfo = NULL,
    .enabledLayerCount = 0,
    .ppEnabledLayerNames = NULL,
    .enabledExtensionCount = 0,
    .ppEnabledExtensionNames = NULL,
};

enum gfx_error_e gfx_init(struct gfx_s *gfx) {
  assert_notnull(gfx);

  VkInstance instance;
  VkResult result = vkCreateInstance(&INSTANCE_CREATE_INFO, NULL, &instance);

  if (result != VK_SUCCESS)
    return gfx_error_instance_create_failed;

  gfx->instance = instance;
  gfx->alive = true;

  return gfx_error_ok;
}

void gfx_deinit(struct gfx_s *gfx) {
  assert_notnull(gfx);

  if (!gfx->alive)
    return;

  vkDestroyInstance(gfx->instance, NULL);

  gfx->instance = VK_NULL_HANDLE;
  gfx->alive = false;
}
