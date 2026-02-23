#pragma once

#include "common.h"

#define ARRAY_INITIAL 4
#define ARRAY_GROWTH 2

struct array_s {
  void *ptr;
  usize_t length, capacity;
  usize_t elem_size;
};

#define array_init(T)                                                          \
  ((array_s){.ptr = NULL, .length = 0, .capacity = 0, .elem_size = sizeof(T)})

void array_push(struct array_s *arr, const void *elem);
void *array_pop(struct array_s *arr, void* out);
void array_deinit(struct array_s *arr);
