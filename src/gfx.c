#include "spaze/gfx.h"
#include "spaze/common.h"
#include "spaze/windowing.h"
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <glad.h>
#include <stdbool.h>
#include <string.h>

#define OPENGL_MAJOR 4
#define OPENGL_MINOR 6
#define CHANNEL_WIDTH 8

static bool GL_LOADED = false;

/* clang-format off */

static const EGLint CONTEXT_ATTRS[] = {
    EGL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR,
    EGL_CONTEXT_MINOR_VERSION, OPENGL_MINOR,
    EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
    EGL_NONE,
};

static const EGLint CONFIG_ATTRS[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_RED_SIZE, CHANNEL_WIDTH,
    EGL_GREEN_SIZE, CHANNEL_WIDTH,
    EGL_BLUE_SIZE, CHANNEL_WIDTH,
    EGL_ALPHA_SIZE, CHANNEL_WIDTH,
    EGL_NONE,
};

/* clang-format on */

static inline void gfx_make_current(EGLDisplay display, EGLSurface surface,
                                    EGLContext context) {
  assert(display != EGL_NO_DISPLAY);
  eglMakeCurrent(display, surface, surface, context);
}

static bool gfx_choose_config(EGLDisplay display, EGLConfig *out_config) {
  assert(display != EGL_NO_DISPLAY);
  assert_notnull(out_config);

  EGLint nconfigs = 0;
  EGLConfig config;

  EGLBoolean success =
      eglChooseConfig(display, CONFIG_ATTRS, &config, 1, &nconfigs);

  if (!success || nconfigs < 1)
    return false;

  *out_config = config;
  return true;
}

enum gfx_error_e gfx_init(struct gfx_s *gfx, struct event_loop_s *evl) {
  assert_notnull(gfx);
  assert_notnull(evl);
  memset(gfx, 0, sizeof(*gfx));

  EGLDisplay edisplay = eglGetDisplay((EGLNativeDisplayType)evl->display);
  if (edisplay == EGL_NO_DISPLAY)
    return gfx_error_get_display_failed;

  if (!eglInitialize(edisplay, NULL, NULL))
    return gfx_error_egl_init_failed;

  EGLConfig econfig;
  if (!gfx_choose_config(edisplay, &econfig))
    return gfx_error_egl_config_not_found;

  if (!eglBindAPI(EGL_OPENGL_API))
    return gfx_error_egl_context_creation_failed;

  EGLContext econtext =
      eglCreateContext(edisplay, econfig, EGL_NO_CONTEXT, CONTEXT_ATTRS);
  if (econtext == EGL_NO_CONTEXT)
    return gfx_error_egl_context_creation_failed;

  gfx->display = edisplay;
  gfx->config = econfig;
  gfx->context = econtext;
  gfx->alive = true;

  return gfx_error_ok;
}

void gfx_deinit(struct gfx_s *gfx) {
  assert_notnull(gfx);
  if (!gfx->alive)
    return;

  gfx_make_current(gfx->display, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  eglDestroyContext(gfx->display, gfx->context);
  eglTerminate(gfx->display);

  memset(gfx, 0, sizeof(*gfx));
}

enum renderer_error_e renderer_init(struct renderer_s *renderer,
                                    struct gfx_s *gfx, struct window_s *window,
                                    uint32_t width, uint32_t height) {
  assert_notnull(renderer);
  assert_notnull(gfx);
  assert_notnull(window);
  memset(renderer, 0, sizeof(*renderer));

  struct wl_egl_window *ewindow =
      wl_egl_window_create(window->surface, width, height);
  if (!ewindow)
    return renderer_error_window_creation_failed;

  EGLSurface esurface = eglCreateWindowSurface(
      gfx->display, gfx->config, (EGLNativeWindowType)ewindow, NULL);

  if (esurface == EGL_NO_SURFACE) {
    wl_egl_window_destroy(ewindow);
    return renderer_error_egl_surface_creation_failed;
  }

  renderer->gfx = gfx;
  renderer->window = ewindow;
  renderer->surface = esurface;
  renderer->width = width;
  renderer->height = height;
  renderer->alive = true;

  return renderer_error_ok;
}

bool renderer_use(struct renderer_s *renderer) {
  assert_notnull(renderer);

  struct gfx_s *gfx = renderer->gfx;
  GLboolean result = eglMakeCurrent(gfx->display, renderer->surface,
                                    renderer->surface, gfx->context);
  if (unlikely(!result))
    return false;

  if (likely(GL_LOADED))
    return true;

  if (unlikely(!gladLoadGLLoader((GLADloadproc)eglGetProcAddress)))
    return false;

  GL_LOADED = true;
  return true;
}

void renderer_swap(struct renderer_s *renderer) {
  assert_notnull(renderer);

  struct gfx_s *gfx = renderer->gfx;

  renderer_use(renderer);
  eglSwapBuffers(gfx->display, renderer->surface);
}

void renderer_resize(struct renderer_s *renderer, uint32_t new_width,
                     uint32_t new_height) {
  assert_notnull(renderer);

  renderer->width = new_width;
  renderer->height = new_height;

  wl_egl_window_resize(renderer->window, new_width, new_height, 0, 0);
}

void renderer_deinit(struct renderer_s *renderer) {
  assert_notnull(renderer);
  if (!renderer->alive)
    return;

  eglDestroySurface(renderer->gfx->display, renderer->surface);
  wl_egl_window_destroy(renderer->window);

  memset(renderer, 0, sizeof(*renderer));
}