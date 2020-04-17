#pragma once
#include "ast.h"
#include "run.h"

enum bind_mode { BIND_OWNER, BIND_BORROW };
struct bind {
  struct bind *next;
  char *id;
  struct value value;
  enum bind_mode mode;
};

struct scope {
  struct scope *global;
  struct scope *parent;
  struct bind *head;
  struct bind *tail;
  struct def_exprs *def_exprs;
};

struct scope *scope_fork(struct scope *parent);
void scope_init(struct scope *scope);
void scope_leave(struct scope *scope);

void scope_bind(struct scope *scope, const char *id, struct value value,
                enum bind_mode mode);
struct bind *scope_resolve(struct scope *scope, const char *id);
struct scope *scope_builtins(struct scope *scope);
