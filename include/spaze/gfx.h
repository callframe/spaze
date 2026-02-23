#pragma once

#include "spaze/windowing.h"
#include <wayland-client-protocol.h>

#define N_BUFFERS 2

enum gfx_error_e {
  gfx_error_ok,
  gfx_error_buffer_create_failed,
};

struct gfx_s {
  struct wl_buffer *buffers[N_BUFFERS];
  struct window_s *window;
};

enum gfx_error_e gfx_init(struct gfx_s *gfx, struct window_s *window);
void gfx_deinit(struct gfx_s *gfx);
