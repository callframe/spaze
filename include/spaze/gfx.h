#pragma once

#include "spaze/common.h"
#include "spaze/list.h"
#include <stdbool.h>
#include <wayland-client-protocol.h>

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

#define shared_pool_get(link) container_of(struct shared_pool, link, link)

enum shared_pool_error_e shared_pool_init(struct shared_pool_s *pool,
                                          struct wl_shm *shm, usize_t size);
void shared_pool_deinit(struct shared_pool_s *pool);
