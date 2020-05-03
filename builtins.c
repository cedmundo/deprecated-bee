#include "builtins.h"
#include "run.h"
#include "scope.h"
#include <assert.h>
#include <stdio.h>

void wrapper_puts(struct scope *scope, struct value *value) {
  assert(scope != NULL);
  assert(value != NULL);

  struct bind *bound_args = scope_resolve(scope, "args");
  struct value args = bound_args->value;
  assert(args.type == TYPE_LIST);
  assert(args.list != NULL);

  struct value arg0 = args.list->value;
  value->type = TYPE_I64;
  value->i64 = puts(arg0.str);
}
