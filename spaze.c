#include "spaze/common.h"
#include "spaze/windowing.h"
#include <stdbool.h>
#include <stdlib.h>

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

static void render_frame(void) {}

int main() {
  struct event_loop_s evl;
  enum event_loop_error_e err = event_loop_init(&evl);
  if (err != event_loop_error_ok) {
    panic("failed to create eventloop with: %d", err);
    return EXIT_FAILURE;
  }

  struct window_s window;
  enum window_error_e win_err = window_init(&window, &evl);
  if (win_err != window_error_ok)
    panic("failed to create window with: %d", win_err);

  bool should_quit = false;

  while (!should_quit) {
    event_loop_update(&evl);
    handle_events(&evl, &should_quit);
    render_frame();
  }

  window_deinit(&window);
  event_loop_deinit(&evl);
}
