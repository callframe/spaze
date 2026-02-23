#pragma once

#include "glad.h"
#include "spaze/gfx.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

static const char *const VERTEX_SHADER = "#version 460 core\n";

static const char *const FRAGMENT_SHADER = "#version 460 core\n";

enum {
  rendering_transfer_format = GL_BGRA,
  rendering_color_format = GL_RGBA8,

  rendering_bits_per_channel = 8,
  rendering_channels_per_pixel = 4,
  rendering_pixel_size =
      rendering_bits_per_channel * rendering_channels_per_pixel / CHAR_BIT,

  rendering_green_shift = 8,
  rendering_blue_shift = 16,
  rendering_alpha_shift = 24,
};

_Static_assert(rendering_pixel_size == sizeof(uint32_t),
               "Expected pixel size to be 4 bytes");

static inline uint32_t rendering_color(uint8_t r, uint8_t g, uint8_t b,
                                       uint8_t a) {
  return (r << 0) | (g << rendering_green_shift) | (b << rendering_blue_shift) |
         (a << rendering_alpha_shift);
}

enum framebuffer_backend_error_e {
  framebuffer_backend_error_ok,
  framebuffer_backend_error_texture_creation_failed,
  framebuffer_backend_error_sampler_creation_failed,
  framebuffer_backend_error_update_failed,
};

struct framebuffer_backend_s {
  GLuint texture;
  GLuint sampler;
  bool alive;
};

enum framebuffer_backend_error_e
framebuffer_backend_init(struct framebuffer_backend_s *backend);
enum framebuffer_backend_error_e
framebuffer_backend_update(struct framebuffer_backend_s *backend,
                           uint32_t width, uint32_t height);
void framebuffer_backend_deinit(struct framebuffer_backend_s *backend);

enum framebuffer_error_e {
  framebuffer_error_ok,
  framebuffer_error_backend_init_failed,
};

struct framebuffer_s {
  struct renderer_s *renderer;
  struct framebuffer_backend_s backend;
  uint32_t *pixels;
  bool alive;
};

enum framebuffer_error_e framebuffer_init(struct framebuffer_s *fb,
                                          struct renderer_s *renderer);
void framebuffer_resize(struct framebuffer_s *fb);
void framebuffer_update(struct framebuffer_s *fb);
void framebuffer_deinit(struct framebuffer_s *fb);
