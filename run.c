#define _GNU_SOURCE
#include "run.h"
#include "builtins.h"
#include <assert.h>
#include <iconv.h>
#include <stdlib.h>
#include <string.h>

struct value run_lit_expr(struct scope *scope, struct lit_expr *lit_expr) {
  struct value res;
  if (lit_expr->type == LIT_STRING) {
    res.type = TYPE_STRING;
    size_t quoted_size = strlen(lit_expr->raw_value);
    res.str = strndup(lit_expr->raw_value + 1, quoted_size - 1);
  } else {
    res.type = TYPE_NUMBER;
    res.num = strtod(lit_expr->raw_value, NULL);
  }
  return res;
}

struct value run_bin_expr(struct scope *scope, struct bin_expr *bin_expr) {
  struct value res;
  return res;
}

struct value run_call_expr(struct scope *scope, struct call_expr *call_expr);
struct value run_def_expr(struct scope *scope, struct def_expr *def_expr);
struct value run_let_expr(struct scope *scope, struct let_expr *let_expr);
struct value run_if_expr(struct scope *scope, struct if_expr *if_expr);
struct value run_for_expr(struct scope *scope, struct for_expr *for_expr);
struct value run_expr(struct scope *scope, struct expr *expr);

struct scope *scope_fork(struct scope *parent) {
  struct scope *scope = malloc(sizeof(struct scope));
  scope->parent = parent;
  scope->binds = NULL;
  return scope;
}

void scope_exit(struct scope *scope) {
  assert(scope != NULL);
  struct bind *cur_bind = scope->binds;
  struct bind *tmp_bind = NULL;

  while (cur_bind != NULL) {
    if (cur_bind->value.type == TYPE_STRING && cur_bind->value.str != NULL) {
      free(cur_bind->value.str);
    } else if (cur_bind->value.type == TYPE_FUNCTION &&
               cur_bind->value.fun != NULL) {
      free(cur_bind->value.fun);
    }

    tmp_bind = cur_bind->next;
    free(cur_bind);
    cur_bind = tmp_bind;
  }

  free(scope);
}

void scope_bind(struct scope *scope, const char *id, struct value value) {
  assert(scope != NULL);
  assert(id != NULL);
  struct bind *new_bind = malloc(sizeof(struct bind));
  new_bind->id = id;
  new_bind->value = value;
  new_bind->next = NULL;

  struct bind *last_bind = scope->binds;
  if (last_bind != NULL) {
    while (last_bind->next != NULL) {
      last_bind = last_bind->next;
    }

    last_bind->next = new_bind;
  } else {
    scope->binds = new_bind;
  }
}

struct bind *scope_resolve(struct scope *scope, const char *id) {
  assert(scope != NULL);
  assert(id != NULL);

  struct bind *bind = scope->binds;
  while (bind != NULL) {
    if (strcmp(bind->id, id) == 0) {
      return bind;
    }
  }

  if (scope->parent != NULL) {
    return scope_resolve(scope->parent, id);
  } else {
    return NULL;
  }
}

void scope_builtins(struct scope *scope) {
  static struct function puts = {.type = FUNC_NATIVE, .nat_ref = mig_put};
  struct value puts_v = {.type = TYPE_FUNCTION, .fun = &puts};
  scope_bind(scope, "puts", puts_v);
}
