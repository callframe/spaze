#include "spaze/windowing.h"
#include "spaze/array.h"
#include "spaze/common.h"
#include <poll.h>
#include <stdbool.h>
#include <string.h>
#include <sys/poll.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

#define WL_VERSION 1

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
  }
}

static void registry_global_remove(void *data, struct wl_registry *registry,
                                   uint32_t name) {
  (void)data;
  (void)registry;
  (void)name;
}

static struct wl_registry_listener registry_listener = {
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

  wl_registry_add_listener(registry, &registry_listener, loop);

  loop->display = display;
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

void event_loop_deinit(struct event_loop_s *loop) {
  assert_notnull(loop);
  array_deinit(&loop->events);

  if (loop->display != NULL)
    wl_display_disconnect(loop->display);

  loop->compositor = NULL;
  loop->display = NULL;
}

enum window_error_e window_init(struct window_s *window,
                                struct event_loop_s *loop) {
  assert_notnull(window);
  assert_notnull(loop);
  memset(window, 0, sizeof(*window));

  enum window_error_e err = window_error_ok;
  struct wl_surface *surface = NULL;
  struct xdg_surface *xdg_surface = NULL;
  struct xdg_toplevel *xdg_toplevel = NULL;

  surface = wl_compositor_create_surface(loop->compositor);
  if (surface == NULL) {
    err = window_error_wl_surface_create_failed;
    goto fail;
  }

  xdg_surface = xdg_wm_base_get_xdg_surface(loop->wm_base, surface);
  if (xdg_surface == NULL) {
    err = window_error_xdg_surface_create_failed;
    goto fail;
  }

  xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
  if (xdg_toplevel == NULL) {
    err = window_error_xdg_toplevel_create_failed;
    goto fail;
  }

  goto ok;

fail:
  if (surface != NULL)
    wl_surface_destroy(surface);

  if (xdg_surface != NULL)
    xdg_surface_destroy(xdg_surface);

  if (xdg_toplevel != NULL)
    xdg_toplevel_destroy(xdg_toplevel);

ok:
  window->surface = surface;
  window->xdg_surface = xdg_surface;
  window->xdg_toplevel = xdg_toplevel;

  return err;
}

enum window_error_e window_deinit(struct window_s *window) {
  assert_notnull(window);

  if (window->xdg_toplevel != NULL)
    xdg_toplevel_destroy(window->xdg_toplevel);

  if (window->xdg_surface != NULL)
    xdg_surface_destroy(window->xdg_surface);

  if (window->surface != NULL)
    wl_surface_destroy(window->surface);

  window->xdg_toplevel = NULL;
  window->xdg_surface = NULL;
  window->surface = NULL;

  return window_error_ok;
}
