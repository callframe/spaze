#include "spaze/gfx.h"
#include "spaze/common.h"
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wayland-client-protocol.h>

#define SHM_NAME "/spaze"
#define SHM_FLAGS (O_RDWR | O_CREAT | O_EXCL)
#define SHM_MODE (S_IRUSR | S_IWUSR)

static void *shared_pool_init_mem(struct shared_pool_s *pool, usize_t size,
                                  int32_t *fd) {
  assert_notnull(pool);
  assert(size > 0);

  int32_t memfd = shm_open(SHM_NAME, SHM_FLAGS, SHM_MODE);
  if (memfd < 0)
    return NULL;

  shm_unlink(SHM_NAME);
  ftruncate(memfd, size);

  void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0);
  if (data == MAP_FAILED) {
    close(memfd);
    return NULL;
  }

  *fd = memfd;
  return data;
}

enum shared_pool_error_e shared_pool_init(struct shared_pool_s *pool,
                                          struct wl_shm *shm, usize_t size) {
  assert_notnull(pool);
  assert_notnull(shm);
  memset(pool, 0, sizeof(*pool));

  size = size * gfx_pixel_size;

  int32_t fd;
  void *data = shared_pool_init_mem(pool, size, &fd);
  if (data == NULL)
    return shared_pool_error_memory_failed;

  struct wl_shm_pool *wl_pool = wl_shm_create_pool(shm, fd, size);
  close(fd);

  if (wl_pool == NULL) {
    munmap(data, size);
    return shared_pool_error_shm_failed;
  }

  pool->pool = wl_pool;
  pool->pool_data = data;
  pool->capacity = size;
  pool->alive = true;
  return shared_pool_error_ok;
}

static inline bool shared_pool_can_allocate(struct shared_pool_s *pool,
                                            usize_t size) {
  return pool->used + size <= pool->capacity;
}

static inline void *shared_pool_offset(struct shared_pool_s *pool,
                                       usize_t offset) {
  assert_notnull(pool);
  assert(offset < pool->capacity);

  return (char *)pool->pool_data + offset;
}

static struct wl_buffer *shared_pool_allocate_soft(struct shared_pool_s *pool,
                                                   usize_t size,
                                                   usize_t *offset) {
  assert_notnull(pool);
  assert_notnull(offset);

  list_iter_forward(&pool->frees, link) {
    struct shared_buffer_s *free = shared_buffer_get(link);
    assert_notnull(free);

    
  }
}

struct wl_buffer *shared_pool_allocate(struct shared_pool_s *pool, usize_t size,
                                       usize_t *offset) {
  assert_notnull(pool);
  assert_notnull(offset);

  if (!shared_pool_can_allocate(pool, size))
    return NULL;
  /*
    usize_t curr_used = pool->used;
    void *ptr = shared_pool_offset(pool, curr_used);
    pool->used += size;

    *offset = curr_used;
    return ptr; */
}

void shared_pool_deinit(struct shared_pool_s *pool) {
  assert_notnull(pool);

  if (!pool->alive)
    return;

  wl_shm_pool_destroy(pool->pool);
  munmap(pool->pool_data, pool->capacity);
  pool->alive = false;
}
