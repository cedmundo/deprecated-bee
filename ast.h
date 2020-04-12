#pragma once

struct def_exprs;
struct expr;
struct lit_expr;
struct lookup_expr;
struct bin_expr;
struct unit_expr;
struct call_args;
struct call_expr;
struct let_assigns;
struct let_expr;
struct def_params;
struct def_expr;
struct if_expr;
struct for_handles;
struct for_expr;
struct list_expr;

struct def_exprs {
  struct def_exprs *next;
  struct def_expr *def_expr;
};

enum expr_type {
  EXPR_LIT,
  EXPR_LOOKUP,
  EXPR_BIN,
  EXPR_UNIT,
  EXPR_CALL,
  EXPR_LET,
  EXPR_DEF,
  EXPR_IF,
  EXPR_FOR,
  EXPR_LIST,
};
struct expr {
  union {
    struct lit_expr *lit_expr;
    struct lookup_expr *lookup_expr;
    struct bin_expr *bin_expr;
    struct unit_expr *unit_expr;
    struct call_expr *call_expr;
    struct let_expr *let_expr;
    struct def_expr *def_expr;
    struct if_expr *if_expr;
    struct for_expr *for_expr;
    struct list_expr *list_expr;
  };
  enum expr_type type;
};

enum lit_type { LIT_NUMBER, LIT_STRING };
struct lit_expr {
  char *raw_value;
  enum lit_type type;
};

struct lookup_expr {
  char *id;
};

enum bin_op {
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MOD,
  OP_ANDS,
  OP_AND,
  OP_ORS,
  OP_OR,
  OP_XOR,
  OP_EQ,
  OP_NEQ,
  OP_LT,
  OP_LE,
  OP_GT,
  OP_GE
};
struct bin_expr {
  struct expr *left;
  struct expr *right;
  enum bin_op op;
};

enum unit_op {
  OP_NOT,
  OP_NEG,
};
struct unit_expr {
  struct expr *right;
  enum unit_op op;
};

struct call_args {
  struct call_args *next;
  struct expr *expr;
};

struct call_expr {
  char *callee;
  struct call_args *args;
};

struct let_assigns {
  struct let_assigns *next;
  char *id;
  struct expr *expr;
};

struct let_expr {
  struct let_assigns *assigns;
  struct expr *in_expr;
};

struct def_params {
  struct def_params *next;
  char *id;
};

struct def_expr {
  char *id;
  struct def_params *params;
  struct expr *body;
};

struct if_expr {
  struct expr *cond_expr;
  struct expr *then_expr;
  struct expr *else_expr;
};

struct for_handles {
  struct for_handles *next;
  char *id;
};

struct for_expr {
  struct expr *iteration_expr;
  struct expr *iterator_expr;
  struct for_handles *handle_expr;
};

struct list_expr {
  struct list_expr *next;
  struct expr *item;
};

struct def_exprs *make_def_exprs(struct def_expr *def_expr);
struct def_exprs *append_def_exprs(struct def_exprs *left,
                                   struct def_expr *def_expr);

struct expr *make_expr_from_lit(struct lit_expr *expr);
struct expr *make_expr_from_lookup(struct lookup_expr *expr);
struct expr *make_expr_from_bin(struct bin_expr *expr);
struct expr *make_expr_from_unit(struct unit_expr *expr);
struct expr *make_expr_from_call(struct call_expr *expr);
struct expr *make_expr_from_let(struct let_expr *expr);
struct expr *make_expr_from_def(struct def_expr *expr);
struct expr *make_expr_from_if(struct if_expr *expr);
struct expr *make_expr_from_for(struct for_expr *expr);
struct expr *make_expr_from_list(struct list_expr *expr);

struct lit_expr *make_lit_expr(enum lit_type type, char *v);

struct lookup_expr *make_lookup_expr(char *id);

struct bin_expr *make_bin_expr(struct expr *left, struct expr *right,
                               enum bin_op op);

struct unit_expr *make_unit_expr(struct expr *right, enum unit_op op);

struct call_args *make_call_args(struct expr *expr);
struct call_args *append_call_args(struct call_args *left, struct expr *expr);

struct call_expr *make_call_expr(char *callee, struct call_args *args);

struct let_assigns *make_let_assigns(char *id, struct expr *expr);
struct let_assigns *append_let_assigns(struct let_assigns *left, char *id,
                                       struct expr *expr);

struct let_expr *make_let_expr(struct let_assigns *assigns,
                               struct expr *in_expr);

struct def_params *make_def_params(char *id);
struct def_params *append_def_params(struct def_params *left, char *id);

struct def_expr *make_def_expr(char *id, struct def_params *params,
                               struct expr *body);

struct if_expr *make_if_expr(struct expr *then_expr, struct expr *cond_expr,
                             struct expr *else_expr);

struct for_handles *make_for_handles(char *id);
struct for_handles *append_for_handles(struct for_handles *left, char *id);

struct for_expr *make_for_expr(struct expr *iteration,
                               struct for_handles *handles,
                               struct expr *iterator);

struct list_expr *make_list_expr(struct expr *item);
struct list_expr *append_list_expr(struct list_expr *left, struct expr *expr);

void free_def_exprs(struct def_exprs *def_exprs);
void free_expr(struct expr *expr);
void free_lit_expr(struct lit_expr *lit_expr);
void free_lookup_expr(struct lookup_expr *lookup_expr);
void free_bin_expr(struct bin_expr *bin_expr);
void free_unit_expr(struct unit_expr *unit_expr);
void free_call_args(struct call_args *call_args);
void free_call_expr(struct call_expr *call_expr);
void free_let_assigns(struct let_assigns *let_assigns);
void free_let_expr(struct let_expr *let_expr);
void free_def_params(struct def_params *def_params);
void free_def_expr(struct def_expr *def_expr);
void free_if_expr(struct if_expr *if_expr);
void free_for_handles(struct for_handles *for_handles);
void free_for_expr(struct for_expr *for_expr);
void free_list_expr(struct list_expr *list_expr);

#define walk_to_last(i, c, block)                                              \
  do {                                                                         \
    i *last = c;                                                               \
    if (last != NULL) {                                                        \
      while (last->next != NULL) {                                             \
        last = last->next;                                                     \
      }                                                                        \
    }                                                                          \
    block                                                                      \
  } while (0)
