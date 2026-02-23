#include "spaze/gfx.h"
#include "spaze/common.h"
#include "spaze/windowing.h"
#include <EGL/egl.h>
#include <EGL/eglplatform.h>

#define OPENGL_MAJOR 4
#define OPENGL_MINOR 6
#define CHANNEL_WIDTH 8

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

static EGLConfig gfx_choose_config(struct gfx_s *gfx) {
  EGLConfig config;
  EGLint nconfs;

  EGLBoolean success =
      eglChooseConfig(gfx->display, CONFIG_ATTRS, &config, 1, &nconfs);
  if (!success || nconfs == 0)
    return NULL;

  return nconfs == 1 ? config : NULL;
}

enum gfx_error_e gfx_init(struct gfx_s *gfx, struct event_loop_s *evl) {
  assert_notnull(gfx);
  assert_notnull(evl);

  EGLDisplay edisplay = eglGetDisplay((EGLNativeDisplayType)evl->display);
  assert_notnull(edisplay);

  if (!eglInitialize(edisplay, NULL, NULL))
    return gfx_error_egl_init_failed;

  EGLConfig econfig = gfx_choose_config(gfx);
  if (econfig == NULL)
    return gfx_error_egl_config_not_found;

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

  eglDestroyContext(gfx->display, gfx->context);
  eglTerminate(gfx->display);

  gfx->alive = false;
  gfx->config = NULL;
  gfx->display = NULL;
  gfx->context = NULL;
}