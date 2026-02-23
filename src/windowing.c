#include "spaze/windowing.h"
#include "spaze/array.h"
#include "spaze/common.h"
#include "spaze/xdg-shell.h"
#include <poll.h>
#include <stdbool.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>

#define WL_VERSION 1

static void wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                         uint32_t serial) {
  (void)data;
  xdg_wm_base_pong(xdg_wm_base, serial);
}

static struct xdg_wm_base_listener WM_BASE_LISTENER = {
    .ping = wm_base_ping,
};

static void registry_global(void *data, struct wl_registry *registry,
                            uint32_t name, const char *interface,
                            uint32_t version) {
  assert(version >= WL_VERSION);

  struct event_loop_s *loop = (struct event_loop_s *)data;
  assert_notnull(loop);

  if (!strcmp(interface, wl_compositor_interface.name)) {
    struct wl_compositor *compositor =
        wl_registry_bind(registry, name, &wl_compositor_interface, WL_VERSION);
    assert_notnull(compositor);

    loop->compositor = compositor;
  }

  if (!strcmp(interface, xdg_wm_base_interface.name)) {
    struct xdg_wm_base *wm_base =
        wl_registry_bind(registry, name, &xdg_wm_base_interface, WL_VERSION);
    assert_notnull(wm_base);

    loop->wm_base = wm_base;
    xdg_wm_base_add_listener(loop->wm_base, &WM_BASE_LISTENER, loop);
  }
}

static void registry_global_remove(void *data, struct wl_registry *registry,
                                   uint32_t name) {
  (void)data;
  (void)registry;
  (void)name;
}

static struct wl_registry_listener WL_REGISTERY_LISTENER = {
    .global = registry_global, .global_remove = registry_global_remove};

enum event_loop_error_e event_loop_init(struct event_loop_s *loop) {
  assert_notnull(loop);
  memset(loop, 0, sizeof(*loop));

  loop->events = array_init(struct event_s);
  struct wl_display *display = wl_display_connect(NULL);
  if (display == NULL)
    return event_loop_error_connect_failed;

  struct wl_registry *registry = wl_display_get_registry(display);
  if (registry == NULL) {
    wl_display_disconnect(display);
    return event_loop_error_registry_get_failed;
  }
  wl_registry_add_listener(registry, &WL_REGISTERY_LISTENER, loop);

  loop->display = display;
  loop->alive = true;

  wl_display_roundtrip(display);

  return event_loop_error_ok;
}

void event_loop_update(struct event_loop_s *loop) {
  assert_notnull(loop);

  int fd = wl_display_get_fd(loop->display);
  struct pollfd pfd = {.fd = fd, .events = POLLIN};
  int rc = poll(&pfd, 1, 0);
  bool has_errors = (rc < 0) || (pfd.revents & (POLLHUP | POLLERR)) != 0;

  if (wl_display_prepare_read(loop->display) == -1) {
    wl_display_dispatch_pending(loop->display);
    return;
  }

  if (wl_display_flush(loop->display) < 0) {
    wl_display_cancel_read(loop->display);
    return;
  }

  if (has_errors) {
    wl_display_cancel_read(loop->display);
    return;
  }

  bool has_events = (pfd.revents & POLLIN) != 0;
  if (has_events)
    wl_display_read_events(loop->display);
  else
    wl_display_cancel_read(loop->display);

  wl_display_dispatch_pending(loop->display);
}

bool event_loop_get(struct event_loop_s *loop, struct event_s *event) {
  assert_notnull(loop);
  assert_notnull(event);

  if (array_is_empty(&loop->events))
    return false;

  array_pop(&loop->events, event);
  return true;
}

void event_loop_deinit(struct event_loop_s *loop) {
  assert_notnull(loop);
  array_deinit(&loop->events);

  if (!loop->alive)
    return;

  wl_display_disconnect(loop->display);
  memset(loop, 0, sizeof(*loop));
}

static void window_close(void *data, struct xdg_toplevel *xdg_toplevel) {
  (void)xdg_toplevel;

  struct window_s *window = (struct window_s *)data;
  assert_notnull(window);

  union event_data_u inner = {0};

  struct event_s event = {
      .kind = event_kind_close, .data = inner, .window = window};
  array_push(&window->loop->events, &event);
}

static void window_configure(void *data, struct xdg_toplevel *xdg_toplevel,
                             int32_t width, int32_t height,
                             struct wl_array *states) {
  (void)xdg_toplevel;
  (void)states;

  struct window_s *window = (struct window_s *)data;
  assert_notnull(window);

  struct event_resize_s resize = {.new_width = width, .new_height = height};

  struct event_s event = {
      .kind = event_kind_resize,
      .data = {.resize = resize},
      .window = window,
  };

  array_push(&window->loop->events, &event);
}

static struct xdg_toplevel_listener XDG_TOPLEVEL_LISTENER = {
    .close = window_close,
    .configure = window_configure,
};

enum window_error_e window_init(struct window_s *window,
                                struct event_loop_s *loop) {
  assert_notnull(window);
  assert_notnull(loop);
  memset(window, 0, sizeof(*window));

  struct wl_surface *surface = NULL;
  struct xdg_surface *xdg_surface = NULL;
  struct xdg_toplevel *xdg_toplevel = NULL;

  surface = wl_compositor_create_surface(loop->compositor);
  if (surface == NULL)
    return window_error_wl_surface_create_failed;

  xdg_surface = xdg_wm_base_get_xdg_surface(loop->wm_base, surface);
  if (xdg_surface == NULL) {
    wl_surface_destroy(surface);
    return window_error_xdg_surface_create_failed;
  }

  xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
  if (xdg_toplevel == NULL) {
    xdg_surface_destroy(xdg_surface);
    wl_surface_destroy(surface);
    return window_error_xdg_toplevel_create_failed;
  }

  window->surface = surface;
  window->xdg_surface = xdg_surface;
  window->xdg_toplevel = xdg_toplevel;
  window->loop = loop;
  window->alive = true;
  xdg_toplevel_add_listener(window->xdg_toplevel, &XDG_TOPLEVEL_LISTENER,
                            window);

  return window_error_ok;
}

void window_deinit(struct window_s *window) {
  assert_notnull(window);

  if (!window->alive)
    return;

  wl_surface_destroy(window->surface);
  xdg_surface_destroy(window->xdg_surface);
  xdg_toplevel_destroy(window->xdg_toplevel);

  memset(window, 0, sizeof(*window));
}
