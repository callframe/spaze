#include "spaze/windowing.h"
#include "spaze/array.h"
#include "spaze/common.h"
#include <string.h>
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

  loop->events = array_init(struct event_s);
  struct wl_display *display = wl_display_connect(NULL);
  if (display == NULL)
    return event_loop_error_connect_failed;

  struct wl_registry *registry = wl_display_get_registry(display);
  if (registry == NULL) {
    wl_display_disconnect(display);
    return event_loop_error_registry_get;
  }

  wl_registry_add_listener(registry, &registry_listener, loop);

  return event_loop_error_ok;
}

void event_loop_deinit(struct event_loop_s *loop) {
  assert_notnull(loop);
  array_deinit(&loop->events);

  if (loop->display == NULL)
    return;

  array_deinit(&loop->events);
  wl_display_disconnect(loop->display);

  loop->display = NULL;
}
