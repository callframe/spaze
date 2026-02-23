#pragma once

#include "spaze/array.h"
#include "spaze/common.h"
#include "spaze/xdg-shell.h"
#include <stdbool.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

enum event_kind_e {
  event_kind_close,
  event_kind_resize,
};

struct event_resize_s {
  usize_t new_width, new_height;
};

union event_data_u {
  struct event_resize_s resize;
};

struct event_s {
  enum event_kind_e kind;
  union event_data_u data;
  struct window_s *window;
};

enum event_loop_error_e {
  event_loop_error_ok,
  event_loop_error_connect_failed,
  event_loop_error_registry_get_failed,
};

struct event_loop_s {
  struct array_s events;
  struct wl_display *display;
  struct wl_compositor *compositor;
  struct xdg_wm_base *wm_base;
  bool alive;
};

enum event_loop_error_e event_loop_init(struct event_loop_s *loop);
void event_loop_update(struct event_loop_s *loop);
bool event_loop_get(struct event_loop_s *loop, struct event_s *event);
void event_loop_deinit(struct event_loop_s *loop);

enum window_error_e {
  window_error_ok,
  window_error_wl_surface_create_failed,
  window_error_xdg_surface_create_failed,
  window_error_xdg_toplevel_create_failed,
};

struct window_s {
  struct wl_surface *surface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
  struct event_loop_s *loop;
  bool alive;
};

enum window_error_e window_init(struct window_s *window,
                                struct event_loop_s *loop);
void window_deinit(struct window_s *window);
