#define _GNU_SOURCE
#include "builtins.h"
#include "vm.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void setup_builtins(struct enclosing *encl) {
  assert(encl != NULL);

  struct object *print_fun = vm_alloc(encl->vm, true);
  print_fun->type = TYPE_FUNCTION;
  print_fun->function =
      (struct function){.target = TARGET_NATIVE, .native_call = bee_print};
  enclosing_bind(encl, print_fun, strdup("print"));

  struct object *typename_fun = vm_alloc(encl->vm, true);
  typename_fun->type = TYPE_FUNCTION;
  typename_fun->function =
      (struct function){.target = TARGET_NATIVE, .native_call = bee_typename};
  enclosing_bind(encl, typename_fun, strdup("typename"));

  struct object *pair_fun = vm_alloc(encl->vm, true);
  pair_fun->type = TYPE_FUNCTION;
  pair_fun->function =
      (struct function){.target = TARGET_NATIVE, .native_call = bee_pair};
  enclosing_bind(encl, pair_fun, strdup("pair"));

  struct object *head_fun = vm_alloc(encl->vm, true);
  head_fun->type = TYPE_FUNCTION;
  head_fun->function =
      (struct function){.target = TARGET_NATIVE, .native_call = bee_head};
  enclosing_bind(encl, head_fun, strdup("head"));

  struct object *tail_fun = vm_alloc(encl->vm, true);
  tail_fun->type = TYPE_FUNCTION;
  tail_fun->function =
      (struct function){.target = TARGET_NATIVE, .native_call = bee_tail};
  enclosing_bind(encl, tail_fun, strdup("tail"));
}

struct object *bee_print(struct enclosing *encl) {
  assert(encl != NULL);
  struct bind *args_bind = enclosing_find(encl, "args");
  assert(args_bind != NULL);

  struct object *args = args_bind->object;
  assert(args->type == TYPE_LIST);
  struct list *arg = args->list;
  size_t wbytes = 0LL;
  while (arg != NULL) {
    struct object *argv = arg->item;
    wbytes += object_print(argv, false);
    if (arg->next != NULL) {
      printf(" ");
    }

    arg = arg->next;
  }

  printf("\n");
  struct object *res = vm_alloc(encl->vm, false);
  res->type = TYPE_I64;
  res->i64 = wbytes;
  return res;
}

struct object *bee_typename(struct enclosing *encl) {
  assert(encl != NULL);
  struct bind *args_bind = enclosing_find(encl, "args");
  assert(args_bind != NULL);

  struct object *args = args_bind->object;
  assert(args->type == TYPE_LIST);
  assert(args->list != NULL);
  assert(args->list->item != NULL);

  struct object *arg0 = args->list->item;
  struct object *res = vm_alloc(encl->vm, false);
  res->type = TYPE_STRING;

  switch (arg0->type) {
  case TYPE_UNIT:
    res->string = strdup("unit");
    break;
  case TYPE_NIL:
    res->string = strdup("nil");
    break;
  case TYPE_FUNCTION:
    res->string = strdup("function");
    break;
  case TYPE_BOL:
    res->string = strdup("bol");
    break;
  case TYPE_U64:
    res->string = strdup("u64");
    break;
  case TYPE_I64:
    res->string = strdup("i64");
    break;
  case TYPE_F64:
    res->string = strdup("f64");
    break;
  case TYPE_STRING:
    res->string = strdup("string");
    break;
  case TYPE_PAIR:
    res->string = strdup("pair");
    break;
  case TYPE_LIST:
    res->string = strdup("list");
    break;
  case TYPE_DICT:
    res->string = strdup("dict");
    break;
  case TYPE_ERROR:
    res->string = strdup("error");
    break;
  }
  return res;
}

struct object *bee_pair(struct enclosing *encl) {
  assert(encl != NULL);

  struct bind *args_bind = enclosing_find(encl, "args");
  assert(args_bind != NULL);
  struct object *args_obj = args_bind->object;
  assert(args_obj != NULL);
  assert(args_obj->type == TYPE_LIST);

  struct object *res = vm_alloc(encl->vm, false);
  res->type = TYPE_PAIR;

  if (args_obj->list == NULL) {
    // TODO: make a global nil object
    struct object *nil = vm_alloc(encl->vm, false);
    nil->type = TYPE_NIL;
    res->pair.head = nil;
    res->pair.tail = nil;
  } else {
    struct list *arg0_item = args_obj->list;
    struct list *arg1_item = arg0_item->next;
    res->pair.head = arg0_item->item;
    res->pair.tail = arg1_item->item;
  }

  return res;
}

struct object *bee_head(struct enclosing *encl) {
  assert(encl != NULL);

  struct bind *args_bind = enclosing_find(encl, "args");
  assert(args_bind != NULL);
  struct object *args_obj = args_bind->object;
  assert(args_obj != NULL);
  assert(args_obj->type == TYPE_LIST);
  assert(args_obj->list != NULL);
  assert(args_obj->list->item != NULL);

  struct object *pair_arg = args_obj->list->item;
  if (pair_arg->type != TYPE_PAIR) {
    struct object *error = vm_alloc(encl->vm, false);
    make_error(error, "head() takes only one argument and must be a pair");
    return error;
  }

  return pair_arg->pair.head;
}

struct object *bee_tail(struct enclosing *encl) {
  struct bind *args_bind = enclosing_find(encl, "args");
  assert(args_bind != NULL);
  struct object *args_obj = args_bind->object;
  assert(args_obj != NULL);
  assert(args_obj->type == TYPE_LIST);
  assert(args_obj->list != NULL);
  assert(args_obj->list->item != NULL);

  struct object *pair_arg = args_obj->list->item;
  if (pair_arg->type != TYPE_PAIR) {
    struct object *error = vm_alloc(encl->vm, false);
    make_error(error, "tail() takes only one argument and must be a pair");
    return error;
  }

  return pair_arg->pair.tail;
}
