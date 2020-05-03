#define _GNU_SOURCE
#include "run.h"
#include "binops.h"
#include "scope.h"
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
    scope_bind(scope, NULL, res, BIND_OWNER);
  } else {
    if (strstr(lit_expr->raw_value, ".") != NULL) {
      res.type = TYPE_F64;
      res.f64 = strtod(lit_expr->raw_value, NULL);
    } else {
      res.type = TYPE_I64;
      res.i64 = strtol(lit_expr->raw_value, NULL, 10);
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
    scope_bind(scope, NULL, res, BIND_OWNER);
    return res;
  }

  res = copy_value(scope, value_bind->value);
  return res;
}

struct value run_bin_expr(struct scope *scope, struct bin_expr *bin_expr) {
  assert(scope != NULL);
  assert(bin_expr != NULL);
  struct value left_value = run_expr(scope, bin_expr->left);
  struct value right_value = run_expr(scope, bin_expr->right);

  struct value res = handle_bin_op(left_value, right_value, bin_expr->op);
  scope_bind(scope, NULL, res, BIND_OWNER);
  return res;
}

struct value run_unit_expr(struct scope *scope, struct unit_expr *unit_expr) {
  assert(scope != NULL);
  assert(unit_expr != NULL);
  struct value res;
  struct value right = run_expr(scope, unit_expr->right);
  scope_bind(scope, NULL, right, BIND_OWNER);
  if (unit_expr->op == OP_NEG) {
    switch (right.type) {
    case TYPE_U64:
    case TYPE_I64:
      res.type = TYPE_I64;
      res.i64 = -right.i64;
      break;
    case TYPE_F64:
      res.type = TYPE_F64;
      res.f64 = -right.f64;
      break;
    default:
      make_error(res, "unsupported operation for type");
      scope_bind(scope, NULL, res, BIND_OWNER);
      break;
    }
  } else if (unit_expr->op == OP_NOT) {
    switch (right.type) {
    case TYPE_U64:
    case TYPE_I64:
      res.type = TYPE_I64;
      res.i64 = !right.i64;
      break;
    case TYPE_F64:
      res.type = TYPE_F64;
      res.f64 = !right.f64;
      break;
    default:
      make_error(res, "unsupported operation for type");
      scope_bind(scope, NULL, res, BIND_OWNER);
      break;
    }
  } else {
    make_error(res, "unrecognized unitary operation");
    scope_bind(scope, NULL, res, BIND_OWNER);
  }

  return res;
}

struct value run_call_expr(struct scope *scope, struct call_expr *call_expr) {
  assert(scope != NULL);
  assert(call_expr != NULL);

  struct value res;
  // resolve function
  struct bind *fun_bind = scope_resolve(scope, call_expr->callee);
  if (fun_bind == NULL) {
    make_errorf(res, "undefined function '%s'", call_expr->callee);
    scope_bind(scope, NULL, res, BIND_OWNER);
    return res;
  }
  struct value fun_value = fun_bind->value;
  struct scope *forked = scope_fork(scope->global);
  struct value res_within_forked;
  if (fun_value.type == TYPE_SCRIPT_FUN) {
    struct def_expr *def_expr = fun_value.script_fun;
    struct def_params *recv_param = def_expr->params;
    struct call_args *send_param = call_expr->args;
    while (send_param != NULL) {
      if (recv_param == NULL) {
        scope_leave(forked);
        free(forked);
        make_errorf(res, "%s expects more arguments", call_expr->callee);
        scope_bind(scope, NULL, res, BIND_OWNER);
        return res;
      }

      struct value local_arg = run_expr(scope, send_param->expr);
      scope_bind(scope, NULL, local_arg, BIND_BORROW);

      struct value copied_arg = copy_value(scope, local_arg);
      scope_bind(forked, recv_param->id, copied_arg, BIND_BORROW);
      send_param = send_param->next;
      recv_param = recv_param->next;
    }

    res_within_forked = run_expr(forked, def_expr->body);
    scope_bind(forked, NULL, res_within_forked, BIND_BORROW);

    res = copy_value(scope, res_within_forked);
    scope_leave(forked);
    free(forked);
    return res;
  } else {
    native_def callee = fun_value.native_fun;
    struct call_args *send_param = call_expr->args;
    struct list *params_head = NULL;
    struct list *params_tail = NULL;
    while (send_param != NULL) {
      struct list *item = malloc(sizeof(struct list));
      item->next = NULL;

      struct value local_arg = run_expr(scope, send_param->expr);
      scope_bind(scope, NULL, local_arg, BIND_BORROW);

      struct value copied_arg = copy_value(scope, local_arg);
      item->value = copied_arg;

      if (params_head == NULL) {
        params_head = item;
      }

      if (params_tail != NULL) {
        params_tail->next = item;
      }

      params_tail = item;
      send_param = send_param->next;
    }

    struct value call_args = {.type = TYPE_LIST, .list = params_head};
    scope_bind(forked, "args", call_args, BIND_OWNER);
    callee(forked, &res_within_forked);
    res = copy_value(scope, res_within_forked);

    scope_leave(forked);
    free(forked);
    return res;
  }
}

struct value run_let_expr(struct scope *scope, struct let_expr *let_expr) {
  assert(scope != NULL);
  assert(let_expr != NULL);
  struct scope *forked = scope_fork(scope);

  struct let_assigns *assign = let_expr->assigns;
  while (assign != NULL) {
    struct value value = run_expr(scope, assign->expr);
    scope_bind(forked, assign->id, value, BIND_BORROW);
    assign = assign->next;
  }

  struct value res_within_forked = run_expr(forked, let_expr->in_expr);
  scope_bind(forked, NULL, res_within_forked, BIND_BORROW);

  struct value res = copy_value(scope, res_within_forked);
  scope_leave(forked);
  free(forked);
  return res;
}

struct value run_def_expr(struct scope *scope, struct def_expr *def_expr) {
  assert(scope != NULL);
  assert(def_expr != NULL);
  struct value value;
  value.type = TYPE_SCRIPT_FUN;
  value.script_fun = def_expr;
  scope_bind(scope, def_expr->id, value, BIND_OWNER);
  return value;
}

struct value run_if_expr(struct scope *scope, struct if_expr *if_expr) {
  assert(scope != NULL);
  assert(if_expr != NULL);

  struct value res;
  struct value cond_value = run_expr(scope, if_expr->cond_expr);
  scope_bind(scope, NULL, cond_value, BIND_BORROW);

  if (cond_value.type == TYPE_UNIT) {
    make_error(res, "cannot evaluate condition for unit type");
    scope_bind(scope, NULL, res, BIND_OWNER);
    return res;
  }

  if (cond_value.u64) {
    res = run_expr(scope, if_expr->then_expr);
  } else {
    res = run_expr(scope, if_expr->else_expr);
  }

  scope_bind(scope, NULL, res, BIND_BORROW);
  return res;
}

struct value run_for_expr(struct scope *scope, struct for_expr *for_expr) {
  assert(scope != NULL);
  assert(for_expr != NULL);
  struct value res;
  make_error(res, "map, filter and reduce not supported yet!");
  return res;
}

struct value run_reduce_expr(struct scope *scope,
                             struct reduce_expr *reduce_expr) {
  assert(scope != NULL);
  assert(reduce_expr != NULL);
  struct value res;
  make_error(res, "map, filter and reduce not supported yet!");
  return res;
}

struct value run_list_expr(struct scope *scope, struct list_expr *list_expr) {
  assert(scope != NULL);

  struct list_expr *cur = list_expr;
  struct list *head = NULL;
  struct list *tail = NULL;
  while (cur != NULL) {
    struct list *item = malloc(sizeof(struct list));
    item->next = NULL;

    struct value item_value = run_expr(scope, cur->item);
    item->value = item_value;

    if (head == NULL) {
      head = item;
    }

    if (tail != NULL) {
      tail->next = item;
    }

    tail = item;
    cur = cur->next;
  }

  struct value res = {.type = TYPE_LIST, .list = head};
  scope_bind(scope, NULL, res, BIND_OWNER);
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
  case EXPR_UNIT:
    res = run_unit_expr(scope, expr->unit_expr);
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
  case EXPR_LIST:
    res = run_list_expr(scope, expr->list_expr);
    break;
  case EXPR_FOR:
    res = run_for_expr(scope, expr->for_expr);
    break;
  case EXPR_REDUCE:
    res = run_reduce_expr(scope, expr->reduce_expr);
    break;
  default:
    make_errorf(res, "unknown expression type: %d", expr->type);
    scope_bind(scope, NULL, res, BIND_OWNER);
  }

  return res;
}

void run_main(struct scope *scope) {
  assert(scope != NULL);
  struct call_expr call_main = {.callee = "main"};
  struct value final_value = run_call_expr(scope, &call_main);
  print_value(final_value);
  scope_bind(scope, NULL, final_value, BIND_BORROW);
}

struct list *copy_list(struct scope *scope, struct list *list) {
  struct list *head = NULL;
  struct list *tail = NULL;
  struct list *cur = list, *tmp = NULL;

  while (cur != NULL) {
    tmp = malloc(sizeof(struct list));
    tmp->next = NULL;
    tmp->value = copy_value(scope, cur->value);
    cur = cur->next;

    if (head == NULL) {
      head = tmp;
    }

    if (tail != NULL) {
      tail->next = tmp;
    }

    tail = tmp;
  }

  return head;
}

struct value copy_value(struct scope *scope, struct value value) {
  struct value copy = {.type = value.type};
  switch (value.type) {
  case TYPE_UNIT:
    break;
  case TYPE_ERROR:
  case TYPE_STRING:
    copy.str = strdup(value.str);
    scope_bind(scope, NULL, copy, BIND_OWNER);
    break;
  case TYPE_LIST:
    copy.list = copy_list(scope, value.list);
    scope_bind(scope, NULL, copy, BIND_OWNER);
    break;
  case TYPE_NATIVE_FUN:
  case TYPE_SCRIPT_FUN:
  case TYPE_U64:
  case TYPE_I64:
  case TYPE_F64:
    memcpy(&copy, &value, sizeof(struct value));
    break;
  }
  return copy;
}

void print_value(struct value value) {
  switch (value.type) {
  case TYPE_UNIT:
    printf("unit");
    break;
  case TYPE_SCRIPT_FUN:
    printf("function");
    break;
  case TYPE_NATIVE_FUN:
    printf("native function");
    break;
  case TYPE_U64:
    printf("u64(%lu)", value.u64);
    break;
  case TYPE_I64:
    printf("i64(%ld)", value.i64);
    break;
  case TYPE_F64:
    printf("f64(%lf)", value.f64);
    break;
  case TYPE_STRING:
    printf("string('%s')", value.str);
    break;
  case TYPE_LIST:
    printf("list[");
    struct list *item = value.list;
    while (item != NULL) {
      print_value(item->value);
      item = item->next;
      if (item != NULL) {
        printf(",");
      }
    }
    printf("]");
    break;
  case TYPE_ERROR:
    printf("error('%s')", value.str);
    break;
  }
}

void free_value(struct value *value) {
  struct list *item = NULL, *tmp = NULL;
  switch (value->type) {
  case TYPE_LIST:
    item = value->list;
    while (item != NULL) {
      tmp = item->next;
      free(item);
      item = tmp;
    }
    break;
  case TYPE_STRING:
  case TYPE_ERROR:
    if (value->str != NULL) {
      free(value->str);
    }
    break;
  case TYPE_NATIVE_FUN:
  case TYPE_SCRIPT_FUN:
  case TYPE_UNIT:
  case TYPE_U64:
  case TYPE_I64:
  case TYPE_F64:
    break;
  }
}

void run_all_def_exprs(struct scope *scope, struct def_exprs *def_exprs) {
  assert(scope != NULL);
  scope->def_exprs = def_exprs;
  while (def_exprs != NULL) {
    run_def_expr(scope, def_exprs->def_expr);
    def_exprs = def_exprs->next;
  }
}
