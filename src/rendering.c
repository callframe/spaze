#include "spaze/rendering.h"
#include "glad.h"
#include "mimalloc.h"
#include "spaze/common.h"
#include "spaze/gfx.h"
#include <stdint.h>
#include <string.h>

enum framebuffer_backend_error_e
framebuffer_backend_init(struct framebuffer_backend_s *backend) {
  assert_notnull(backend);
  memset(backend, 0, sizeof(*backend));

  GLuint texture, sampler;
  glCreateTextures(GL_TEXTURE_2D, 1, &texture);
  if (texture == 0)
    return framebuffer_backend_error_texture_creation_failed;

  glCreateSamplers(1, &sampler);
  if (sampler == 0) {
    glDeleteTextures(1, &texture);
    return framebuffer_backend_error_sampler_creation_failed;
  }

  glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  backend->texture = texture;
  backend->sampler = sampler;
  backend->alive = true;

  return framebuffer_backend_error_ok;
}

enum framebuffer_backend_error_e
framebuffer_backend_update(struct framebuffer_backend_s *backend,
                           uint32_t width, uint32_t height) {
  assert_notnull(backend);
  assert(backend->alive);

  GLuint texture = backend->texture;
  glDeleteTextures(1, &texture);
  glCreateTextures(GL_TEXTURE_2D, 1, &texture);
  glTextureStorage2D(texture, 1, rendering_transfer_format, width, height);

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    return framebuffer_backend_error_update_failed;

  backend->texture = texture;
  return framebuffer_backend_error_ok;
}

void framebuffer_update(struct framebuffer_s *fb) {
  assert_notnull(fb);
  assert(fb->alive);

  struct framebuffer_backend_s *backend = &fb->backend;
  glTextureSubImage2D(backend->texture, 0, 0, 0, fb->renderer->width,
                      fb->renderer->height, rendering_color_format,
                      GL_UNSIGNED_BYTE, fb->pixels);
}

void framebuffer_backend_deinit(struct framebuffer_backend_s *backend) {
  assert_notnull(backend);

  if (!backend->alive)
    return;

  glDeleteSamplers(1, &backend->sampler);
  glDeleteTextures(1, &backend->texture);

  memset(backend, 0, sizeof(*backend));
}

enum framebuffer_error_e framebuffer_init(struct framebuffer_s *fb,
                                          struct renderer_s *renderer) {
  assert_notnull(fb);
  assert_notnull(renderer);
  memset(fb, 0, sizeof(*fb));

  uint32_t width = renderer->width;
  uint32_t height = renderer->height;
  uint32_t dimension = width * height;

  uint32_t fb_size = dimension * rendering_pixel_size;
  uint32_t *pixels = mi_malloc(fb_size);
  memset(pixels, 0, fb_size);

  enum framebuffer_backend_error_e err = framebuffer_backend_init(&fb->backend);
  if (err != framebuffer_backend_error_ok) {
    mi_free(pixels);
    return framebuffer_error_backend_init_failed;
  }

  framebuffer_backend_update(&fb->backend, width, height);

  fb->renderer = renderer;
  fb->pixels = pixels;
  fb->alive = true;

  return framebuffer_error_ok;
}

void framebuffer_resize(struct framebuffer_s *fb) {
  assert_notnull(fb);
  assert(fb->alive);

  struct renderer_s *renderer = fb->renderer;
  framebuffer_backend_update(&fb->backend, renderer->width, renderer->height);
}

void framebuffer_deinit(struct framebuffer_s *fb) {
  assert_notnull(fb);
  if (!fb->alive)
    return;

  framebuffer_backend_deinit(&fb->backend);
  mi_free(fb->pixels);

  memset(fb, 0, sizeof(*fb));
}