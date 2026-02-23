#include "spaze/gfx.h"
#include "spaze/common.h"
#include <string.h>
#include <wayland-client-protocol.h>

enum gfx_error_e gfx_init(struct gfx_s *gfx, struct window_s *window) {
  assert_notnull(gfx);
  assert_notnull(window);
  memset(gfx, 0, sizeof(*gfx));

  return gfx_error_ok;
}

void gfx_deinit(struct gfx_s *gfx);
