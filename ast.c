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
                                   struct def_exprs *right) {
  assert(left != NULL);
  assert(right != NULL);
  walk_to_last(struct def_exprs, left, {
    assert(last != NULL);
    last->next = right;
  });
  return left;
}

void free_def_exprs(struct def_exprs *def_exprs) {
  assert(def_exprs != NULL);
  if (def_exprs->def_expr != NULL) {
    free_def_expr(def_exprs->def_expr);
    free(def_exprs->def_expr);
    def_exprs->def_expr = NULL;
  }

  if (def_exprs->next != NULL) {
    free_def_exprs(def_exprs->next);
  }
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

void free_expr(struct expr *expr) {
  switch (expr->type) {
  case EXPR_LIT:
    free_lit_expr(expr->lit_expr);
    break;
  case EXPR_LOOKUP:
    free_lookup_expr(expr->lookup_expr);
    break;
  case EXPR_BIN:
    free_bin_expr(expr->bin_expr);
    break;
  case EXPR_CALL:
    free_call_expr(expr->call_expr);
    break;
  case EXPR_LET:
    free_let_expr(expr->let_expr);
    break;
  case EXPR_DEF:
    free_def_expr(expr->def_expr);
    break;
  case EXPR_IF:
    free_if_expr(expr->if_expr);
    break;
  case EXPR_FOR:
    free_for_expr(expr->for_expr);
    break;
  }
}

struct lit_expr *make_lit_expr(enum lit_type type, char *v) {
  assert(v != NULL);
  struct lit_expr *lit_expr = malloc(sizeof(struct lit_expr));
  lit_expr->raw_value = strdup(v);
  lit_expr->type = type;
  return lit_expr;
}

void free_lit_expr(struct lit_expr *lit_expr) {
  assert(lit_expr != NULL);
  if (lit_expr->raw_value != NULL) {
    free(lit_expr->raw_value);
  }
}

struct lookup_expr *make_lookup_expr(char *id) {
  assert(id != NULL);
  struct lookup_expr *lookup_expr = malloc(sizeof(struct lookup_expr));
  lookup_expr->id = strdup(id);
  return lookup_expr;
}

void free_lookup_expr(struct lookup_expr *lookup_expr) {
  assert(lookup_expr != NULL);
  if (lookup_expr->id != NULL) {
    free(lookup_expr->id);
  }
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

void free_bin_expr(struct bin_expr *bin_expr) {
  assert(bin_expr != NULL);
  if (bin_expr->left != NULL) {
    free_expr(bin_expr->left);
    bin_expr->left = NULL;
  }

  if (bin_expr->right != NULL) {
    free_expr(bin_expr->right);
    bin_expr->right = NULL;
  }
}

struct call_args *make_call_args(struct expr *expr) {
  assert(expr != NULL);
  struct call_args *call_args = malloc(sizeof(struct call_args));
  call_args->next = NULL;
  call_args->expr = expr;
  return call_args;
}

struct call_args *append_call_args(struct call_args *left,
                                   struct call_args *right) {
  assert(left != NULL);
  assert(right != NULL);
  walk_to_last(struct call_args, left, {
    assert(last != NULL);
    last->next = right;
  });
  return left;
}

void free_call_args(struct call_args *call_args) {
  assert(call_args != NULL);
  if (call_args->expr != NULL) {
    free_expr(call_args->expr);
    free(call_args->expr);
    call_args->expr = NULL;
  }

  if (call_args->next != NULL) {
    free_call_args(call_args->next);
  }
}

struct call_expr *make_call_expr(char *callee, struct call_args *args) {
  assert(callee != NULL);
  struct call_expr *call_expr = malloc(sizeof(struct call_expr));
  call_expr->callee = strdup(callee);
  call_expr->args = args;
  return call_expr;
}

void free_call_expr(struct call_expr *call_expr) {
  assert(call_expr != NULL);
  if (call_expr->callee != NULL) {
    free(call_expr->callee);
    call_expr->callee = NULL;
  }

  if (call_expr->args != NULL) {
    free_call_args(call_expr->args);
  }
}

struct let_assigns *make_let_assigns(char *id, struct expr *expr) {
  assert(id != NULL);
  assert(expr != NULL);
  struct let_assigns *let_assigns = malloc(sizeof(struct let_assigns));
  let_assigns->next = NULL;
  let_assigns->id = strdup(id);
  let_assigns->expr = expr;
  return let_assigns;
}

struct let_assigns *append_let_assigns(struct let_assigns *left,
                                       struct let_assigns *right) {
  assert(left != NULL);
  assert(right != NULL);
  walk_to_last(struct let_assigns, left, {
    assert(last != NULL);
    last->next = right;
  });
  return left;
}

void free_let_assigns(struct let_assigns *let_assigns) {
  assert(let_assigns != NULL);
  if (let_assigns->expr != NULL) {
    free_expr(let_assigns->expr);
    free(let_assigns->expr);
    let_assigns->expr = NULL;
  }

  if (let_assigns->id != NULL) {
    free(let_assigns->id);
    let_assigns->id = NULL;
  }

  if (let_assigns->next != NULL) {
    free_let_assigns(let_assigns->next);
  }
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

void free_let_expr(struct let_expr *let_expr) {
  assert(let_expr != NULL);
  if (let_expr->assigns != NULL) {
    free_let_assigns(let_expr->assigns);
    free(let_expr->assigns);
    let_expr->assigns = NULL;
  }

  if (let_expr->in_expr != NULL) {
    free_expr(let_expr->in_expr);
    free(let_expr->in_expr);
    let_expr->in_expr = NULL;
  }
}

struct def_params *make_def_params(char *id) {
  assert(id != NULL);
  struct def_params *def_params = malloc(sizeof(struct def_params));
  def_params->id = strdup(id);
  def_params->next = NULL;
  return def_params;
}

struct def_params *append_def_params(struct def_params *left,
                                     struct def_params *right) {
  assert(left != NULL);
  assert(right != NULL);
  walk_to_last(struct def_params, left, {
    assert(last != NULL);
    last->next = right;
  });
  return left;
}

void free_def_params(struct def_params *def_params) {
  assert(def_params != NULL);
  if (def_params->id != NULL) {
    free(def_params->id);
    def_params->id = NULL;
  }

  if (def_params->next != NULL) {
    free_def_params(def_params->next);
  }
}

struct def_expr *make_def_expr(char *id, struct def_params *params,
                               struct expr *body) {
  assert(id != NULL);
  assert(body != NULL);
  struct def_expr *def_expr = malloc(sizeof(struct def_expr));
  def_expr->id = strdup(id);
  def_expr->params = params;
  def_expr->body = body;
  return def_expr;
}

void free_def_expr(struct def_expr *def_expr) {
  assert(def_expr != NULL);

  if (def_expr->id != NULL) {
    free(def_expr->id);
    def_expr->id = NULL;
  }

  if (def_expr->params != NULL) {
    free_def_params(def_expr->params);
    free(def_expr->params);
    def_expr->params = NULL;
  }

  if (def_expr->body != NULL) {
    free_expr(def_expr->body);
    free(def_expr->body);
    def_expr->body = NULL;
  }
}

struct if_expr *make_if_expr(struct expr *then_expr, struct expr *cond_expr,
                             struct expr *else_expr) {
  assert(then_expr != NULL);
  assert(cond_expr != NULL);
  assert(else_expr != NULL);
  struct if_expr *if_expr = malloc(sizeof(struct if_expr));
  if_expr->then_expr = then_expr;
  if_expr->cond_expr = cond_expr;
  if_expr->else_expr = else_expr;
  return if_expr;
}

void free_if_expr(struct if_expr *if_expr) {
  assert(if_expr != NULL);

  if (if_expr->then_expr != NULL) {
    free_expr(if_expr->then_expr);
    free(if_expr->then_expr);
    if_expr->then_expr = NULL;
  }

  if (if_expr->cond_expr != NULL) {
    free_expr(if_expr->cond_expr);
    free(if_expr->cond_expr);
    if_expr->cond_expr = NULL;
  }

  if (if_expr->else_expr != NULL) {
    free_expr(if_expr->else_expr);
    free(if_expr->else_expr);
    if_expr->else_expr = NULL;
  }
}

struct for_handles *make_for_handles(char *id) {
  assert(id != NULL);
  struct for_handles *for_handles = malloc(sizeof(struct for_handles));
  for_handles->next = NULL;
  for_handles->id = strdup(id);
  return for_handles;
}

struct for_handles *append_for_handles(struct for_handles *left,
                                       struct for_handles *right) {
  assert(left != NULL);
  assert(right != NULL);
  walk_to_last(struct for_handles, left, {
    assert(last != NULL);
    last->next = right;
  });
  return left;
}

void free_for_handles(struct for_handles *for_handles) {
  assert(for_handles != NULL);
  if (for_handles->id != NULL) {
    free(for_handles->id);
    for_handles->id = NULL;
  }

  if (for_handles->next != NULL) {
    free_for_handles(for_handles->next);
  }
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
  return for_expr;
}

void free_for_expr(struct for_expr *for_expr) {
  assert(for_expr != NULL);
  if (for_expr->iteration_expr != NULL) {
    free_expr(for_expr->iteration_expr);
    free(for_expr->iteration_expr);
    for_expr->iteration_expr = NULL;
  }

  if (for_expr->iterator_expr != NULL) {
    free_expr(for_expr->iterator_expr);
    free(for_expr->iterator_expr);
    for_expr->iterator_expr = NULL;
  }

  if (for_expr->handle_expr != NULL) {
    free_for_handles(for_expr->handle_expr);
    free(for_expr->handle_expr);
    for_expr->handle_expr = NULL;
  }
}
