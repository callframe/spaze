#pragma once

#include <stdint.h>

#if UINTPTR_MAX == UINT64_MAX
typedef uint64_t usize_t;
typedef uint64_t isize_t;
#elif UINTPTR_MAX == UINT32_MAX
typedef uint32_t usize_t;
typedef uint32_t isize_t;
#else
#error "Unsupported pointer size"
#endif

_Noreturn void panic_impl(const char *file, int line, const char *func,
                          const char *fmt, ...);

#define panic(fmt, ...)                                                        \
  panic_impl(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define assert(expr)                                                           \
  do {                                                                         \
    if (!(expr))                                                               \
      panic("Assertion failed: %s", #expr);                                    \
  } while (0)

#define assert_notnull(ptr) assert((ptr) != NULL)

#define container_of(type, member, link)                                       \
  ((type *)((char *)(link) - offsetof(type, member)))
