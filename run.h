#pragma once
#include "ast.h"

struct value;
struct bind;
struct scope;
struct function;

enum type {
  TYPE_FUNCTION,
  TYPE_STRING,
  TYPE_NUMBER,
};
struct value {
  union {
    struct function *fun;
    double num;
    char *str;
  };
  enum type type;
};

struct bind {
  struct bind *next;
  const char *id;
  struct value value;
};

struct scope {
  struct scope *parent;
  struct bind *binds;
};

typedef struct value (*native_def)(struct scope *);

struct function {
  native_def nat_ref;
  struct def_expr def_ref;
  enum {
    FUNC_NATIVE,
    FUNC_DEF,
  } type;
};

struct value run_lit_expr(struct scope *scope, struct lit_expr *lit_expr);
struct value run_bin_expr(struct scope *scope, struct bin_expr *bin_expr);
struct value run_call_expr(struct scope *scope, struct call_expr *call_expr);
struct value run_def_expr(struct scope *scope, struct def_expr *def_expr);
struct value run_let_expr(struct scope *scope, struct let_expr *let_expr);
struct value run_if_expr(struct scope *scope, struct if_expr *if_expr);
struct value run_for_expr(struct scope *scope, struct for_expr *for_expr);
struct value run_expr(struct scope *scope, struct expr *expr);

struct scope *scope_fork(struct scope *parent);
void scope_exit(struct scope *scope);

void scope_bind(struct scope *scope, const char *id, struct value value);
struct bind *scope_resolve(struct scope *scope, const char *id);

void scope_builtins(struct scope *scope);
