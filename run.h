#pragma once
#include "ast.h"
#include <stdint.h>

struct value;
struct bind;
struct scope;
struct function;
struct list;
typedef void (*native_def)(struct scope *, struct value *value);

enum type {
  TYPE_UNIT = 1,
  TYPE_STRING = 2,
  TYPE_I64 = 3,
  TYPE_U64 = 4,
  TYPE_F64 = 5,
  TYPE_LIST = 6,
  TYPE_ERROR = 7,
  TYPE_NATIVE_FUN = 8,
  TYPE_SCRIPT_FUN = 9,
};
struct value {
  union {
    char *str;
    struct list *list;
    struct def_expr *script_fun;
    native_def native_fun;
    uint64_t u64;
    int64_t i64;
    double f64;
  };
  enum type type;
};

struct list {
  struct list *next;
  struct value value;
};

struct value run_lit_expr(struct scope *scope, struct lit_expr *lit_expr);
struct value run_lookup_expr(struct scope *scope,
                             struct lookup_expr *lookup_expr);
struct value run_bin_expr(struct scope *scope, struct bin_expr *bin_expr);
struct value run_unit_expr(struct scope *scope, struct unit_expr *unit_expr);
struct value run_call_expr(struct scope *scope, struct call_expr *call_expr);
struct value run_def_expr(struct scope *scope, struct def_expr *def_expr);
struct value run_let_expr(struct scope *scope, struct let_expr *let_expr);
struct value run_if_expr(struct scope *scope, struct if_expr *if_expr);
struct value run_for_expr(struct scope *scope, struct for_expr *for_expr);
struct value run_list_expr(struct scope *scope, struct list_expr *expr);
struct value run_expr(struct scope *scope, struct expr *expr);

void run_all_def_exprs(struct scope *scope, struct def_exprs *def_exprs);
void run_main(struct scope *scope);

struct list *copy_list(struct list *list);
struct value copy_value(struct value value);
void print_value(struct value value);
void free_value(struct value *value);

#define ERROR_BUFFER_SIZE 100
#define make_error(res, msg)                                                   \
  do {                                                                         \
    res.type = TYPE_ERROR;                                                     \
    const char *errmsg = (msg);                                                \
    const size_t msgsize = strlen(errmsg);                                     \
    res.str = malloc(msgsize + 1);                                             \
    memset(res.str, 0L, msgsize);                                              \
    memcpy(res.str, errmsg, msgsize);                                          \
  } while (0);

#define make_errorf(res, msg, ...)                                             \
  do {                                                                         \
    res.type = TYPE_ERROR;                                                     \
    res.str = malloc(ERROR_BUFFER_SIZE);                                       \
    memset(res.str, 0L, ERROR_BUFFER_SIZE);                                    \
    sprintf(res.str, msg, __VA_ARGS__);                                        \
  } while (0);
