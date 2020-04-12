#include "builtins.h"
#include "run.h"
#include "scope.h"
#include <stdio.h>

struct value wrapper_puts(struct scope *scope) {
  struct bind *arg = scope_resolve(scope, "arg_0");
  struct value res = {.type = TYPE_I64, .i64 = 0};
  res.i64 = puts(arg->value.str);
  return res;
}
