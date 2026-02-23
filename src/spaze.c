#include "spaze/common.h"
#include "spaze/gfx.h"
#include "spaze/windowing.h"
#include <stdbool.h>
#include <stdlib.h>
#include <wayland-client.h>

#define KiB(n) ((n) * 1024)
#define MiB(n) (KiB(n) * 1024)
#define SHM_POOL_SIZE (MiB(20))

static void handle_events(struct event_loop_s *loop, bool *should_quit) {
  struct event_s event;

  while (event_loop_get(loop, &event)) {
    switch (event.kind) {
    case event_kind_close:
      *should_quit = true;
      break;
    case event_kind_resize:
      break;
    }
  }
}

int main() {
  struct event_loop_s evl;
  enum event_loop_error_e err = event_loop_init(&evl);
  if (err != event_loop_error_ok) {
    panic("failed to create eventloop with: %d\n", err);
    return EXIT_FAILURE;
  }

  struct shm_pool_s shm_pool;
  enum shm_pool_error_e shm_err =
      shm_pool_init(&shm_pool, SHM_POOL_SIZE, evl.shm);
  if (shm_err != shm_pool_error_ok)
    panic("failed to create shm pool with: %d\n", shm_err);

  struct window_s window;
  enum window_error_e win_err = window_init(&window, &evl);
  if (win_err != window_error_ok)
    panic("failed to create window with: %d\n", win_err);

  bool should_quit = false;
  while (!should_quit) {
    event_loop_update(&evl);
    handle_events(&evl, &should_quit);
  }

  window_deinit(&window);
  shm_pool_deinit(&shm_pool);
  event_loop_deinit(&evl);
}
