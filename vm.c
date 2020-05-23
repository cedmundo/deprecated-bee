#define _GNU_SOURCE
#include "vm.h"
#include "ast.h"
#include "binops.h"
#include "builtins.h"
#include "hashmap.h"
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
  enclosing_init(&vm->globals, vm, NULL);
  setup_builtins(&vm->globals);
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
  // vm_gc(vm);
  return obj;
}

size_t object_free(struct object *obj) {
  assert(obj != NULL);
  switch (obj->type) {
  case TYPE_STRING:
  case TYPE_ERROR:
    free(obj->string);
    break;
  case TYPE_PAIR: {
    size_t hf = object_free(obj->pair.head);
    size_t tf = object_free(obj->pair.tail);
    free(obj->pair.head);
    free(obj->pair.tail);
    return hf + tf;
  }
  case TYPE_DICT: {
    // hashmap_free(&obj->hashmap);
    break;
  }
  case TYPE_LIST: {
    struct list *cur = obj->list;
    struct list *tmp = NULL;
    size_t total = 0LL;
    while (cur != NULL) {
      tmp = cur->next;
      free(cur);
      cur = tmp;
    }
    return total;
  }
  case TYPE_FUNCTION: {
    if (obj->function.closure != NULL) {
      enclosing_free(obj->function.closure);
      free(obj->function.closure);
    }
    break;
  }
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

  if (obj->type == TYPE_LIST) {
    size_t tm = 0LL;
    struct list *cur = obj->list;
    while (cur != NULL) {
      tm += object_mark(cur->item);
      cur = cur->next;
    }
  }

  if (obj->type == TYPE_DICT) {
    struct hashmap hm = obj->hashmap;
    struct kv_entry **rows = hm.rows;
    for (size_t ri = 0LL; ri < hm.total_rows; ri++) {
      struct kv_entry *col = rows[ri];
      while (col != NULL) {
        object_mark(col->value);
        col = col->next;
      }
    }
  }

  return 1;
}

size_t dict_print_kvs(struct object *obj, bool debug) {
  assert(obj != NULL);
  assert(obj->type == TYPE_DICT);

  size_t wbytes = 0LL;
  struct hashmap hm = obj->hashmap;
  struct kv_entry **rows = hm.rows;
  for (size_t ri = 0LL; ri < hm.total_rows; ri++) {
    struct kv_entry *col = rows[ri];
    while (col != NULL) {
      wbytes += printf("%s: ", col->key);
      wbytes += object_print(col->value, debug);
      printf(", ");

      col = col->next;
    }
  }

  return wbytes;
}

size_t object_print(struct object *value, bool debug) {
  size_t wbytes = 0LL;
  switch (value->type) {
  case TYPE_NIL:
    wbytes += printf("nil");
    break;
  case TYPE_UNIT:
    wbytes += printf("unit");
    break;
  case TYPE_FUNCTION:
    wbytes += printf("function");
    break;
  case TYPE_BOL:
    if (debug) {
      wbytes += printf("bol(%d)", value->bol);
    } else {
      wbytes += printf(value->bol ? "true" : "false");
    }
    break;
  case TYPE_U64:
    if (debug) {
      wbytes += printf("u64(%lu)", value->u64);
    } else {
      wbytes += printf("%lu", value->u64);
    }
    break;
  case TYPE_I64:
    if (debug) {
      wbytes += printf("i64(%ld)", value->i64);
    } else {
      wbytes += printf("%ld", value->i64);
    }
    break;
  case TYPE_F64:
    if (debug) {
      wbytes += printf("f64(%lf)", value->f64);
    } else {
      wbytes += printf("%lf", value->f64);
    }
    break;
  case TYPE_STRING:
    if (debug) {
      wbytes += printf("string('%s')", value->string);
    } else {
      wbytes += printf("%s", value->string);
    }
    break;
  case TYPE_PAIR:
    printf("pair(");
    wbytes += object_print(value->pair.head, debug);
    printf(",");
    wbytes += object_print(value->pair.tail, debug);
    printf(")");
    break;
  case TYPE_LIST:
    wbytes += printf("list[");
    struct list *item = value->list;
    while (item != NULL) {
      wbytes += object_print(item->item, debug);
      item = item->next;
      if (item != NULL) {
        wbytes += printf(",");
      }
    }
    wbytes += printf("]");
    break;
  case TYPE_DICT:
    wbytes += printf("dict{");
    wbytes += dict_print_kvs(value, debug);
    wbytes += printf("}");
    break;
  case TYPE_ERROR:
    if (debug) {
      wbytes += printf("error('%s')", value->string);
    } else {
      wbytes += printf("%s", value->string);
    }
    break;
  }

  return wbytes;
}

void enclosing_init(struct enclosing *e, struct vm *vm,
                    struct enclosing *parent) {
  assert(e != NULL);
  e->vm = vm;
  e->parent = parent;
  e->head = NULL;
  e->tail = NULL;
}

void enclosing_capture(struct enclosing *into, struct enclosing *from) {
  assert(into != NULL);
  if (from == NULL) {
    return;
  }

  struct bind *src_bind = from->head;
  while (src_bind != NULL) {
    struct bind *cpy_bind = malloc(sizeof(struct bind));
    cpy_bind->id = strdup(src_bind->id);
    cpy_bind->object = src_bind->object;
    cpy_bind->next = NULL;

    if (into->head == NULL) {
      into->head = cpy_bind;
    }

    if (into->tail != NULL) {
      into->tail->next = cpy_bind;
    }

    into->tail = cpy_bind;
    src_bind = src_bind->next;
  }
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

  e->head = NULL;
  e->tail = NULL;
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

  struct enclosing main_enclosing;
  enclosing_init(&main_enclosing, vm, &vm->globals);

  struct call_expr main_call = {
      .callee = "main",
      .args = NULL,
  };
  struct object *res = vm_run_call(&main_enclosing, &main_call);

  enclosing_free(&main_enclosing);
  return res;
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
      .closure = NULL,
      .params = def->params,
      .body = def->body,
      .id = def->id,
  };

  return object;
}

struct object *vm_run_lit(struct enclosing *encl, struct lit_expr *lit_expr) {
  assert(encl != NULL);
  assert(lit_expr != NULL);

  struct object *res = vm_alloc(encl->vm, false);
  if (lit_expr->type == LIT_STRING) {
    res->type = TYPE_STRING;
    size_t quoted_size = strlen(lit_expr->raw_value);
    res->string = strndup(lit_expr->raw_value + 1, quoted_size - 2);
  } else {
    if (strstr(lit_expr->raw_value, ".") != NULL) {
      res->type = TYPE_F64;
      res->f64 = strtod(lit_expr->raw_value, NULL);
    } else {
      res->type = TYPE_I64;
      res->i64 = strtol(lit_expr->raw_value, NULL, 10);
    }
  }

  return res;
}

struct object *vm_run_lookup(struct enclosing *encl,
                             struct lookup_expr *lookup_expr) {
  assert(encl != NULL);
  assert(lookup_expr != NULL);

  if (lookup_expr->type == LOOKUP_ID) {
    struct bind *bind = enclosing_find(encl, lookup_expr->id);
    if (bind == NULL) {
      struct object *res = vm_alloc(encl->vm, false);
      make_errorf(res, "undefined variable '%s'", lookup_expr->id);
      return res;
    }

    return bind->object;
  } else {
    assert(lookup_expr->object != NULL);
    assert(lookup_expr->key != NULL);
  }

  return NULL;
}

struct object *vm_run_bin(struct enclosing *encl, struct bin_expr *bin_expr) {
  assert(encl != NULL);
  assert(bin_expr != NULL);
  struct object *left = vm_run_expr(encl, bin_expr->left);
  struct object *right = vm_run_expr(encl, bin_expr->right);
  return handle_bin_op(encl->vm, left, right, bin_expr->op);
}

struct object *vm_run_unit(struct enclosing *encl,
                           struct unit_expr *unit_expr) {
  assert(encl != NULL);
  assert(unit_expr != NULL);
  struct object *res = vm_alloc(encl->vm, false);
  struct object *right = vm_run_expr(encl, unit_expr->right);
  if (unit_expr->op == OP_NEG) {
    switch (right->type) {
    case TYPE_U64:
    case TYPE_I64:
      res->type = TYPE_I64;
      res->i64 = -right->i64;
      break;
    case TYPE_F64:
      res->type = TYPE_F64;
      res->f64 = -right->f64;
      break;
    default:
      make_error(res, "unsupported operation for type");
      break;
    }
  } else if (unit_expr->op == OP_NOT) {
    switch (right->type) {
    case TYPE_U64:
    case TYPE_I64:
      res->type = TYPE_I64;
      res->i64 = !right->i64;
      break;
    case TYPE_F64:
      res->type = TYPE_F64;
      res->f64 = !right->f64;
      break;
    default:
      make_error(res, "unsupported operation for type");
      break;
    }
  } else {
    make_error(res, "unrecognized unitary operation");
  }

  return res;
}

struct object *vm_run_let(struct enclosing *encl, struct let_expr *let_expr) {
  assert(encl != NULL);
  assert(let_expr != NULL);
  struct enclosing forked;
  enclosing_init(&forked, encl->vm, encl);

  struct let_assigns *assign = let_expr->assigns;
  while (assign != NULL) {
    struct object *object = vm_run_expr(encl, assign->expr);
    enclosing_bind(&forked, object, strdup(assign->id));
    assign = assign->next;
  }

  struct object *res = vm_run_expr(&forked, let_expr->in_expr);
  enclosing_free(&forked);
  return res;
}

struct object *vm_run_function(struct enclosing *encl, struct function function,
                               struct call_args *expr_args,
                               struct list *value_args) {
  struct object *res = NULL;
  if (function.target == TARGET_SCRIPT) {
    struct enclosing forked;
    if (function.closure != NULL) {
      enclosing_init(&forked, encl->vm, function.closure);
    } else {
      enclosing_init(&forked, encl->vm, &encl->vm->globals);
    }
    struct def_params *recv_param = function.params;
    struct call_args *send_param = expr_args;
    struct list *send_param_value = value_args;
    while (send_param != NULL || send_param_value != NULL) {
      if (recv_param == NULL) {
        enclosing_free(&forked);

        res = vm_alloc(encl->vm, false);
        make_error(res, "function expects more arguments");
        return res;
      }

      struct object *arg_value;
      if (send_param != NULL) {
        arg_value = vm_run_expr(encl, send_param->expr);
        send_param = send_param->next;
      } else if (send_param_value != NULL) {
        arg_value = send_param_value->item;
        send_param_value = send_param_value->next;
      } else {
        res = vm_alloc(encl->vm, false);
        make_error(res, "expecting a value or expression argument");
        return res;
      }

      enclosing_bind(&forked, arg_value, strdup(recv_param->id));
      recv_param = recv_param->next;
    }

    res = vm_run_expr(&forked, function.body);
    enclosing_free(&forked);
    return res;
  } else {
    native_fun callee = function.native_call;
    struct call_args *send_param = expr_args;
    struct list *send_param_value = value_args;
    struct list *params_head = NULL;
    struct list *params_tail = NULL;

    while (send_param != NULL || send_param_value != NULL) {
      struct list *item = malloc(sizeof(struct list));
      item->next = NULL;
      if (send_param != NULL) {
        item->item = vm_run_expr(encl, send_param->expr);
        send_param = send_param->next;
      } else if (send_param_value != NULL) {
        item->item = send_param_value->item;
        send_param_value = send_param_value->next;
      } else {
        res = vm_alloc(encl->vm, false);
        make_error(res, "expecting a value or expression argument");
        return res;
      }

      if (params_head == NULL) {
        params_head = item;
      }

      if (params_tail != NULL) {
        params_tail->next = item;
      }

      params_tail = item;
    }

    struct enclosing forked;
    if (function.closure != NULL) {
      enclosing_init(&forked, encl->vm, function.closure);
    } else {
      enclosing_init(&forked, encl->vm, &encl->vm->globals);
    }

    struct object *args = vm_alloc(encl->vm, false);
    args->type = TYPE_LIST;
    args->list = params_head;

    enclosing_bind(&forked, args, strdup("args"));
    res = callee(&forked);
    enclosing_free(&forked);
    return res;
  }
}

struct object *vm_run_call(struct enclosing *encl,
                           struct call_expr *call_expr) {
  assert(encl != NULL);
  assert(call_expr != NULL);
  //
  // resolve function
  struct bind *fun_bind = enclosing_find(encl, call_expr->callee);
  if (fun_bind == NULL) {
    struct object *res = vm_alloc(encl->vm, false);
    make_errorf(res, "undefined function '%s'", call_expr->callee);
    return res;
  }

  struct object *fun_object = fun_bind->object;
  struct function function = fun_object->function;
  return vm_run_function(encl, function, call_expr->args, NULL);
}

struct object *vm_run_if(struct enclosing *encl, struct if_expr *if_expr) {
  assert(encl != NULL);
  assert(if_expr != NULL);

  struct object *res;
  struct cond_expr *cur_cond = if_expr->conds;
  while (cur_cond != NULL) {
    struct object *cond_res = vm_run_expr(encl, cur_cond->cond);
    if (cond_res->type == TYPE_UNIT) {
      res = vm_alloc(encl->vm, false);
      make_error(res, "cannot evaluate condition for unit type");
      return res;
    }

    if (cond_res->u64) {
      res = vm_run_expr(encl, cur_cond->then);
      return res;
    }

    cur_cond = cur_cond->next;
  }

  res = vm_run_expr(encl, if_expr->else_expr);
  return res;
}

struct object *vm_run_list(struct enclosing *encl,
                           struct list_expr *list_expr) {
  assert(encl != NULL);

  struct list_expr *cur = list_expr;
  struct list *head = NULL;
  struct list *tail = NULL;
  while (cur != NULL) {
    struct list *item = malloc(sizeof(struct list));
    item->next = NULL;
    item->item = vm_run_expr(encl, cur->item);

    if (head == NULL) {
      head = item;
    }

    if (tail != NULL) {
      tail->next = item;
    }

    tail = item;
    cur = cur->next;
  }

  struct object *res = vm_alloc(encl->vm, false);
  res->type = TYPE_LIST;
  res->list = head;
  return res;
}

struct object *vm_run_dict(struct enclosing *encl,
                           struct dict_expr *dict_expr) {
  assert(encl != NULL);
  struct object *res = vm_alloc(encl->vm, false);
  res->type = TYPE_DICT;
  hashmap_init(&res->hashmap, DEFAULT_HM_TOTAL_ROWS, DEFAULT_HM_MAX_OBJECTS);

  struct dict_expr *cur = dict_expr;
  while (cur != NULL) {
    struct object *value = vm_run_expr(encl, cur->value);
    enum hashmap_state state = hashmap_put(&res->hashmap, cur->key, value);
    assert(state == HM_OK);
    cur = cur->next;
  }
  return res;
}

struct object *vm_run_for(struct enclosing *encl, struct for_expr *for_expr) {
  assert(encl != NULL);
  assert(for_expr != NULL);
  struct list *res_head = NULL;
  struct list *res_tail = NULL;

  struct object *iterator_value = vm_run_expr(encl, for_expr->iterator_expr);
  if (iterator_value->type == TYPE_LIST) {
    struct list *cur_item = iterator_value->list;
    while (cur_item != NULL) {
      struct enclosing forked;
      enclosing_init(&forked, encl->vm, encl);

      char *item_handle_id =
          for_expr->handle_expr->id; // only one iterator handler is supported
      enclosing_bind(&forked, cur_item->item, strdup(item_handle_id));

      struct object *iteration_value =
          vm_run_expr(&forked, for_expr->iteration_expr);

      if (for_expr->filter_expr != NULL) {
        enclosing_bind(&forked, iteration_value, strdup("it"));
        struct object *filter_value =
            vm_run_expr(&forked, for_expr->filter_expr);
        if (filter_value->type != TYPE_ERROR &&
            filter_value->type != TYPE_FUNCTION && filter_value->u64 == 0) {
          cur_item = cur_item->next;
          enclosing_free(&forked);
          continue;
        }
      }

      struct list *new_item = malloc(sizeof(struct list));
      new_item->next = NULL;
      new_item->item = iteration_value;

      if (res_head == NULL) {
        res_head = new_item;
      }

      if (res_tail != NULL) {
        res_tail->next = new_item;
      }

      res_tail = new_item;
      enclosing_free(&forked);
      cur_item = cur_item->next;
    }
  } else if (iterator_value->type == TYPE_FUNCTION) {
    struct object *state = vm_alloc(encl->vm, false);
    bool keep_iterating = true;
    char *item_handle_id =
        for_expr->handle_expr->id; // only one iterator handler is supported

    while (keep_iterating) {
      struct list iterator_step_args = {
          .next = NULL,
          .item = state,
      };
      struct object *iterator_next = vm_run_function(
          encl, iterator_value->function, NULL, &iterator_step_args);
      if (iterator_next->type != TYPE_PAIR) {
        keep_iterating = false;
      } else {
        state = iterator_next;
        struct object *head = state->pair.head;
        assert(head != NULL);
        keep_iterating = head->bol;
      }

      struct enclosing forked;
      enclosing_init(&forked, encl->vm, encl);
      enclosing_bind(&forked, iterator_next->pair.tail, strdup(item_handle_id));

      struct object *iteration_value =
          vm_run_expr(&forked, for_expr->iteration_expr);

      if (for_expr->filter_expr != NULL) {
        enclosing_bind(&forked, iteration_value, strdup("it"));
        struct object *filter_value =
            vm_run_expr(&forked, for_expr->filter_expr);
        if (filter_value->type != TYPE_ERROR &&
            filter_value->type != TYPE_FUNCTION && filter_value->u64 == 0) {
          enclosing_free(&forked);
          continue;
        }
      }

      struct list *new_item = malloc(sizeof(struct list));
      new_item->next = NULL;
      new_item->item = iteration_value;

      if (res_head == NULL) {
        res_head = new_item;
      }

      if (res_tail != NULL) {
        res_tail->next = new_item;
      }

      res_tail = new_item;
      enclosing_free(&forked);
    }
  } else {
    struct object *res = vm_alloc(encl->vm, false);
    make_errorf(res, "cannot iterate over type %d", iterator_value->type);
    return res;
  }

  struct object *res = vm_alloc(encl->vm, false);
  res->type = TYPE_LIST;
  res->list = res_head;
  return res;
}

struct object *vm_run_reduce(struct enclosing *encl,
                             struct reduce_expr *reduce_expr) {
  assert(encl != NULL);
  assert(reduce_expr != NULL);
  struct for_expr *for_expr = reduce_expr->for_expr;
  struct object *iterator_value = vm_run_expr(encl, for_expr->iterator_expr);
  struct object *carry = vm_run_expr(encl, reduce_expr->value);

  if (iterator_value->type == TYPE_LIST) {
    struct list *cur_item = iterator_value->list;
    while (cur_item != NULL) {
      struct enclosing forked;
      enclosing_init(&forked, encl->vm, encl);

      char *item_handle_id =
          for_expr->handle_expr->id; // only one iterator handler is supported
      enclosing_bind(&forked, cur_item->item, strdup(item_handle_id));
      enclosing_bind(&forked, carry, strdup(reduce_expr->id));

      if (for_expr->filter_expr != NULL) {
        struct object *filter_value =
            vm_run_expr(&forked, for_expr->filter_expr);
        if (filter_value->type != TYPE_ERROR &&
            filter_value->type != TYPE_FUNCTION && filter_value->u64 == 0) {
          cur_item = cur_item->next;
          enclosing_free(&forked);
          continue;
        }
      }

      carry = vm_run_expr(&forked, for_expr->iteration_expr);
      enclosing_free(&forked);
      cur_item = cur_item->next;
    }
  } else if (iterator_value->type == TYPE_FUNCTION) {
    bool keep_iterating = true;
    char *item_handle_id =
        for_expr->handle_expr->id; // only one iterator handler is supported

    while (keep_iterating) {
      struct list iterator_step_args = {
          .next = NULL,
          .item = carry,
      };
      struct object *iterator_next = vm_run_function(
          encl, iterator_value->function, NULL, &iterator_step_args);
      if (iterator_next->type != TYPE_PAIR) {
        keep_iterating = false;
      } else {
        carry = iterator_next;
        struct object *head = carry->pair.head;
        assert(head != NULL);
        keep_iterating = head->bol;
      }

      struct enclosing forked;
      enclosing_init(&forked, encl->vm, encl);
      enclosing_bind(&forked, iterator_next->pair.tail, strdup(item_handle_id));

      struct object *iteration_value =
          vm_run_expr(&forked, for_expr->iteration_expr);

      if (for_expr->filter_expr != NULL) {
        enclosing_bind(&forked, iteration_value, strdup("it"));
        struct object *filter_value =
            vm_run_expr(&forked, for_expr->filter_expr);
        if (filter_value->type != TYPE_ERROR &&
            filter_value->type != TYPE_FUNCTION && filter_value->u64 == 0) {
          enclosing_free(&forked);
          continue;
        }
      }
    }
  } else {
    struct object *res = vm_alloc(encl->vm, false);
    make_errorf(res, "cannot iterate over type %d", iterator_value->type);
    return res;
  }

  return carry;
}

struct object *vm_run_lambda(struct enclosing *encl,
                             struct lambda_expr *lambda_expr) {
  assert(encl != NULL);
  assert(lambda_expr != NULL);

  struct enclosing *closure = malloc(sizeof(struct enclosing));
  enclosing_init(closure, encl->vm, &encl->vm->globals);
  enclosing_capture(closure, encl);

  struct object *object = vm_alloc(encl->vm, false);
  object->type = TYPE_FUNCTION;
  object->function = (struct function){
      .target = TARGET_SCRIPT,
      .native_call = NULL,
      .closure = closure,
      .params = lambda_expr->params,
      .body = lambda_expr->body,
      .id = NULL,
  };

  return object;
}

struct object *vm_run_expr(struct enclosing *encl, struct expr *expr) {
  assert(encl != NULL);
  assert(expr != NULL);

  struct object *res = NULL;
  switch (expr->type) {
  case EXPR_LIT:
    res = vm_run_lit(encl, expr->lit_expr);
    break;
  case EXPR_LOOKUP:
    res = vm_run_lookup(encl, expr->lookup_expr);
    break;
  case EXPR_BIN:
    res = vm_run_bin(encl, expr->bin_expr);
    break;
  case EXPR_UNIT:
    res = vm_run_unit(encl, expr->unit_expr);
    break;
  case EXPR_LET:
    res = vm_run_let(encl, expr->let_expr);
    break;
  case EXPR_CALL:
    res = vm_run_call(encl, expr->call_expr);
    break;
  case EXPR_IF:
    res = vm_run_if(encl, expr->if_expr);
    break;
  case EXPR_LIST:
    res = vm_run_list(encl, expr->list_expr);
    break;
  case EXPR_DICT:
    res = vm_run_dict(encl, expr->dict_expr);
    break;
  case EXPR_FOR:
    res = vm_run_for(encl, expr->for_expr);
    break;
  case EXPR_REDUCE:
    res = vm_run_reduce(encl, expr->reduce_expr);
    break;
  case EXPR_LAMBDA:
    res = vm_run_lambda(encl, expr->lambda_expr);
    break;
  case EXPR_DEF:
    res = vm_run_def(encl->vm, expr->def_expr);
    break;
  }

  return res;
}
