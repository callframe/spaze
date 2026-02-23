#pragma once

#include "spaze/array.h"
#include <stdbool.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

enum event_loop_error_e {
  event_loop_error_ok,
  event_loop_error_connect_failed,
  event_loop_error_registry_get,
};

struct event_loop_s {
  struct array_s events;
  struct wl_display *display;
  struct wl_compositor *compositor;
};

enum event_loop_error_e event_loop_init(struct event_loop_s *loop);
void event_loop_run(struct event_loop_s *loop, bool *should_exit);
void event_loop_deinit(struct event_loop_s *loop);
