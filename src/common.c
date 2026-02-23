#include "spaze/common.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

_Noreturn void panic_impl(const char *file, int line, const char *func,
                          const char *fmt, ...) {
  fprintf(stderr, "Panic at %s:%d in %s: ", file, line, func);
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  abort();
}
