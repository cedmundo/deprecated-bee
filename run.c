#define _GNU_SOURCE
#include "run.h"
#include "binops.h"
#include "builtins.h"
#include <assert.h>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct value run_lit_expr(struct scope *scope, struct lit_expr *lit_expr) {
  assert(scope != NULL);
  assert(lit_expr != NULL);

  struct value res;
  if (lit_expr->type == LIT_STRING) {
    res.type = TYPE_STRING;
    size_t quoted_size = strlen(lit_expr->raw_value);
    res.str = strndup(lit_expr->raw_value + 1, quoted_size - 2);
  } else {
    if (strstr(lit_expr->raw_value, ".") != NULL) {
      res.type = TYPE_F64;
      res.f64 = strtod(lit_expr->raw_value, NULL);
    } else if (strstr(lit_expr->raw_value, "-") != NULL) {
      res.type = TYPE_I64;
      res.i64 = strtol(lit_expr->raw_value, NULL, 10);
    } else {
      res.type = TYPE_U64;
      res.u64 = strtoul(lit_expr->raw_value, NULL, 10);
    }
  }
  return res;
}

struct value run_lookup_expr(struct scope *scope,
                             struct lookup_expr *lookup_expr) {
  assert(scope != NULL);
  assert(lookup_expr != NULL);
  struct value res;
  struct bind *value_bind = scope_resolve(scope, lookup_expr->id);
  if (value_bind == NULL) {
    make_errorf(res, "%s is not defined", lookup_expr->id);
    return res;
  }

  return value_bind->value;
}

struct value run_bin_expr(struct scope *scope, struct bin_expr *bin_expr) {
  assert(scope != NULL);
  assert(bin_expr != NULL);
  struct value left_value = run_expr(scope, bin_expr->left);
  struct value right_value = run_expr(scope, bin_expr->right);

  struct value res = handle_bin_op(left_value, right_value, bin_expr->op);
  return res;
}

struct value run_call_expr(struct scope *scope, struct call_expr *call_expr) {
  assert(scope != NULL);
  assert(call_expr != NULL);

  struct value res;
  struct scope *forked = scope_fork(scope);
  // resolve function
  struct bind *fun_bind = scope_resolve(scope, call_expr->callee);
  if (fun_bind == NULL) {
    scope_exit(forked);
    make_errorf(res, "error: undefined function %s", call_expr->callee);
    return res;
  }
  struct value fun_value = fun_bind->value;
  if (fun_value.type != TYPE_FUNCTION) {
    scope_exit(forked);
    make_errorf(res, "error: %s is not a function", call_expr->callee);
    return res;
  }
  struct function *fun = fun_value.fun;

  // bind arguments and call depending if native or defined
  if (fun->type == FUN_NATIVE) {
    char argn[20];
    memset(argn, 0L, 20);

    struct call_args *cur_arg = call_expr->args;
    int i = 0;
    while (cur_arg != NULL) {
      sprintf(argn, "arg_%d", i);
      struct value arg_value = run_expr(scope, cur_arg->expr);
      scope_bind(scope, argn, arg_value);
      cur_arg = cur_arg->next;
    }

    res = fun->nat_ref(forked);
  } else {
    struct def_expr def_expr = fun->def_ref;
    struct def_params *recv_param = def_expr.params;
    struct call_args *send_param = call_expr->args;
    while (recv_param != NULL) {
      if (send_param == NULL) {
        scope_exit(forked);
        make_errorf(res, "error: %s expects more arguments", call_expr->callee);
        return res;
      }

      scope_bind(forked, recv_param->id, run_expr(scope, send_param->expr));
      send_param = send_param->next;
      recv_param = recv_param->next;
    }

    res = run_expr(forked, def_expr.body);
  }

  scope_exit(forked);
  return res;
}

struct value run_def_expr(struct scope *scope, struct def_expr *def_expr) {
  assert(scope != NULL);
  assert(def_expr != NULL);
  struct value fun_value;
  struct function *fun = malloc(sizeof(struct function));
  fun->type = FUN_DEF;
  fun->def_ref = *def_expr;
  fun_value.type = TYPE_FUNCTION;
  fun_value.fun = fun;
  scope_bind(scope, def_expr->id, fun_value);
  free(def_expr);
  return fun_value;
}

struct value run_let_expr(struct scope *scope, struct let_expr *let_expr) {
  assert(scope != NULL);
  assert(let_expr != NULL);
  struct scope *forked = scope_fork(scope);

  struct let_assigns *assign = let_expr->assigns;
  while (assign != NULL) {
    scope_bind(forked, assign->id, run_expr(scope, assign->expr));
    assign = assign->next;
  }

  struct value res = run_expr(forked, let_expr->in_expr);
  scope_exit(forked);
  return res;
}

struct value run_if_expr(struct scope *scope, struct if_expr *if_expr) {
  assert(scope != NULL);
  assert(if_expr != NULL);

  struct value res;
  struct value cond_value = run_expr(scope, if_expr->cond_expr);
  if (cond_value.type == TYPE_UNIT) {
    make_error(res, "error: cannot evaluate condition for unit type");
    return res;
  }

  if (cond_value.u64) {
    res = run_expr(scope, if_expr->then_expr);
  } else {
    res = run_expr(scope, if_expr->else_expr);
  }

  free_value(&cond_value);
  return res;
}

struct value run_for_expr(struct scope *scope, struct for_expr *for_expr) {
  assert(scope != NULL);
  assert(for_expr != NULL);
  struct value res;
  make_error(res, "error: is not supported yet!");
  return res;
}

struct value run_expr(struct scope *scope, struct expr *expr) {
  assert(scope != NULL);
  assert(expr != NULL);

  struct value res;
  switch (expr->type) {
  case EXPR_LIT:
    res = run_lit_expr(scope, expr->lit_expr);
    break;
  case EXPR_LOOKUP:
    res = run_lookup_expr(scope, expr->lookup_expr);
    break;
  case EXPR_BIN:
    res = run_bin_expr(scope, expr->bin_expr);
    break;
  case EXPR_CALL:
    res = run_call_expr(scope, expr->call_expr);
    break;
  case EXPR_LET:
    res = run_let_expr(scope, expr->let_expr);
    break;
  case EXPR_DEF:
    res = run_def_expr(scope, expr->def_expr);
    break;
  case EXPR_IF:
    res = run_if_expr(scope, expr->if_expr);
    break;
  case EXPR_FOR:
    res = run_for_expr(scope, expr->for_expr);
    break;
  default:
    make_errorf(res, "error: unknown expression type: %d", expr->type);
  }

  return res;
}

void run_main(struct scope *scope) {
  assert(scope != NULL);
  struct call_expr call_main = {.callee = "main"};
  struct value final_value = run_call_expr(scope, &call_main);
  switch (final_value.type) {
  case TYPE_UNIT:
    printf("success [unit]\n");
    break;
  case TYPE_FUNCTION:
    printf("success [function]\n");
    break;
  case TYPE_U64:
    printf("success [u64] %lu\n", final_value.u64);
    break;
  case TYPE_I64:
    printf("success [i64] %lu\n", final_value.i64);
    break;
  case TYPE_F64:
    printf("success [f64] %lf\n", final_value.f64);
    break;
  case TYPE_STRING:
    printf("success [string] %s\n", final_value.str);
    break;
  case TYPE_ERROR:
    printf("failure [error] %s\n", final_value.str);
    break;
  }

  free_value(&final_value);
}

void run_all_def_exprs(struct scope *scope, struct def_exprs *def_exprs) {
  assert(scope != NULL);
  while (def_exprs != NULL) {
    run_def_expr(scope, def_exprs->def_expr);
    def_exprs = def_exprs->next;
  }
}

void free_value(struct value *value) {
  if (value->type == TYPE_F64 || value->type == TYPE_I64 ||
      value->type == TYPE_U64 || value->type == TYPE_UNIT) {
    return;
  }

  if (value->type == TYPE_FUNCTION) {
    struct function *fun = value->fun;
    if (fun->type == FUN_DEF) {
      free_def_expr(&fun->def_ref);
    }
  }

  if (value->str != NULL) {
    free(value->str);
    value->str = NULL;
  }
}

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
    free_value(&cur_bind->value);
    free(cur_bind->id);
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
  assert(new_bind != NULL);

  new_bind->id = strdup(id);
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

    bind = bind->next;
  }

  if (scope->parent != NULL) {
    return scope_resolve(scope->parent, id);
  } else {
    return NULL;
  }
}

struct scope *scope_builtins(struct scope *scope) {
  struct function *fun = malloc(sizeof(struct function));
  assert(fun != NULL);

  fun->type = FUN_NATIVE;
  fun->nat_ref = &wrapper_puts;

  struct value puts_v = {.type = TYPE_FUNCTION, .fun = fun};
  scope_bind(scope, "puts", puts_v);
  return scope;
}
