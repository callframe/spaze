#pragma once

#include "spaze/common.h"
#include "spaze/list.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <wayland-client-protocol.h>

enum {
  gfx_format = WL_SHM_FORMAT_ARGB8888,
  gfx_channel_width = 8,
  gfx_channel_count = 4,
  gfx_pixel_size = (gfx_channel_width * gfx_channel_count) / CHAR_BIT,

  gfx_green_shift = CHAR_BIT,
  gfx_red_shift = CHAR_BIT * 2,
  gfx_alpha_shift = CHAR_BIT * 3,
};

static inline uint32_t gfx_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return ((uint32_t)a << gfx_alpha_shift) | ((uint32_t)r << gfx_red_shift) |
         ((uint32_t)g << gfx_green_shift) | (uint32_t)b;
}

enum shared_pool_error_e {
  shared_pool_error_ok,
  shared_pool_error_memory_failed,
  shared_pool_error_shm_failed,
};

struct shared_pool_s {
  struct wl_shm_pool *pool;
  void *pool_data;
  struct link_s link;
  usize_t used, capacity;
  bool alive;
};

#define shared_pool_get(link) container_of(struct shared_pool_s, link, link)

enum shared_pool_error_e shared_pool_init(struct shared_pool_s *pool,
                                          struct wl_shm *shm, usize_t size);
void *shared_pool_allocate(struct shared_pool_s *pool, usize_t size,
                           usize_t *offset);
void shared_pool_deinit(struct shared_pool_s *pool);
