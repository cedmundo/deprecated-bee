#pragma once
#include "ast.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

struct vm;
struct object;
struct enclosing;
struct pair {
  struct object *head;
  struct object *tail;
};

struct list {
  struct list *next;
  struct object *item;
};

typedef struct object *(*native_fun)(struct enclosing *);

enum function_target { TARGET_SCRIPT, TARGET_NATIVE };
struct function {
  native_fun native_call;
  struct def_params *params;
  struct expr *body;
  char *id;
  enum function_target target;
};

enum gc_flag { GC_UNMARKED = 0, GC_MARKED, GC_ROOT };
enum object_type {
  TYPE_UNIT,
  TYPE_NIL,
  TYPE_BOL,
  TYPE_U64,
  TYPE_I64,
  TYPE_F64,
  TYPE_ERROR,
  TYPE_STRING,
  TYPE_PAIR,
  TYPE_LIST,
  TYPE_FUNCTION,
};

struct object {
  struct object *gc_next;
  union {
    uint8_t bol;
    uint64_t u64;
    int64_t i64;
    double f64;
    char *error;
    char *string;
    struct list *list;
    struct pair pair;
    struct function function;
  };
  enum object_type type;
  enum gc_flag flag;
};

struct bind {
  struct bind *next;
  struct object *object;
  char *id;
};

struct enclosing {
  struct vm *vm;
  struct enclosing *parent;
  struct bind *head;
  struct bind *tail;
};

struct vm {
  struct object *heap_head;
  struct object *heap_tail;
  struct enclosing globals;
  struct def_exprs *source_exprs;
  struct timespec last_gc;
};

void vm_init(struct vm *vm);
void vm_free(struct vm *vm);

size_t vm_mark_all(struct vm *vm);
size_t vm_sweep(struct vm *vm);
size_t vm_gc(struct vm *vm);

struct object *vm_alloc(struct vm *vm, bool is_root);
size_t object_free(struct object *obj);
size_t object_mark(struct object *obj);
size_t object_print(struct object *value, bool debug);

void enclosing_init(struct enclosing *e, struct vm *vm,
                    struct enclosing *parent);
void enclosing_free(struct enclosing *e);
void enclosing_bind(struct enclosing *e, struct object *object, char *id);
struct bind *enclosing_find(struct enclosing *e, char *id);

void vm_define_all(struct vm *vm, struct def_exprs *defs);
struct object *vm_run_main(struct vm *vm);
struct object *vm_run_def(struct vm *vm, struct def_expr *def);

struct object *vm_run_lit(struct enclosing *encl, struct lit_expr *lit_expr);
struct object *vm_run_lookup(struct enclosing *encl,
                             struct lookup_expr *lookup_expr);
struct object *vm_run_bin(struct enclosing *encl, struct bin_expr *bin_expr);
struct object *vm_run_unit(struct enclosing *encl, struct unit_expr *unit_expr);
struct object *vm_run_let(struct enclosing *encl, struct let_expr *let_expr);
struct object *vm_run_call(struct enclosing *encl, struct call_expr *call_expr);
struct object *vm_run_if(struct enclosing *encl, struct if_expr *if_expr);
struct object *vm_run_list(struct enclosing *encl, struct list_expr *list_expr);
struct object *vm_run_for(struct enclosing *encl, struct for_expr *for_expr);
struct object *vm_run_reduce(struct enclosing *encl,
                             struct reduce_expr *reduce_expr);
struct object *vm_run_lambda(struct enclosing *encl,
                             struct lambda_expr *lambda_expr);
struct object *vm_run_expr(struct enclosing *encl, struct expr *expr);

#define DEFAULT_GC_INTERVAL_NS 100000
#define make_error(res, msg)                                                   \
  do {                                                                         \
    res->type = TYPE_ERROR;                                                    \
    res->error = malloc(sizeof(char) * 200);                                   \
    memset(res->error, 0L, sizeof(char) * 200);                                \
    sprintf(res->error, "%s", msg);                                            \
  } while (0);

#define make_errorf(res, msg, ...)                                             \
  do {                                                                         \
    res->type = TYPE_ERROR;                                                    \
    res->error = malloc(sizeof(char) * 200);                                   \
    memset(res->error, 0L, sizeof(char) * 200);                                \
    sprintf(res->error, msg, __VA_ARGS__);                                     \
  } while (0);
