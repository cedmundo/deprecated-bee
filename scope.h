#pragma once
#include "ast.h"
#include "run.h"

struct bind {
  struct bind *next;
  char *id;
  struct value value;
};

struct scope {
  struct scope *parent;
  struct bind *binds;
  struct def_exprs *def_exprs;
};

struct scope *scope_fork(struct scope *parent);
void scope_init(struct scope *scope);
void scope_leave(struct scope *scope);

void scope_bind(struct scope *scope, const char *id, struct value value);
struct bind *scope_resolve(struct scope *scope, const char *id);
struct scope *scope_builtins(struct scope *scope);
