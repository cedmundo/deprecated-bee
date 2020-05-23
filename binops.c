#include "binops.h"
#include "vm.h"
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// i64
struct object *handle_i64_u64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_I64;

  switch (op) {
  case OP_ADD:
    res->i64 = left->i64 + right->u64;
    break;
  case OP_SUB:
    res->i64 = left->i64 - right->u64;
    break;
  case OP_MUL:
    res->i64 = left->i64 * right->u64;
    break;
  case OP_DIV:
    res->i64 = left->i64 / right->u64;
    break;
  case OP_MOD:
    res->i64 = left->i64 % right->u64;
    break;
  case OP_ANDS:
    res->i64 = left->i64 && right->u64;
    break;
  case OP_ORS:
    res->i64 = left->i64 || right->u64;
    break;
  case OP_AND:
    res->i64 = left->i64 & right->u64;
    break;
  case OP_OR:
    res->i64 = left->i64 | right->u64;
    break;
  case OP_XOR:
    res->i64 = left->i64 ^ right->u64;
    break;
  case OP_EQ:
    res->i64 = left->i64 == right->u64;
    break;
  case OP_NEQ:
    res->i64 = left->i64 != right->u64;
    break;
  case OP_LT:
    res->i64 = left->i64 < right->u64;
    break;
  case OP_LE:
    res->i64 = left->i64 <= right->u64;
    break;
  case OP_GT:
    res->i64 = left->i64 > right->u64;
    break;
  case OP_GE:
    res->i64 = left->i64 >= right->u64;
    break;
  }
  return res;
}

struct object *handle_i64_i64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_I64;

  switch (op) {
  case OP_ADD:
    res->i64 = left->i64 + right->i64;
    break;
  case OP_SUB:
    res->i64 = left->i64 - right->i64;
    break;
  case OP_MUL:
    res->i64 = left->i64 * right->i64;
    break;
  case OP_DIV:
    res->i64 = left->i64 / right->i64;
    break;
  case OP_MOD:
    res->i64 = left->i64 % right->i64;
    break;
  case OP_ANDS:
    res->i64 = left->i64 && right->i64;
    break;
  case OP_ORS:
    res->i64 = left->i64 || right->i64;
    break;
  case OP_AND:
    res->i64 = left->i64 & right->i64;
    break;
  case OP_OR:
    res->i64 = left->i64 | right->i64;
    break;
  case OP_XOR:
    res->i64 = left->i64 ^ right->i64;
    break;
  case OP_EQ:
    res->i64 = left->i64 == right->i64;
    break;
  case OP_NEQ:
    res->i64 = left->i64 != right->i64;
    break;
  case OP_LT:
    res->i64 = left->i64 < right->i64;
    break;
  case OP_LE:
    res->i64 = left->i64 <= right->i64;
    break;
  case OP_GT:
    res->i64 = left->i64 > right->i64;
    break;
  case OP_GE:
    res->i64 = left->i64 >= right->i64;
    break;
  }
  return res;
}

struct object *handle_i64_f64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_F64;

  switch (op) {
  case OP_ADD:
    res->f64 = left->i64 + right->f64;
    break;
  case OP_SUB:
    res->f64 = left->i64 - right->f64;
    break;
  case OP_MUL:
    res->f64 = left->i64 * right->f64;
    break;
  case OP_DIV:
    res->f64 = left->i64 / right->f64;
    break;
  case OP_ANDS:
    res->f64 = left->i64 && right->f64;
    break;
  case OP_ORS:
    res->f64 = left->i64 || right->f64;
    break;
  case OP_EQ:
    res->f64 = left->i64 == right->f64;
    break;
  case OP_NEQ:
    res->f64 = left->i64 != right->f64;
    break;
  case OP_LT:
    res->f64 = left->i64 < right->f64;
    break;
  case OP_LE:
    res->f64 = left->i64 <= right->f64;
    break;
  case OP_GT:
    res->f64 = left->i64 > right->f64;
    break;
  case OP_GE:
    res->f64 = left->i64 >= right->i64;
    break;
  default:
    make_error(res, "undefined operation between i64 and f64");
  }
  return res;
}

struct object *handle_i64_str(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_STRING;

  if (op != OP_ADD) {
    make_error(res, "undefined operation between i64 and f64");
    return res;
  }

  char buffer[64];
  snprintf(buffer, 64L, "%ld", left->i64);
  size_t new_size = strlen(buffer) + strlen(right->string);
  char *new_str = malloc(new_size + 1);

  memset(new_str, 0L, new_size + 1);
  sprintf(new_str, "%s%s", buffer, right->string);

  res->string = new_str;
  return res;
}

// u64
struct object *handle_u64_u64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_U64;

  switch (op) {
  case OP_ADD:
    res->u64 = left->u64 + right->u64;
    break;
  case OP_SUB:
    res->u64 = left->u64 - right->u64;
    break;
  case OP_MUL:
    res->u64 = left->u64 * right->u64;
    break;
  case OP_DIV:
    res->u64 = left->u64 / right->u64;
    break;
  case OP_MOD:
    res->u64 = left->u64 % right->u64;
    break;
  case OP_ANDS:
    res->u64 = left->u64 && right->u64;
    break;
  case OP_ORS:
    res->u64 = left->u64 || right->u64;
    break;
  case OP_AND:
    res->u64 = left->u64 & right->u64;
    break;
  case OP_OR:
    res->u64 = left->u64 | right->u64;
    break;
  case OP_XOR:
    res->u64 = left->u64 ^ right->u64;
    break;
  case OP_EQ:
    res->u64 = left->u64 == right->u64;
    break;
  case OP_NEQ:
    res->u64 = left->u64 != right->u64;
    break;
  case OP_LT:
    res->u64 = left->u64 < right->u64;
    break;
  case OP_LE:
    res->u64 = left->u64 <= right->u64;
    break;
  case OP_GT:
    res->u64 = left->u64 > right->u64;
    break;
  case OP_GE:
    res->u64 = left->u64 >= right->u64;
    break;
  }
  return res;
}

struct object *handle_u64_i64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_I64;

  switch (op) {
  case OP_ADD:
    res->u64 = left->u64 + right->i64;
    break;
  case OP_SUB:
    res->u64 = left->u64 - right->i64;
    break;
  case OP_MUL:
    res->u64 = left->u64 * right->i64;
    break;
  case OP_DIV:
    res->u64 = left->u64 / right->i64;
    break;
  case OP_MOD:
    res->u64 = left->u64 % right->i64;
    break;
  case OP_ANDS:
    res->u64 = left->u64 && right->i64;
    break;
  case OP_ORS:
    res->u64 = left->u64 || right->i64;
    break;
  case OP_AND:
    res->u64 = left->u64 & right->i64;
    break;
  case OP_OR:
    res->u64 = left->u64 | right->i64;
    break;
  case OP_XOR:
    res->u64 = left->u64 ^ right->i64;
    break;
  case OP_EQ:
    res->u64 = left->u64 == right->i64;
    break;
  case OP_NEQ:
    res->u64 = left->u64 != right->i64;
    break;
  case OP_LT:
    res->u64 = left->u64 < right->i64;
    break;
  case OP_LE:
    res->u64 = left->u64 <= right->i64;
    break;
  case OP_GT:
    res->u64 = left->u64 > right->i64;
    break;
  case OP_GE:
    res->u64 = left->u64 >= right->i64;
    break;
  }
  return res;
}

struct object *handle_u64_f64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_F64;

  switch (op) {
  case OP_ADD:
    res->f64 = left->u64 + right->f64;
    break;
  case OP_SUB:
    res->f64 = left->u64 - right->f64;
    break;
  case OP_MUL:
    res->f64 = left->u64 * right->f64;
    break;
  case OP_DIV:
    res->f64 = left->u64 / right->f64;
    break;
  case OP_ANDS:
    res->f64 = left->u64 && right->f64;
    break;
  case OP_ORS:
    res->f64 = left->u64 || right->f64;
    break;
  case OP_EQ:
    res->f64 = left->u64 == right->f64;
    break;
  case OP_NEQ:
    res->f64 = left->u64 != right->f64;
    break;
  case OP_LT:
    res->f64 = left->u64 < right->f64;
    break;
  case OP_LE:
    res->f64 = left->u64 <= right->f64;
    break;
  case OP_GT:
    res->f64 = left->u64 > right->f64;
    break;
  case OP_GE:
    res->f64 = left->u64 >= right->i64;
    break;
  default:
    make_error(res, "undefined operation between u64 and f64");
  }
  return res;
}

struct object *handle_u64_str(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_STRING;

  if (op != OP_ADD) {
    make_error(res, "undefined operation between u64 and string");
    return res;
  }

  char buffer[64];
  snprintf(buffer, 64L, "%lu", left->u64);
  size_t new_size = strlen(buffer) + strlen(right->string);
  char *new_str = malloc(new_size + 1);
  memset(new_str, 0L, new_size + 1);
  sprintf(new_str, "%s%s", buffer, right->string);

  res->string = new_str;
  return res;
}

// f64
struct object *handle_f64_u64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_F64;

  switch (op) {
  case OP_ADD:
    res->f64 = left->f64 + right->u64;
    break;
  case OP_SUB:
    res->f64 = left->f64 - right->u64;
    break;
  case OP_MUL:
    res->f64 = left->f64 * right->u64;
    break;
  case OP_DIV:
    res->f64 = left->f64 / right->u64;
    break;
  case OP_ANDS:
    res->f64 = left->f64 && right->u64;
    break;
  case OP_ORS:
    res->f64 = left->f64 || right->u64;
    break;
  case OP_EQ:
    res->f64 = left->f64 == right->u64;
    break;
  case OP_NEQ:
    res->f64 = left->f64 != right->u64;
    break;
  case OP_LT:
    res->f64 = left->f64 < right->u64;
    break;
  case OP_LE:
    res->f64 = left->f64 <= right->u64;
    break;
  case OP_GT:
    res->f64 = left->f64 > right->u64;
    break;
  case OP_GE:
    res->f64 = left->f64 >= right->i64;
    break;
  default:
    make_error(res, "undefined operation between f64 and u64");
  }
  return res;
}

struct object *handle_f64_i64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_F64;

  switch (op) {
  case OP_ADD:
    res->f64 = left->f64 + right->i64;
    break;
  case OP_SUB:
    res->f64 = left->f64 - right->i64;
    break;
  case OP_MUL:
    res->f64 = left->f64 * right->i64;
    break;
  case OP_DIV:
    res->f64 = left->f64 / right->i64;
    break;
  case OP_ANDS:
    res->f64 = left->f64 && right->i64;
    break;
  case OP_ORS:
    res->f64 = left->f64 || right->i64;
    break;
  case OP_EQ:
    res->f64 = left->f64 == right->i64;
    break;
  case OP_NEQ:
    res->f64 = left->f64 != right->i64;
    break;
  case OP_LT:
    res->f64 = left->f64 < right->i64;
    break;
  case OP_LE:
    res->f64 = left->f64 <= right->i64;
    break;
  case OP_GT:
    res->f64 = left->f64 > right->i64;
    break;
  case OP_GE:
    res->f64 = left->f64 >= right->i64;
    break;
  default:
    make_error(res, "undefined operation between f64 and i64");
  }
  return res;
}

struct object *handle_f64_f64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_F64;

  switch (op) {
  case OP_ADD:
    res->f64 = left->f64 + right->f64;
    break;
  case OP_SUB:
    res->f64 = left->f64 - right->f64;
    break;
  case OP_MUL:
    res->f64 = left->f64 * right->f64;
    break;
  case OP_DIV:
    res->f64 = left->f64 / right->f64;
    break;
  case OP_ANDS:
    res->f64 = left->f64 && right->f64;
    break;
  case OP_ORS:
    res->f64 = left->f64 || right->f64;
    break;
  case OP_EQ:
    res->f64 = left->f64 == right->f64;
    break;
  case OP_NEQ:
    res->f64 = left->f64 != right->f64;
    break;
  case OP_LT:
    res->f64 = left->f64 < right->f64;
    break;
  case OP_LE:
    res->f64 = left->f64 <= right->f64;
    break;
  case OP_GT:
    res->f64 = left->f64 > right->f64;
    break;
  case OP_GE:
    res->f64 = left->f64 >= right->f64;
    break;
  default:
    make_error(res, "undefined operation between f64 and f64");
  }
  return res;
}

struct object *handle_f64_str(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_STRING;

  if (op != OP_ADD) {
    make_error(res, "undefined operation between f64 and string");
    return res;
  }

  char buffer[200];
  snprintf(buffer, 64L, "%lf", left->f64);
  size_t new_size = strlen(buffer) + strlen(right->string);
  char *new_str = malloc(new_size + 1);
  memset(new_str, 0L, new_size + 1);
  sprintf(new_str, "%s%s", buffer, right->string);

  res->string = new_str;
  return res;
}

// string
struct object *handle_str_u64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_STRING;

  if (op != OP_ADD) {
    make_error(res, "undefined operation between string and u64");
    return res;
  }

  char buffer[200];
  snprintf(buffer, 64L, "%lu", right->u64);
  size_t new_size = strlen(buffer) + strlen(left->string);
  char *new_str = malloc(new_size + 1);
  memset(new_str, 0L, new_size + 1);
  sprintf(new_str, "%s%s", left->string, buffer);

  res->string = new_str;
  return res;
}

struct object *handle_str_i64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_STRING;

  if (op != OP_ADD) {
    make_error(res, "undefined operation between string and i64");
    return res;
  }

  char buffer[200];
  snprintf(buffer, 64L, "%ld", right->i64);
  size_t new_size = strlen(buffer) + strlen(left->string);
  char *new_str = malloc(new_size + 1);
  memset(new_str, 0L, new_size + 1);
  sprintf(new_str, "%s%s", left->string, buffer);

  res->string = new_str;
  return res;
}

struct object *handle_str_f64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  res->type = TYPE_STRING;

  if (op != OP_ADD) {
    make_error(res, "undefined operation between string and f64");
    return res;
  }

  char buffer[200];
  snprintf(buffer, 64L, "%lf", right->f64);
  size_t new_size = strlen(buffer) + strlen(left->string);
  char *new_str = malloc(new_size + 1);

  memset(new_str, 0L, new_size + 1);
  sprintf(new_str, "%s%s", left->string, buffer);

  res->string = new_str;
  return res;
}

struct object *handle_str_str(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op) {
  struct object *res = vm_alloc(vm, false);
  if (op == OP_ADD) {
    res->type = TYPE_STRING;
    size_t left_size = strlen(left->string);
    size_t right_size = strlen(right->string);
    size_t new_size = left_size + right_size + 1;
    res->string = malloc(new_size);
    memset(res->string, 0L, new_size);
    sprintf(res->string, "%s%s", left->string, right->string);
  } else if (op == OP_EQ) {
    res->type = TYPE_I64;
    res->i64 = strcmp(left->string, right->string) == 0;
  } else if (op == OP_NEQ) {
    res->type = TYPE_I64;
    res->i64 = strcmp(left->string, right->string) != 0;
  } else {
    make_error(res, "undefined operation between strings");
  }

  return res;
}

struct object *handle_bin_op(struct vm *vm, struct object *left,
                             struct object *right, enum bin_op op) {
  struct object *res;
  int ltype = left->type;
  if (ltype == TYPE_ERROR) {
    return left;
  }

  int rtype = right->type;
  if (rtype == TYPE_ERROR) {
    return right;
  }

  if (ltype == TYPE_U64 && rtype == TYPE_U64) {
    res = handle_u64_u64(vm, left, right, op);
  } else if (ltype == TYPE_U64 && rtype == TYPE_I64) {
    res = handle_u64_i64(vm, left, right, op);
  } else if (ltype == TYPE_U64 && rtype == TYPE_F64) {
    res = handle_u64_f64(vm, left, right, op);
  } else if (ltype == TYPE_U64 && rtype == TYPE_STRING) {
    res = handle_u64_str(vm, left, right, op);
  } else if (ltype == TYPE_I64 && rtype == TYPE_U64) {
    res = handle_i64_u64(vm, left, right, op);
  } else if (ltype == TYPE_I64 && rtype == TYPE_I64) {
    res = handle_i64_i64(vm, left, right, op);
  } else if (ltype == TYPE_I64 && rtype == TYPE_F64) {
    res = handle_i64_f64(vm, left, right, op);
  } else if (ltype == TYPE_I64 && rtype == TYPE_STRING) {
    res = handle_i64_str(vm, left, right, op);
  } else if (ltype == TYPE_F64 && rtype == TYPE_U64) {
    res = handle_f64_u64(vm, left, right, op);
  } else if (ltype == TYPE_F64 && rtype == TYPE_I64) {
    res = handle_f64_i64(vm, left, right, op);
  } else if (ltype == TYPE_F64 && rtype == TYPE_F64) {
    res = handle_f64_f64(vm, left, right, op);
  } else if (ltype == TYPE_F64 && rtype == TYPE_STRING) {
    res = handle_f64_str(vm, left, right, op);
  } else if (ltype == TYPE_STRING && rtype == TYPE_U64) {
    res = handle_str_u64(vm, left, right, op);
  } else if (ltype == TYPE_STRING && rtype == TYPE_I64) {
    res = handle_str_i64(vm, left, right, op);
  } else if (ltype == TYPE_STRING && rtype == TYPE_F64) {
    res = handle_str_f64(vm, left, right, op);
  } else if (ltype == TYPE_STRING && rtype == TYPE_STRING) {
    res = handle_str_str(vm, left, right, op);
  } else {
    res = vm_alloc(vm, false);
    make_error(res, "undefined binary operation");
  }

  return res;
}
