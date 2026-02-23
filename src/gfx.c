#include "spaze/gfx.h"
#include "mimalloc.h"
#include "spaze/common.h"
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#define APPLICATION_NAME "spaze"
#define APPLICATION_VERSION VK_MAKE_VERSION(0, 1, 0)

static const float QUEUE_PRIORITY = 1.0f;

static const struct VkApplicationInfo APPLICATION_INFO = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = APPLICATION_NAME,
    .applicationVersion = APPLICATION_VERSION,
    .pEngineName = APPLICATION_NAME,
    .engineVersion = APPLICATION_VERSION,
    .apiVersion = VK_API_VERSION_1_3,
};

static const struct VkInstanceCreateInfo INSTANCE_CREATE_INFO = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &APPLICATION_INFO,
    .enabledLayerCount = 0,
    .ppEnabledLayerNames = NULL,
    .enabledExtensionCount = 0,
    .ppEnabledExtensionNames = NULL,
};

static const struct VkDeviceQueueCreateInfo DEVICE_QUEUE_CREATE_INFO = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = 0,
    .queueCount = 1,
    .pQueuePriorities = &QUEUE_PRIORITY,
};

static VkPhysicalDevice gfx_device_choose_adapter(VkInstance instance) {
  uint32_t nadapters = 0;
  VkResult result = vkEnumeratePhysicalDevices(instance, &nadapters, NULL);
  if (result != VK_SUCCESS || nadapters == 0)
    return VK_NULL_HANDLE;

  VkPhysicalDevice *adapters = mi_malloc(sizeof(VkPhysicalDevice) * nadapters);
  result = vkEnumeratePhysicalDevices(instance, &nadapters, adapters);
  if (result != VK_SUCCESS) {
    mi_free(adapters);
    return VK_NULL_HANDLE;
  }

  VkPhysicalDevice adapter = adapters[0];
  mi_free(adapters);
  return adapter;
}

static bool gfx_device_choose_queue_family(VkPhysicalDevice adapter,
                                           uint32_t *out_family) {
  uint32_t nqueues = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(adapter, &nqueues, NULL);

  VkQueueFamilyProperties *queues =
      mi_malloc(sizeof(VkQueueFamilyProperties) * nqueues);

  vkGetPhysicalDeviceQueueFamilyProperties(adapter, &nqueues, queues);

  uint32_t family = 0;
  bool found_family = false;
  for (uint32_t i = 0; i < nqueues; i++) {
    bool has_graphics = (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
    if (!has_graphics)
      continue;

    family = i;
    found_family = true;
    break;
  }

  mi_free(queues);
  *out_family = family;
  return found_family;
}

enum gfx_device_error_e gfx_device_init(struct gfx_device_s *device,
                                        VkInstance instance) {
  assert_notnull(device);
  assert_vkhandle(instance);
  memset(device, 0, sizeof(*device));

  VkPhysicalDevice adapter = gfx_device_choose_adapter(instance);
  if (adapter == VK_NULL_HANDLE)
    return gfx_device_error_adapter_not_found;

  uint32_t family;
  if (!gfx_device_choose_queue_family(adapter, &family))
    return gfx_device_error_queue_not_found;

  struct VkDeviceQueueCreateInfo queue_create_info = DEVICE_QUEUE_CREATE_INFO;
  queue_create_info.queueFamilyIndex = family;

  struct VkDeviceCreateInfo device_create_info = {0};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.queueCreateInfoCount = 1;
  device_create_info.pQueueCreateInfos = &queue_create_info;

  VkDevice devhandle;
  VkResult result =
      vkCreateDevice(adapter, &device_create_info, NULL, &devhandle);
  if (result != VK_SUCCESS)
    return gfx_device_error_device_create_failed;

  VkQueue queue;
  vkGetDeviceQueue(devhandle, family, 0, &queue);

  device->adapter = adapter;
  device->device = devhandle;
  device->queue = queue;
  device->family = family;
  device->alive = true;

  return gfx_device_error_ok;
}

void gfx_device_deinit(struct gfx_device_s *device) {
  assert_notnull(device);

  if (!device->alive)
    return;

  vkDestroyDevice(device->device, NULL);

  device->adapter = VK_NULL_HANDLE;
  device->device = VK_NULL_HANDLE;
  device->queue = VK_NULL_HANDLE;
  device->alive = false;
}

enum gfx_error_e gfx_init(struct gfx_s *gfx) {
  assert_notnull(gfx);
  memset(gfx, 0, sizeof(*gfx));

  VkInstance instance;
  VkResult result = vkCreateInstance(&INSTANCE_CREATE_INFO, NULL, &instance);

  if (result != VK_SUCCESS)
    return gfx_error_instance_create_failed;

  struct gfx_device_s device;
  enum gfx_device_error_e device_err = gfx_device_init(&device, instance);
  if (device_err != gfx_device_error_ok) {
    vkDestroyInstance(instance, NULL);
    return gfx_error_device_get_failed;
  }

  gfx->instance = instance;
  gfx->device = device;
  gfx->alive = true;

  return gfx_error_ok;
}

void gfx_deinit(struct gfx_s *gfx) {
  assert_notnull(gfx);

  if (!gfx->alive)
    return;

  gfx_device_deinit(&gfx->device);
  vkDestroyInstance(gfx->instance, NULL);

  gfx->instance = VK_NULL_HANDLE;
  gfx->alive = false;
}
