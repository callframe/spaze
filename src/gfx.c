
#include "spaze/gfx.h"
#include "spaze/common.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define SHM_OFLAGS (O_RDWR | O_CREAT | O_EXCL)
#define SHM_MODE (S_IRUSR | S_IWUSR)
#define SHM_PROT (PROT_READ | PROT_WRITE)
#define SHM_MAP_FLAGS (MAP_SHARED)
#define SHM_NAME "/spaze"

enum shm_pool_error_e shm_pool_grow(int32_t fd, usize_t old_capacity,
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

struct wl_buffer *shm_pool_allocate(struct shm_pool_s *shm_pool, usize_t width,
                                    usize_t height, uint32_t format) {
  assert_notnull(shm_pool);
  assert(shm_pool->alive);

  return NULL;
}

void shm_pool_deinit(struct shm_pool_s *shm_pool) {
  assert_notnull(shm_pool);

  if (!shm_pool->alive)
    return;

  wl_shm_pool_destroy(shm_pool->pool);
  munmap(shm_pool->pool_data, shm_pool->pool_capacity);
  close(shm_pool->fd);

  memset(shm_pool, 0, sizeof(*shm_pool));
}
