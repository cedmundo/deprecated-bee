#define _GNU_SOURCE
#include "vm.h"
#include "ast.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void vm_init(struct vm *vm) {
  assert(vm != NULL);
  vm->heap_head = NULL;
  vm->heap_tail = NULL;
  vm->source_exprs = NULL;
  enclosing_init(&vm->globals, NULL);
  timespec_get(&vm->last_gc, TIME_UTC);
}

void vm_free(struct vm *vm) {
  assert(vm != NULL);
  struct object *rem = vm->heap_head;
  struct object *tmp = NULL;

  while (rem != NULL) {
    tmp = rem->gc_next;
    object_free(rem);
    free(rem);
    rem = tmp;
  }

  if (vm->source_exprs != NULL) {
    free_def_exprs(vm->source_exprs);
    free(vm->source_exprs);
  }

  enclosing_free(&vm->globals);
}

size_t vm_mark_all(struct vm *vm) {
  struct object *cur = vm->heap_head;
  size_t marked = 0LL;
  while (cur != NULL) {
    if (cur->flag == GC_ROOT) {
      marked += object_mark(cur);
    }
    cur = cur->gc_next;
  }

  return marked;
}

size_t vm_sweep(struct vm *vm) {
  struct object *cur = vm->heap_head;
  struct object *last = NULL;
  size_t collected = 0LL;

  // warning: this is not thread safe
  while (cur != NULL) {
    if (cur->flag == GC_UNMARKED) {
      struct object *next = cur->gc_next;
      if (last != NULL) {
        last->gc_next = next;
      } else {
        vm->heap_head = next;
      }

      if (next == NULL) {
        vm->heap_tail = NULL;
      }

      collected += object_free(cur);
      free(cur);
      cur = next;
      continue;
    } else {
      cur->flag = GC_UNMARKED;
    }

    last = cur;
    cur = cur->gc_next;
  }

  return collected;
}

size_t vm_gc(struct vm *vm) {
  struct timespec cur_time;
  timespec_get(&cur_time, TIME_UTC);
  if (cur_time.tv_nsec - vm->last_gc.tv_nsec < DEFAULT_GC_INTERVAL_NS) {
    return 0LL;
  }

  size_t marked = vm_mark_all(vm);
  size_t collected = vm_sweep(vm);
  return marked - collected;
}

struct object *vm_alloc(struct vm *vm, bool is_root) {
  struct object *obj = malloc(sizeof(struct object));
  memset(obj, 0L, sizeof(struct object));
  obj->flag = is_root ? GC_ROOT : GC_MARKED;

  // warning: this is not thread safe
  if (vm->heap_head == NULL) {
    vm->heap_head = obj;
  }

  if (vm->heap_tail != NULL) {
    vm->heap_tail->gc_next = obj;
  }

  vm->heap_tail = obj;
  vm_gc(vm);
  return obj;
}

size_t object_free(struct object *obj) {
  assert(obj != NULL);
  size_t hf, tf;
  switch (obj->type) {
  case TYPE_STRING:
  case TYPE_ERROR:
    free(obj->string);
    break;
  case TYPE_PAIR:
    hf = object_free(obj->pair.head);
    tf = object_free(obj->pair.tail);
    free(obj->pair.head);
    free(obj->pair.tail);
    return hf + tf;
  case TYPE_FUNCTION:
  case TYPE_UNIT:
  case TYPE_NIL:
  case TYPE_BOL:
  case TYPE_U64:
  case TYPE_I64:
  case TYPE_F64:
    break;
  }

  return 1;
}

size_t object_mark(struct object *obj) {
  assert(obj != NULL);
  if (obj->flag == GC_MARKED) {
    return 0;
  }

  if (obj->flag == GC_UNMARKED) {
    obj->flag = GC_MARKED;
  }

  if (obj->type == TYPE_PAIR) {
    size_t hm = object_mark(obj->pair.head);
    size_t tm = object_mark(obj->pair.tail);
    return hm + tm;
  }

  return 1;
}

void enclosing_init(struct enclosing *e, struct enclosing *parent) {
  assert(e != NULL);
  e->head = NULL;
  e->tail = NULL;
  e->parent = parent;
}

void enclosing_free(struct enclosing *e) {
  assert(e != NULL);
  struct bind *cur = e->head;
  struct bind *tmp = NULL;

  // only release stuff outside gc tracking
  while (cur != NULL) {
    tmp = cur->next;
    if (cur->id != NULL) {
      free(cur->id);
    }

    free(cur);
    cur = tmp;
  }
}

void enclosing_bind(struct enclosing *e, struct object *object, char *id) {
  assert(e != NULL);
  assert(object != NULL);
  assert(id != NULL);

  struct bind *binding = malloc(sizeof(struct bind));
  binding->next = NULL;
  binding->object = object;
  binding->id = id;

  if (e->head == NULL) {
    e->head = binding;
  }

  if (e->tail != NULL) {
    e->tail->next = binding;
  }

  e->tail = binding;
}

struct bind *enclosing_find(struct enclosing *e, char *id) {
  assert(e != NULL);
  assert(id != NULL);
  struct bind *cur = e->head;
  while (cur != NULL) {
    if (strcmp(cur->id, id) == 0) {
      return cur;
    }

    cur = cur->next;
  }

  if (e->parent != NULL) {
    return enclosing_find(e->parent, id);
  }

  return NULL;
}

struct object *vm_run_main(struct vm *vm) {
  assert(vm != NULL);
  // TODO: implement
  return NULL;
}

void vm_define_all(struct vm *vm, struct def_exprs *defs) {
  assert(vm != NULL);
  assert(defs != NULL);
  vm->source_exprs = defs;

  struct def_exprs *cur = defs;
  while (cur != NULL) {
    struct object *to_define = vm_run_def(vm, cur->def_expr);
    assert(to_define->type == TYPE_FUNCTION);
    enclosing_bind(&vm->globals, to_define, strdup(to_define->function.id));
    cur = cur->next;
  }
}

struct object *vm_run_def(struct vm *vm, struct def_expr *def) {
  assert(vm != NULL);
  assert(def != NULL);

  struct object *object = vm_alloc(vm, true);
  object->type = TYPE_FUNCTION;
  object->function = (struct function){
      .target = TARGET_SCRIPT,
      .native_call = NULL,
      .params = def->params,
      .body = def->body,
      .id = def->id,
  };

  return object;
}
