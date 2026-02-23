#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <EGL/egl.h>
#include <wayland-egl.h>

struct event_loop_s;

enum gfx_error_e {
  gfx_error_ok,
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
