#include "spaze/common.h"
#include "spaze/windowing.h"
#include <stdbool.h>
#include <stdlib.h>

int main() {
  struct event_loop_s evl;
  enum event_loop_error_e err = event_loop_init(&evl);
  if (err != event_loop_error_ok) {
    panic("failed to create eventloop with: %d", err);
    return EXIT_FAILURE;
  }

  while (true)
    event_loop_update(&evl);

  event_loop_deinit(&evl);
}
