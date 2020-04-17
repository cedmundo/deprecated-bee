#define _GNU_SOURCE
#include "scope.h"
#include "builtins.h"
#include "run.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct scope *scope_fork(struct scope *parent) {
  struct scope *scope = malloc(sizeof(struct scope));
  scope_init(scope);
  scope->parent = parent;
  scope->global = parent->global;
  return scope;
}

void scope_init(struct scope *scope) {
  scope->parent = NULL;
  scope->global = NULL;
  scope->head = NULL;
  scope->tail = NULL;
  scope->def_exprs = NULL;
}

void scope_bind(struct scope *scope, const char *id, struct value value,
                enum bind_mode mode) {
  assert(scope != NULL);
  struct bind *new_bind = malloc(sizeof(struct bind));
  assert(new_bind != NULL);

  if (id != NULL) {
    new_bind->id = strdup(id);
  } else {
    new_bind->id = NULL;
  }
  new_bind->value = value;
  new_bind->mode = mode;
  new_bind->next = NULL;

  if (scope->head == NULL) {
    scope->head = new_bind;
  }

  if (scope->tail != NULL) {
    scope->tail->next = new_bind;
  }

  scope->tail = new_bind;
}

struct bind *scope_resolve(struct scope *scope, const char *id) {
  assert(scope != NULL);
  assert(id != NULL);
  struct bind *bind = scope->head;
  while (bind != NULL) {
    if (bind->id == NULL) {
      bind = bind->next;
      continue;
    }

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
  fun->type = FUN_NATIVE;
  fun->nat_ref = &wrapper_puts;
  struct value puts_v = {.type = TYPE_FUNCTION, .fun = fun};
  scope_bind(scope, "puts", puts_v, BIND_OWNER);

  // Code analysis tools may recognize 'fun' being a possible leak,
  // I don't know quite why but it is freed during scope_leave so it should
  // not be a big deal. Also, valgrind does not complain about this anyway.
  return scope; // NOLINT
}

void scope_leave(struct scope *scope) {
  assert(scope != NULL);

  struct bind *cur = scope->head;
  struct bind *tmp = NULL;
  while (cur != NULL) {
    if (cur->id != NULL) {
      free(cur->id);
    }

    if (cur->mode == BIND_OWNER) {
      free_value(&cur->value);
    }

    tmp = cur->next;
    free(cur);
    cur = tmp;
  }

  if (scope->def_exprs != NULL) {
    free_def_exprs(scope->def_exprs);
    free(scope->def_exprs);
  }
}
