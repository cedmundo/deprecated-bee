#define _GNU_SOURCE
#include "ast.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct def_exprs *make_def_exprs(struct def_expr *def_expr) {
  assert(def_expr != NULL);
  struct def_exprs *def_exprs = malloc(sizeof(struct def_exprs));
  def_exprs->next = NULL;
  def_exprs->def_expr = def_expr;
  return def_exprs;
}

struct def_exprs *append_def_exprs(struct def_exprs *left,
                                   struct def_expr *def_expr) {
  assert(left != NULL);
  struct def_exprs *right = make_def_exprs(def_expr);
  walk_to_last(struct def_exprs, left, {
    assert(last != NULL);
    last->next = right;
  });
  return left;
}

struct expr *make_expr_from_lit(struct lit_expr *expr) {
  assert(expr != NULL);
  struct expr *new_expr = malloc(sizeof(struct expr));
  new_expr->type = EXPR_LIT;
  new_expr->lit_expr = expr;
  return new_expr;
}

struct expr *make_expr_from_lookup(struct lookup_expr *expr) {
  assert(expr != NULL);
  struct expr *new_expr = malloc(sizeof(struct expr));
  new_expr->type = EXPR_LOOKUP;
  new_expr->lookup_expr = expr;
  return new_expr;
}

struct expr *make_expr_from_bin(struct bin_expr *expr) {
  assert(expr != NULL);
  struct expr *new_expr = malloc(sizeof(struct expr));
  new_expr->type = EXPR_BIN;
  new_expr->bin_expr = expr;
  return new_expr;
}

struct expr *make_expr_from_unit(struct unit_expr *expr) {
  assert(expr != NULL);
  struct expr *new_expr = malloc(sizeof(struct expr));
  new_expr->type = EXPR_UNIT;
  new_expr->unit_expr = expr;
  return new_expr;
}

struct expr *make_expr_from_call(struct call_expr *expr) {
  assert(expr != NULL);
  struct expr *new_expr = malloc(sizeof(struct expr));
  new_expr->type = EXPR_CALL;
  new_expr->call_expr = expr;
  return new_expr;
}

struct expr *make_expr_from_let(struct let_expr *expr) {
  assert(expr != NULL);
  struct expr *new_expr = malloc(sizeof(struct expr));
  new_expr->type = EXPR_LET;
  new_expr->let_expr = expr;
  return new_expr;
}

struct expr *make_expr_from_def(struct def_expr *expr) {
  assert(expr != NULL);
  struct expr *new_expr = malloc(sizeof(struct expr));
  new_expr->type = EXPR_DEF;
  new_expr->def_expr = expr;
  return new_expr;
}

struct expr *make_expr_from_if(struct if_expr *expr) {
  assert(expr != NULL);
  struct expr *new_expr = malloc(sizeof(struct expr));
  new_expr->type = EXPR_IF;
  new_expr->if_expr = expr;
  return new_expr;
}

struct expr *make_expr_from_for(struct for_expr *expr) {
  assert(expr != NULL);
  struct expr *new_expr = malloc(sizeof(struct expr));
  new_expr->type = EXPR_FOR;
  new_expr->for_expr = expr;
  return new_expr;
}

struct expr *make_expr_from_reduce(struct reduce_expr *expr) {
  assert(expr != NULL);
  struct expr *new_expr = malloc(sizeof(struct expr));
  new_expr->type = EXPR_REDUCE;
  new_expr->reduce_expr = expr;
  return new_expr;
}

struct expr *make_expr_from_list(struct list_expr *expr) {
  struct expr *new_expr = malloc(sizeof(struct expr));
  new_expr->type = EXPR_LIST;
  new_expr->list_expr = expr;
  return new_expr;
}

struct expr *make_expr_from_lambda(struct lambda_expr *expr) {
  struct expr *new_expr = malloc(sizeof(struct expr));
  new_expr->type = EXPR_LAMBDA;
  new_expr->lambda_expr = expr;
  return new_expr;
}

struct lit_expr *make_lit_expr(enum lit_type type, char *v) {
  assert(v != NULL);
  struct lit_expr *lit_expr = malloc(sizeof(struct lit_expr));
  lit_expr->raw_value = v;
  lit_expr->type = type;
  return lit_expr;
}

struct lookup_expr *make_lookup_expr(char *id) {
  assert(id != NULL);
  struct lookup_expr *lookup_expr = malloc(sizeof(struct lookup_expr));
  lookup_expr->type = LOOKUP_ID;
  lookup_expr->id = id;
  lookup_expr->object = NULL;
  lookup_expr->key = NULL;
  return lookup_expr;
}

struct lookup_expr *make_lookup_key_expr(struct expr *object,
                                         struct expr *key) {
  assert(object != NULL);
  assert(key != NULL);
  struct lookup_expr *lookup_expr = malloc(sizeof(struct lookup_expr));
  lookup_expr->type = LOOKUP_KEY;
  lookup_expr->id = NULL;
  lookup_expr->object = object;
  lookup_expr->key = key;
  return lookup_expr;
}

struct bin_expr *make_bin_expr(struct expr *left, struct expr *right,
                               enum bin_op op) {
  assert(left != NULL);
  assert(right != NULL);
  struct bin_expr *bin_expr = malloc(sizeof(struct bin_expr));
  bin_expr->left = left;
  bin_expr->right = right;
  bin_expr->op = op;
  return bin_expr;
}

struct unit_expr *make_unit_expr(struct expr *right, enum unit_op op) {
  assert(right != NULL);
  struct unit_expr *unit_expr = malloc(sizeof(struct unit_expr));
  unit_expr->right = right;
  unit_expr->op = op;
  return unit_expr;
}

struct call_args *make_call_args(struct expr *expr) {
  assert(expr != NULL);
  struct call_args *call_args = malloc(sizeof(struct call_args));
  call_args->next = NULL;
  call_args->expr = expr;
  return call_args;
}

struct call_args *append_call_args(struct call_args *left, struct expr *expr) {
  assert(left != NULL);
  struct call_args *right = make_call_args(expr);
  walk_to_last(struct call_args, left, {
    assert(last != NULL);
    last->next = right;
  });
  return left;
}

struct call_expr *make_call_expr(char *callee, struct call_args *args) {
  assert(callee != NULL);
  struct call_expr *call_expr = malloc(sizeof(struct call_expr));
  call_expr->callee = callee;
  call_expr->args = args;
  return call_expr;
}

struct let_assigns *make_let_assigns(char *id, struct expr *expr) {
  assert(id != NULL);
  assert(expr != NULL);
  struct let_assigns *let_assigns = malloc(sizeof(struct let_assigns));
  let_assigns->next = NULL;
  let_assigns->id = id;
  let_assigns->expr = expr;
  return let_assigns;
}

struct let_assigns *append_let_assigns(struct let_assigns *left, char *id,
                                       struct expr *expr) {
  assert(left != NULL);
  struct let_assigns *right = make_let_assigns(id, expr);
  walk_to_last(struct let_assigns, left, {
    assert(last != NULL);
    last->next = right;
  });
  return left;
}

struct let_expr *make_let_expr(struct let_assigns *assigns,
                               struct expr *in_expr) {

  assert(assigns != NULL);
  assert(in_expr != NULL);
  struct let_expr *let_expr = malloc(sizeof(struct let_expr));
  let_expr->assigns = assigns;
  let_expr->in_expr = in_expr;
  return let_expr;
}

struct def_params *make_def_params(char *id) {
  assert(id != NULL);
  struct def_params *def_params = malloc(sizeof(struct def_params));
  def_params->id = id;
  def_params->next = NULL;
  return def_params;
}

struct def_params *append_def_params(struct def_params *left, char *id) {
  assert(left != NULL);
  struct def_params *right = make_def_params(id);
  walk_to_last(struct def_params, left, {
    assert(last != NULL);
    last->next = right;
  });
  return left;
}

struct def_expr *make_def_expr(char *id, struct def_params *params,
                               struct expr *body) {
  assert(id != NULL);
  assert(body != NULL);
  struct def_expr *def_expr = malloc(sizeof(struct def_expr));
  def_expr->id = id;
  def_expr->params = params;
  def_expr->body = body;
  return def_expr;
}

struct if_expr *make_if_expr(struct expr *cond_expr, struct expr *then_expr,
                             struct cond_expr *elifs, struct expr *else_expr) {
  assert(cond_expr != NULL);
  assert(then_expr != NULL);
  assert(else_expr != NULL);

  struct if_expr *if_expr = malloc(sizeof(struct if_expr));
  struct cond_expr *cond = malloc(sizeof(struct cond_expr));
  cond->next = elifs;
  cond->then = then_expr;
  cond->cond = cond_expr;

  if_expr->conds = cond;
  if_expr->else_expr = else_expr;
  return if_expr;
}

struct cond_expr *make_cond_expr(struct expr *cond, struct expr *then) {
  assert(cond != NULL);
  assert(then != NULL);
  struct cond_expr *cond_expr = malloc(sizeof(struct cond_expr));
  cond_expr->cond = cond;
  cond_expr->then = then;
  cond_expr->next = NULL;
  return cond_expr;
}

struct cond_expr *append_cond_expr(struct cond_expr *left,
                                   struct cond_expr *right) {
  assert(left != NULL);
  assert(right != NULL);
  walk_to_last(struct cond_expr, left, {
    assert(last != NULL);
    last->next = right;
  });

  return left;
}

struct for_handles *make_for_handles(char *id) {
  assert(id != NULL);
  struct for_handles *for_handles = malloc(sizeof(struct for_handles));
  for_handles->next = NULL;
  for_handles->id = id;
  return for_handles;
}

struct for_handles *append_for_handles(struct for_handles *left, char *id) {
  assert(left != NULL);
  struct for_handles *right = make_for_handles(id);
  walk_to_last(struct for_handles, left, {
    assert(last != NULL);
    last->next = right;
  });
  return left;
}

struct for_expr *make_for_expr(struct expr *iteration,
                               struct for_handles *handles,
                               struct expr *iterator) {
  assert(iteration != NULL);
  assert(handles != NULL);
  assert(iterator != NULL);
  struct for_expr *for_expr = malloc(sizeof(struct for_expr));
  for_expr->iteration_expr = iteration;
  for_expr->handle_expr = handles;
  for_expr->iterator_expr = iterator;
  for_expr->filter_expr = NULL;
  return for_expr;
}

struct for_expr *make_filter_expr(struct expr *iteration,
                                  struct for_handles *handles,
                                  struct expr *iterator, struct expr *filter) {
  assert(iteration != NULL);
  assert(handles != NULL);
  assert(iterator != NULL);
  assert(filter != NULL);
  struct for_expr *for_expr = make_for_expr(iteration, handles, iterator);
  for_expr->filter_expr = filter;
  return for_expr;
}

struct reduce_expr *make_reduce_expr(struct for_expr *for_expr, char *id,
                                     struct expr *expr) {
  assert(for_expr != NULL);
  assert(id != NULL);
  assert(expr != NULL);
  struct reduce_expr *reduce_expr = malloc(sizeof(struct reduce_expr));
  reduce_expr->for_expr = for_expr;
  reduce_expr->id = id;
  reduce_expr->value = expr;
  return reduce_expr;
}

struct list_expr *make_list_expr(struct expr *item) {
  assert(item != NULL);
  struct list_expr *list_expr = malloc(sizeof(struct list_expr));
  list_expr->next = NULL;
  list_expr->item = item;
  return list_expr;
}

struct list_expr *append_list_expr(struct list_expr *left, struct expr *expr) {
  assert(left != NULL);
  assert(expr != NULL);
  struct list_expr *right = make_list_expr(expr);
  walk_to_last(struct list_expr, left, {
    assert(last != NULL);
    last->next = right;
  });
  return left;
}

struct lambda_expr *make_lambda_expr(struct def_params *params,
                                     struct expr *body) {
  assert(params != NULL);
  assert(body != NULL);
  struct lambda_expr *lambda_expr = malloc(sizeof(struct lambda_expr));
  lambda_expr->params = params;
  lambda_expr->body = body;
  return lambda_expr;
}

void free_def_exprs(struct def_exprs *def_exprs) {
  assert(def_exprs != NULL);
  if (def_exprs->def_expr != NULL) {
    free_def_expr(def_exprs->def_expr);
    free(def_exprs->def_expr);
  }

  if (def_exprs->next != NULL) {
    free_def_exprs(def_exprs->next);
    free(def_exprs->next);
  }
}

void free_expr(struct expr *expr) {
  assert(expr != NULL);
  switch (expr->type) {
  case EXPR_LIT:
    free_lit_expr(expr->lit_expr);
    free(expr->lit_expr);
    break;
  case EXPR_LOOKUP:
    free_lookup_expr(expr->lookup_expr);
    free(expr->lookup_expr);
    break;
  case EXPR_BIN:
    free_bin_expr(expr->bin_expr);
    free(expr->bin_expr);
    break;
  case EXPR_UNIT:
    free_unit_expr(expr->unit_expr);
    free(expr->unit_expr);
    break;
  case EXPR_CALL:
    free_call_expr(expr->call_expr);
    free(expr->call_expr);
    break;
  case EXPR_LET:
    free_let_expr(expr->let_expr);
    free(expr->let_expr);
    break;
  case EXPR_DEF:
    free_def_expr(expr->def_expr);
    free(expr->def_expr);
    break;
  case EXPR_IF:
    free_if_expr(expr->if_expr);
    free(expr->if_expr);
    break;
  case EXPR_FOR:
    free_for_expr(expr->for_expr);
    free(expr->for_expr);
    break;
  case EXPR_REDUCE:
    free_reduce_expr(expr->reduce_expr);
    free(expr->for_expr);
    break;
  case EXPR_LIST:
    if (expr->list_expr != NULL) {
      free_list_expr(expr->list_expr);
    }
    free(expr->list_expr);
    break;
  case EXPR_LAMBDA:
    if (expr->lambda_expr != NULL) {
      free_lambda_expr(expr->lambda_expr);
    }
    free(expr->lambda_expr);
    break;
  }
}

void free_lit_expr(struct lit_expr *lit_expr) {
  assert(lit_expr != NULL);
  if (lit_expr->raw_value != NULL) {
    free(lit_expr->raw_value);
  }
}

void free_lookup_expr(struct lookup_expr *lookup_expr) {
  assert(lookup_expr != NULL);
  if (lookup_expr->type == LOOKUP_ID) {
    if (lookup_expr->id != NULL) {
      free(lookup_expr->id);
    }
  } else {
    if (lookup_expr->object != NULL) {
      free_expr(lookup_expr->object);
      free(lookup_expr->object);
    }

    if (lookup_expr->key != NULL) {
      free_expr(lookup_expr->key);
      free(lookup_expr->key);
    }
  }
}

void free_bin_expr(struct bin_expr *bin_expr) {
  assert(bin_expr != NULL);
  if (bin_expr->left != NULL) {
    free_expr(bin_expr->left);
    free(bin_expr->left);
  }

  if (bin_expr->right != NULL) {
    free_expr(bin_expr->right);
    free(bin_expr->right);
  }
}

void free_unit_expr(struct unit_expr *unit_expr) {
  assert(unit_expr != NULL);
  if (unit_expr->right != NULL) {
    free_expr(unit_expr->right);
    free(unit_expr->right);
  }
}

void free_call_args(struct call_args *call_args) {
  assert(call_args != NULL);
  if (call_args->expr != NULL) {
    free_expr(call_args->expr);
    free(call_args->expr);
  }

  if (call_args->next != NULL) {
    free_call_args(call_args->next);
    free(call_args->next);
  }
}

void free_call_expr(struct call_expr *call_expr) {
  assert(call_expr != NULL);
  if (call_expr->callee != NULL) {
    free(call_expr->callee);
  }

  if (call_expr->args != NULL) {
    free_call_args(call_expr->args);
    free(call_expr->args);
  }
}

void free_let_assigns(struct let_assigns *let_assigns) {
  assert(let_assigns != NULL);
  if (let_assigns->id != NULL) {
    free(let_assigns->id);
  }

  if (let_assigns->expr != NULL) {
    free_expr(let_assigns->expr);
    free(let_assigns->expr);
  }

  if (let_assigns->next != NULL) {
    free_let_assigns(let_assigns->next);
    free(let_assigns->next);
  }
}

void free_let_expr(struct let_expr *let_expr) {
  assert(let_expr != NULL);
  if (let_expr->assigns != NULL) {
    free_let_assigns(let_expr->assigns);
    free(let_expr->assigns);
  }

  if (let_expr->in_expr != NULL) {
    free_expr(let_expr->in_expr);
    free(let_expr->in_expr);
  }
}

void free_def_params(struct def_params *def_params) {
  assert(def_params != NULL);
  if (def_params->id != NULL) {
    free(def_params->id);
  }

  if (def_params->next != NULL) {
    free_def_params(def_params->next);
    free(def_params->next);
  }
}

void free_def_expr(struct def_expr *def_expr) {
  assert(def_expr != NULL);
  if (def_expr->id != NULL) {
    free(def_expr->id);
  }

  if (def_expr->params != NULL) {
    free_def_params(def_expr->params);
    free(def_expr->params);
  }

  if (def_expr->body != NULL) {
    free_expr(def_expr->body);
    free(def_expr->body);
  }
}

void free_if_expr(struct if_expr *if_expr) {
  assert(if_expr != NULL);
  if (if_expr->conds != NULL) {
    free_cond_expr(if_expr->conds);
    free(if_expr->conds);
  }

  if (if_expr->else_expr != NULL) {
    free_expr(if_expr->else_expr);
    free(if_expr->else_expr);
  }
}

void free_cond_expr(struct cond_expr *cond_expr) {
  if (cond_expr->cond != NULL) {
    free_expr(cond_expr->cond);
    free(cond_expr->cond);
  }

  if (cond_expr->then != NULL) {
    free_expr(cond_expr->then);
    free(cond_expr->then);
  }

  if (cond_expr->next != NULL) {
    free_cond_expr(cond_expr->next);
    free(cond_expr->next);
  }
}

void free_for_handles(struct for_handles *for_handles) {
  assert(for_handles != NULL);
  if (for_handles->id != NULL) {
    free(for_handles->id);
  }

  if (for_handles->next != NULL) {
    free_for_handles(for_handles->next);
    free(for_handles->next);
  }
}

void free_for_expr(struct for_expr *for_expr) {
  assert(for_expr != NULL);
  if (for_expr->iteration_expr != NULL) {
    free_expr(for_expr->iteration_expr);
    free(for_expr->iteration_expr);
  }

  if (for_expr->iterator_expr != NULL) {
    free_expr(for_expr->iterator_expr);
    free(for_expr->iterator_expr);
  }

  if (for_expr->handle_expr != NULL) {
    free_for_handles(for_expr->handle_expr);
    free(for_expr->handle_expr);
  }
}

void free_reduce_expr(struct reduce_expr *reduce_expr) {
  assert(reduce_expr != NULL);
  if (reduce_expr->for_expr != NULL) {
    free_for_expr(reduce_expr->for_expr);
    free(reduce_expr->for_expr);
  }

  if (reduce_expr->id != NULL) {
    free(reduce_expr->id);
  }

  if (reduce_expr->value != NULL) {
    free_expr(reduce_expr->value);
    free(reduce_expr->value);
  }
}

void free_list_expr(struct list_expr *list_expr) {
  assert(list_expr != NULL);
  if (list_expr->item != NULL) {
    free_expr(list_expr->item);
    free(list_expr->item);
  }

  if (list_expr->next != NULL) {
    free_list_expr(list_expr->next);
    free(list_expr->next);
  }
}

void free_lambda_expr(struct lambda_expr *lambda_expr) {
  if (lambda_expr->params != NULL) {
    free_def_params(lambda_expr->params);
    free(lambda_expr->params);
  }

  if (lambda_expr->body != NULL) {
    free_expr(lambda_expr->body);
    free(lambda_expr->body);
  }
}
