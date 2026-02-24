
#include "spaze/gfx.h"
#include "spaze/common.h"
#include "spaze/list.h"
#include <fcntl.h>
#include <limits.h>
#include <mimalloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wayland-client-protocol.h>

#define SHM_OFLAGS (O_RDWR | O_CREAT | O_EXCL)
#define SHM_MODE (S_IRUSR | S_IWUSR)
#define SHM_PROT (PROT_READ | PROT_WRITE)
#define SHM_MAP_FLAGS (MAP_SHARED)
#define SHM_NAME "/spaze"
#define SHM_GROWTH 2

static void buffer_release(void *data, struct wl_buffer *wl_buffer) {
  (void)wl_buffer;
  struct shm_block_s *block = (struct shm_block_s *)data;
  block->busy = false;
}

static struct wl_buffer_listener WL_BUFFER_LISTENER = {
    .release = buffer_release,
};

static enum shm_pool_error_e shm_pool_grow(int32_t fd, usize_t old_capacity,
                                           void *old_data, usize_t new_capacity,
                                           void **new_data) {
  assert_notnull(new_data);

  if (ftruncate(fd, new_capacity) < 0)
    return shm_pool_error_truncate_failed;

  void *ptr = mmap(NULL, new_capacity, SHM_PROT, SHM_MAP_FLAGS, fd, 0);
  if (ptr == MAP_FAILED)
    return shm_pool_error_mmap_failed;

  if (old_data != NULL && old_capacity > 0)
    munmap(old_data, old_capacity);

  *new_data = ptr;
  return shm_pool_error_ok;
}

enum shm_pool_error_e shm_pool_init(struct shm_pool_s *shm_pool,
                                    usize_t capacity, struct wl_shm *shm) {
  assert_notnull(shm_pool);
  assert(capacity > 0);
  assert_notnull(shm);
  memset(shm_pool, 0, sizeof(*shm_pool));

  int32_t fd = shm_open(SHM_NAME, SHM_OFLAGS, SHM_MODE);
  if (fd < 0)
    return shm_pool_error_shmem_open_failed;
  shm_unlink(SHM_NAME);

  void *pool_data;
  enum shm_pool_error_e err = shm_pool_grow(fd, 0, NULL, capacity, &pool_data);
  if (err != shm_pool_error_ok) {
    close(fd);
    return err;
  }

  struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, capacity);
  if (pool == NULL) {
    munmap(pool_data, capacity);
    close(fd);
    return shm_pool_error_shmem_open_failed;
  }

  shm_pool->pool_data = pool_data;
  shm_pool->pool = pool;
  shm_pool->pool_capacity = capacity;
  shm_pool->fd = fd;
  shm_pool->alive = true;

  return shm_pool_error_ok;
}

static inline bool shm_pool_needs_grow(struct shm_pool_s *shm_pool,
                                       usize_t size) {
  return shm_pool->pool_used + size > shm_pool->pool_capacity;
}

static inline usize_t shm_pool_new_capacity(usize_t old_capacity) {
  return old_capacity * SHM_GROWTH;
}

static enum shm_pool_error_e shm_pool_do_grow(struct shm_pool_s *shm_pool,
                                              usize_t new_capacity) {
  assert(new_capacity > shm_pool->pool_capacity);

  void *new_data;
  enum shm_pool_error_e err =
      shm_pool_grow(shm_pool->fd, shm_pool->pool_capacity, shm_pool->pool_data,
                    new_capacity, &new_data);
  if (err != shm_pool_error_ok)
    return err;

  wl_shm_pool_resize(shm_pool->pool, new_capacity);
  shm_pool->pool_data = new_data;
  shm_pool->pool_capacity = new_capacity;

  return shm_pool_error_ok;
}

struct shm_block_s *shm_pool_allocate(struct shm_pool_s *shm_pool,
                                      usize_t width, usize_t height,
                                      usize_t stride) {
  assert_notnull(shm_pool);
  assert(shm_pool->alive);

  usize_t req_size = stride * height;

  list_for_each_reversed(&shm_pool->free_blocks, link) {
    struct shm_block_s *block = container_of(struct shm_block_s, link, link);
    if (block->width != width || block->height != height ||
        block->stride != stride)
      continue;

    list_remove(&shm_pool->free_blocks, &block->link);
    return block;
  }

  usize_t offset = shm_pool->pool_used;

  if (!shm_pool_needs_grow(shm_pool, req_size))
    goto create_buffer;

  usize_t new_capacity = shm_pool_new_capacity(shm_pool->pool_capacity);
  while (new_capacity < offset + req_size)
    new_capacity = shm_pool_new_capacity(new_capacity);

  enum shm_pool_error_e err = shm_pool_do_grow(shm_pool, new_capacity);
  if (err != shm_pool_error_ok)
    return NULL;

create_buffer:;
  struct wl_buffer *buffer = wl_shm_pool_create_buffer(
      shm_pool->pool, offset, width, height, stride, GFX_FORMAT);
  if (buffer == NULL)
    return NULL;
  shm_pool->pool_used += req_size;

  struct shm_block_s *block = mi_malloc(sizeof(*block));
  block->buffer = buffer;
  block->width = width;
  block->height = height;
  block->stride = stride;
  block->busy = false;

  wl_buffer_add_listener(buffer, &WL_BUFFER_LISTENER, block);

  return block;
}

void shm_pool_deallocate(struct shm_pool_s *shm_pool,
                         struct shm_block_s *block) {
  assert_notnull(shm_pool);
  assert_notnull(block);

  list_push(&shm_pool->free_blocks, &block->link);
}

void shm_pool_deinit(struct shm_pool_s *shm_pool) {
  assert_notnull(shm_pool);

  if (!shm_pool->alive)
    return;

  list_for_each(&shm_pool->free_blocks, link) {
    struct shm_block_s *block = container_of(struct shm_block_s, link, link);
    wl_buffer_destroy(block->buffer);
    mi_free(block);
  }

  wl_shm_pool_destroy(shm_pool->pool);
  munmap(shm_pool->pool_data, shm_pool->pool_capacity);
  close(shm_pool->fd);

  memset(shm_pool, 0, sizeof(*shm_pool));
}

enum swapchain_error_e swapchain_init(struct swapchain_s *swapchain,
                                      struct shm_pool_s *shm_pool,
                                      usize_t width, usize_t height) {
  assert_notnull(swapchain);
  assert_notnull(shm_pool);
  memset(swapchain, 0, sizeof(*swapchain));

  swapchain->shm_pool = shm_pool;
  swapchain->width = width;
  swapchain->height = height;
  swapchain->alive = true;

  return swapchain_error_ok;
}

void swapchain_resize(struct swapchain_s *swapchain, usize_t new_width,
                      usize_t new_height) {
  assert_notnull(swapchain);
  assert(swapchain->alive);

  if (swapchain->width == new_width && swapchain->height == new_height)
    return;

  list_for_each(&swapchain->chain, link) {
    struct shm_block_s *block = container_of(struct shm_block_s, link, link);
    shm_pool_deallocate(swapchain->shm_pool, block);
  }

  swapchain->chain = (struct list_s){0};
  swapchain->width = new_width;
  swapchain->height = new_height;
}

struct shm_block_s *swapchain_acquire(struct swapchain_s *swapchain) {
  assert_notnull(swapchain);
  assert(swapchain->alive);

  struct link_s *link = list_pop_front(&swapchain->chain);
  struct shm_block_s *block = shm_block_get(link);
  assert_notnull(block);

  if (block->busy)
    return NULL;

  return block;
}

void swapchain_reclaim(struct swapchain_s *swapchain,
                       struct shm_block_s *block) {
  assert_notnull(swapchain);
  assert_notnull(block);

  list_push(&swapchain->chain, &block->link);
}

void swapchain_deinit(struct swapchain_s *swapchain) {
  assert_notnull(swapchain);

  if (!swapchain->alive)
    return;

  list_for_each(&swapchain->chain, link) {
    struct shm_block_s *block = container_of(struct shm_block_s, link, link);
    shm_pool_deallocate(swapchain->shm_pool, block);
  }

  swapchain->chain = (struct list_s){0};
  swapchain->alive = false;
}
