#include "spaze/gfx.h"
#include "spaze/common.h"
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wayland-client-protocol.h>

#define SHM_NAME "/spaze"
#define SHM_FLAGS (O_RDWR | O_CREAT | O_EXCL)
#define SHM_MODE (S_IRUSR | S_IWUSR)

static void *shared_pool_init_mem(struct shared_pool_s *pool, usize_t size,
                                  int *fd) {
  assert_notnull(pool);
  assert(size > 0);

  int memfd = shm_open(SHM_NAME, SHM_FLAGS, SHM_MODE);
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

  int fd;
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

void shared_pool_deinit(struct shared_pool_s *pool) {
  assert_notnull(pool);

  if (!pool->alive)
    return;

  wl_shm_pool_destroy(pool->pool);
  munmap(pool->pool_data, pool->capacity);
  pool->alive = false;
}
