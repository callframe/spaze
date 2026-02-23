#include "spaze/array.h"
#include "spaze/common.h"
#include <mimalloc.h>
#include <stdbool.h>
#include <string.h>

static inline bool array_needs_resize(struct array_s *arr, usize_t new_elems) {
  assert_notnull(arr);
  return arr->length + new_elems > arr->capacity;
}

static inline usize_t array_new_capacity(const struct array_s *arr) {
  assert_notnull(arr);
  return arr->capacity == 0 ? ARRAY_INITIAL : arr->capacity * ARRAY_GROWTH;
}

static inline void *array_elem_ptr(const struct array_s *arr, usize_t index) {
  assert_notnull(arr);
  return (char *)arr->ptr + index * arr->elem_size;
}

static void array_resize(struct array_s *arr, usize_t new_capacity) {
  assert_notnull(arr);
  assert(new_capacity > arr->capacity);

  void *new_ptr = mi_malloc(new_capacity * arr->elem_size);
  memcpy(new_ptr, arr->ptr, arr->length * arr->elem_size);
  mi_free(arr->ptr);

  arr->ptr = new_ptr;
  arr->capacity = new_capacity;
}

void array_push(struct array_s *arr, const void *elem) {
  assert_notnull(arr);
  assert_notnull(elem);

  if (unlikely(array_needs_resize(arr, 1)))
    array_resize(arr, array_new_capacity(arr));

  void *dest = array_elem_ptr(arr, arr->length);
  memcpy(dest, elem, arr->elem_size);
  arr->length++;
}

void *array_pop(struct array_s *arr, void *out) {
  assert_notnull(arr);

  if (unlikely(arr->length == 0))
    return NULL;

  arr->length--;
  void *src = array_elem_ptr(arr, arr->length);
  memcpy(out, src, arr->elem_size);
  return out;
}

void array_deinit(struct array_s *arr) {
  assert_notnull(arr);
  if (unlikely(arr->ptr == NULL))
    return;

  mi_free(arr->ptr);
  arr->ptr = NULL;
  arr->length = 0;
  arr->capacity = 0;
}
