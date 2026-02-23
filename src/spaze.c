#include "glad.h"
#include "spaze/common.h"
#include "spaze/gfx.h"
#include "spaze/rendering.h"
#include "spaze/windowing.h"
#include <stdbool.h>
#include <stdlib.h>
#include <wayland-client.h>

#define KiB(n) ((n) * 1024)
#define MiB(n) (KiB(n) * 1024)
#define SHM_POOL_SIZE (MiB(20))

static void handle_events(struct event_loop_s *loop,
                          struct renderer_s *renderer, struct framebuffer_s *fb,
                          bool *should_quit) {
  struct event_s event;

  while (event_loop_get(loop, &event)) {
    switch (event.kind) {
    case event_kind_close:
      *should_quit = true;
      break;
    case event_kind_resize:
      renderer_resize(renderer, event.data.resize.new_width,
                      event.data.resize.new_height);
      framebuffer_resize(fb);
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

  struct gfx_s gfx;
  enum gfx_error_e gfx_err = gfx_init(&gfx, &evl);
  if (gfx_err != gfx_error_ok)
    panic("failed to initialize gfx with: %d\n", gfx_err);

  struct window_s window;
  enum window_error_e win_err = window_init(&window, &evl);
  if (win_err != window_error_ok)
    panic("failed to create window with: %d\n", win_err);

  struct renderer_s renderer;
  enum renderer_error_e renderer_err =
      renderer_init(&renderer, &gfx, &window, 800, 600);
  if (renderer_err != renderer_error_ok)
    panic("failed to initialize renderer with: %d\n", renderer_err);
  renderer_use(&renderer);

  struct framebuffer_s framebuffer;
  enum framebuffer_error_e fb_err = framebuffer_init(&framebuffer, &renderer);
  if (fb_err != framebuffer_error_ok)
    panic("failed to initialize framebuffer with: %d\n", fb_err);

  bool should_quit = false;
  while (!should_quit) {
    event_loop_update(&evl);
    handle_events(&evl, &renderer, &framebuffer, &should_quit);

    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    framebuffer_update(&framebuffer);
    renderer_swap(&renderer);
  }

  framebuffer_deinit(&framebuffer);
  renderer_deinit(&renderer);
  window_deinit(&window);
  gfx_deinit(&gfx);
  event_loop_deinit(&evl);
}
