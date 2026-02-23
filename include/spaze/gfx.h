#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <EGL/egl.h>
#include <wayland-egl.h>

struct event_loop_s;
struct window_s;

enum gfx_error_e {
  gfx_error_ok,
  gfx_error_get_display_failed,
  gfx_error_egl_init_failed,
  gfx_error_egl_config_not_found,
  gfx_error_egl_context_creation_failed,
};

struct gfx_s {
  EGLConfig config;
  EGLDisplay display;
  EGLContext context;
  bool alive;
};

enum gfx_error_e gfx_init(struct gfx_s *gfx, struct event_loop_s *evl);
void gfx_deinit(struct gfx_s *gfx);

enum renderer_error_e {
  renderer_error_ok,
  renderer_error_window_creation_failed,
  renderer_error_egl_surface_creation_failed,
};

struct renderer_s {
  struct gfx_s *gfx;
  EGLSurface surface;
  struct wl_egl_window *window;
  uint32_t width, height;
  bool alive;
};

enum renderer_error_e renderer_init(struct renderer_s *renderer,
                                    struct gfx_s *gfx, struct window_s *window,
                                    uint32_t width, uint32_t height);
bool renderer_use(struct renderer_s *renderer);
void renderer_swap(struct renderer_s *renderer);
void renderer_deinit(struct renderer_s *renderer);
