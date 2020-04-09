#include "builtins.h"
#include "run.h"
#include <stdio.h>

struct value mig_put(struct scope *scope) {
  struct bind *arg = scope_resolve(scope, "args_0");
  struct value res = {.type = TYPE_NUMBER, .num = 0};
  res.num = puts(arg->value.str);
  return res;
}
