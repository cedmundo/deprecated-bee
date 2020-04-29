#include "builtins.h"
#include "run.h"
#include "scope.h"
#include <stdio.h>

void wrapper_puts(struct scope *scope, struct value *value) {
  struct bind *arg = scope_resolve(scope, "arg_0");
  value->i64 = puts(arg->value.str);
}
