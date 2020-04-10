#include "binops.h"
#include "run.h"
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// i64
struct value handle_i64_u64(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_I64};
  switch (op) {
  case OP_ADD:
    res.i64 = left.i64 + right.u64;
    break;
  case OP_SUB:
    res.i64 = left.i64 - right.u64;
    break;
  case OP_MUL:
    res.i64 = left.i64 * right.u64;
    break;
  case OP_DIV:
    res.i64 = left.i64 / right.u64;
    break;
  case OP_MOD:
    res.i64 = left.i64 % right.u64;
    break;
  case OP_ANDS:
    res.i64 = left.i64 && right.u64;
    break;
  case OP_ORS:
    res.i64 = left.i64 || right.u64;
    break;
  case OP_AND:
    res.i64 = left.i64 & right.u64;
    break;
  case OP_OR:
    res.i64 = left.i64 | right.u64;
    break;
  case OP_XOR:
    res.i64 = left.i64 ^ right.u64;
    break;
  case OP_EQ:
    res.i64 = left.i64 == right.u64;
    break;
  case OP_NEQ:
    res.i64 = left.i64 != right.u64;
    break;
  case OP_LT:
    res.i64 = left.i64 < right.u64;
    break;
  case OP_LE:
    res.i64 = left.i64 <= right.u64;
    break;
  case OP_GT:
    res.i64 = left.i64 > right.u64;
    break;
  case OP_GE:
    res.i64 = left.i64 >= right.u64;
    break;
  }
  return res;
}

struct value handle_i64_i64(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_I64};
  switch (op) {
  case OP_ADD:
    res.i64 = left.i64 + right.i64;
    break;
  case OP_SUB:
    res.i64 = left.i64 - right.i64;
    break;
  case OP_MUL:
    res.i64 = left.i64 * right.i64;
    break;
  case OP_DIV:
    res.i64 = left.i64 / right.i64;
    break;
  case OP_MOD:
    res.i64 = left.i64 % right.i64;
    break;
  case OP_ANDS:
    res.i64 = left.i64 && right.i64;
    break;
  case OP_ORS:
    res.i64 = left.i64 || right.i64;
    break;
  case OP_AND:
    res.i64 = left.i64 & right.i64;
    break;
  case OP_OR:
    res.i64 = left.i64 | right.i64;
    break;
  case OP_XOR:
    res.i64 = left.i64 ^ right.i64;
    break;
  case OP_EQ:
    res.i64 = left.i64 == right.i64;
    break;
  case OP_NEQ:
    res.i64 = left.i64 != right.i64;
    break;
  case OP_LT:
    res.i64 = left.i64 < right.i64;
    break;
  case OP_LE:
    res.i64 = left.i64 <= right.i64;
    break;
  case OP_GT:
    res.i64 = left.i64 > right.i64;
    break;
  case OP_GE:
    res.i64 = left.i64 >= right.i64;
    break;
  }
  return res;
}

struct value handle_i64_f64(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_F64};
  switch (op) {
  case OP_ADD:
    res.f64 = left.i64 + right.f64;
    break;
  case OP_SUB:
    res.f64 = left.i64 - right.f64;
    break;
  case OP_MUL:
    res.f64 = left.i64 * right.f64;
    break;
  case OP_DIV:
    res.f64 = left.i64 / right.f64;
    break;
  case OP_ANDS:
    res.f64 = left.i64 && right.f64;
    break;
  case OP_ORS:
    res.f64 = left.i64 || right.f64;
    break;
  case OP_EQ:
    res.f64 = left.i64 == right.f64;
    break;
  case OP_NEQ:
    res.f64 = left.i64 != right.f64;
    break;
  case OP_LT:
    res.f64 = left.i64 < right.f64;
    break;
  case OP_LE:
    res.f64 = left.i64 <= right.f64;
    break;
  case OP_GT:
    res.f64 = left.i64 > right.f64;
    break;
  case OP_GE:
    res.f64 = left.i64 >= right.i64;
    break;
  default:
    make_error(res, "unsupported or invalid operation over i64 and f64");
  }
  return res;
}

struct value handle_i64_str(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_STRING};
  if (op != OP_ADD) {
    make_error(res, "unsupported or invalid operation over i64 and f64");
    return res;
  }

  char buffer[64];
  snprintf(buffer, 64L, "%ld", left.i64);
  size_t new_size = strlen(buffer) + strlen(right.str);
  char *new_str = malloc(new_size + 1);
  if (new_str == NULL) {
    free(new_str);
    make_error(res, "no more memory");
    return res;
  }

  memset(new_str, 0L, new_size + 1);
  sprintf(new_str, "%s%s", buffer, right.str);

  res.str = new_str;
  return res;
}

// u64
struct value handle_u64_u64(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_U64};
  switch (op) {
  case OP_ADD:
    res.u64 = left.u64 + right.u64;
    break;
  case OP_SUB:
    res.u64 = left.u64 - right.u64;
    break;
  case OP_MUL:
    res.u64 = left.u64 * right.u64;
    break;
  case OP_DIV:
    res.u64 = left.u64 / right.u64;
    break;
  case OP_MOD:
    res.u64 = left.u64 % right.u64;
    break;
  case OP_ANDS:
    res.u64 = left.u64 && right.u64;
    break;
  case OP_ORS:
    res.u64 = left.u64 || right.u64;
    break;
  case OP_AND:
    res.u64 = left.u64 & right.u64;
    break;
  case OP_OR:
    res.u64 = left.u64 | right.u64;
    break;
  case OP_XOR:
    res.u64 = left.u64 ^ right.u64;
    break;
  case OP_EQ:
    res.u64 = left.u64 == right.u64;
    break;
  case OP_NEQ:
    res.u64 = left.u64 != right.u64;
    break;
  case OP_LT:
    res.u64 = left.u64 < right.u64;
    break;
  case OP_LE:
    res.u64 = left.u64 <= right.u64;
    break;
  case OP_GT:
    res.u64 = left.u64 > right.u64;
    break;
  case OP_GE:
    res.u64 = left.u64 >= right.u64;
    break;
  }
  return res;
}

struct value handle_u64_i64(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_I64};
  switch (op) {
  case OP_ADD:
    res.u64 = left.u64 + right.i64;
    break;
  case OP_SUB:
    res.u64 = left.u64 - right.i64;
    break;
  case OP_MUL:
    res.u64 = left.u64 * right.i64;
    break;
  case OP_DIV:
    res.u64 = left.u64 / right.i64;
    break;
  case OP_MOD:
    res.u64 = left.u64 % right.i64;
    break;
  case OP_ANDS:
    res.u64 = left.u64 && right.i64;
    break;
  case OP_ORS:
    res.u64 = left.u64 || right.i64;
    break;
  case OP_AND:
    res.u64 = left.u64 & right.i64;
    break;
  case OP_OR:
    res.u64 = left.u64 | right.i64;
    break;
  case OP_XOR:
    res.u64 = left.u64 ^ right.i64;
    break;
  case OP_EQ:
    res.u64 = left.u64 == right.i64;
    break;
  case OP_NEQ:
    res.u64 = left.u64 != right.i64;
    break;
  case OP_LT:
    res.u64 = left.u64 < right.i64;
    break;
  case OP_LE:
    res.u64 = left.u64 <= right.i64;
    break;
  case OP_GT:
    res.u64 = left.u64 > right.i64;
    break;
  case OP_GE:
    res.u64 = left.u64 >= right.i64;
    break;
  }
  return res;
}

struct value handle_u64_f64(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_F64};
  switch (op) {
  case OP_ADD:
    res.f64 = left.u64 + right.f64;
    break;
  case OP_SUB:
    res.f64 = left.u64 - right.f64;
    break;
  case OP_MUL:
    res.f64 = left.u64 * right.f64;
    break;
  case OP_DIV:
    res.f64 = left.u64 / right.f64;
    break;
  case OP_ANDS:
    res.f64 = left.u64 && right.f64;
    break;
  case OP_ORS:
    res.f64 = left.u64 || right.f64;
    break;
  case OP_EQ:
    res.f64 = left.u64 == right.f64;
    break;
  case OP_NEQ:
    res.f64 = left.u64 != right.f64;
    break;
  case OP_LT:
    res.f64 = left.u64 < right.f64;
    break;
  case OP_LE:
    res.f64 = left.u64 <= right.f64;
    break;
  case OP_GT:
    res.f64 = left.u64 > right.f64;
    break;
  case OP_GE:
    res.f64 = left.u64 >= right.i64;
    break;
  default:
    make_error(res, "unsupported or invalid operation between u64 and f64");
  }
  return res;
}

struct value handle_u64_str(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_STRING};
  if (op != OP_ADD) {
    make_error(res, "unsupported or invalid operation between u64 and str");
    return res;
  }

  char buffer[64];
  snprintf(buffer, 64L, "%lu", left.u64);
  size_t new_size = strlen(buffer) + strlen(right.str);
  char *new_str = malloc(new_size + 1);
  if (new_str == NULL) {
    free(new_str);
    make_error(res, "no more memory");
    return res;
  }

  memset(new_str, 0L, new_size + 1);
  sprintf(new_str, "%s%s", buffer, right.str);

  res.str = new_str;
  return res;
}

// f64
struct value handle_f64_u64(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_F64};
  switch (op) {
  case OP_ADD:
    res.f64 = left.f64 + right.u64;
    break;
  case OP_SUB:
    res.f64 = left.f64 - right.u64;
    break;
  case OP_MUL:
    res.f64 = left.f64 * right.u64;
    break;
  case OP_DIV:
    res.f64 = left.f64 / right.u64;
    break;
  case OP_ANDS:
    res.f64 = left.f64 && right.u64;
    break;
  case OP_ORS:
    res.f64 = left.f64 || right.u64;
    break;
  case OP_EQ:
    res.f64 = left.f64 == right.u64;
    break;
  case OP_NEQ:
    res.f64 = left.f64 != right.u64;
    break;
  case OP_LT:
    res.f64 = left.f64 < right.u64;
    break;
  case OP_LE:
    res.f64 = left.f64 <= right.u64;
    break;
  case OP_GT:
    res.f64 = left.f64 > right.u64;
    break;
  case OP_GE:
    res.f64 = left.f64 >= right.i64;
    break;
  default:
    make_error(res, "unsupported or invalid operation between f64 and u64");
  }
  return res;
}

struct value handle_f64_i64(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_F64};
  switch (op) {
  case OP_ADD:
    res.f64 = left.f64 + right.i64;
    break;
  case OP_SUB:
    res.f64 = left.f64 - right.i64;
    break;
  case OP_MUL:
    res.f64 = left.f64 * right.i64;
    break;
  case OP_DIV:
    res.f64 = left.f64 / right.i64;
    break;
  case OP_ANDS:
    res.f64 = left.f64 && right.i64;
    break;
  case OP_ORS:
    res.f64 = left.f64 || right.i64;
    break;
  case OP_EQ:
    res.f64 = left.f64 == right.i64;
    break;
  case OP_NEQ:
    res.f64 = left.f64 != right.i64;
    break;
  case OP_LT:
    res.f64 = left.f64 < right.i64;
    break;
  case OP_LE:
    res.f64 = left.f64 <= right.i64;
    break;
  case OP_GT:
    res.f64 = left.f64 > right.i64;
    break;
  case OP_GE:
    res.f64 = left.f64 >= right.i64;
    break;
  default:
    make_error(res, "unsupported or invalid operation between f64 and i64");
  }
  return res;
}

struct value handle_f64_f64(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_F64};
  switch (op) {
  case OP_ADD:
    res.f64 = left.f64 + right.f64;
    break;
  case OP_SUB:
    res.f64 = left.f64 - right.f64;
    break;
  case OP_MUL:
    res.f64 = left.f64 * right.f64;
    break;
  case OP_DIV:
    res.f64 = left.f64 / right.f64;
    break;
  case OP_ANDS:
    res.f64 = left.f64 && right.f64;
    break;
  case OP_ORS:
    res.f64 = left.f64 || right.f64;
    break;
  case OP_EQ:
    res.f64 = left.f64 == right.f64;
    break;
  case OP_NEQ:
    res.f64 = left.f64 != right.f64;
    break;
  case OP_LT:
    res.f64 = left.f64 < right.f64;
    break;
  case OP_LE:
    res.f64 = left.f64 <= right.f64;
    break;
  case OP_GT:
    res.f64 = left.f64 > right.f64;
    break;
  case OP_GE:
    res.f64 = left.f64 >= right.f64;
    break;
  default:
    make_error(res, "unsupported or invalid operation between f64 and f64");
  }
  return res;
}

struct value handle_f64_str(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_STRING};
  if (op != OP_ADD) {
    make_error(res, "unsupported or invalid operation between f64 and str");
    return res;
  }

  char buffer[200];
  snprintf(buffer, 64L, "%lf", left.f64);
  size_t new_size = strlen(buffer) + strlen(right.str);
  char *new_str = malloc(new_size + 1);
  if (new_str == NULL) {
    free(new_str);
    make_error(res, "no more memory");
    return res;
  }

  memset(new_str, 0L, new_size + 1);
  sprintf(new_str, "%s%s", buffer, right.str);

  res.str = new_str;
  return res;
}

// str
struct value handle_str_u64(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_STRING};
  if (op != OP_ADD) {
    make_error(res, "unsupported or invalid operation between str and u64");
    return res;
  }

  char buffer[200];
  snprintf(buffer, 64L, "%lu", left.u64);
  size_t new_size = strlen(buffer) + strlen(left.str);
  char *new_str = malloc(new_size + 1);
  if (new_str == NULL) {
    free(new_str);
    make_error(res, "no more memory");
    return res;
  }

  memset(new_str, 0L, new_size + 1);
  sprintf(new_str, "%s%s", left.str, buffer);

  res.str = new_str;
  return res;
}

struct value handle_str_i64(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_STRING};
  if (op != OP_ADD) {
    make_error(res, "unsupported or invalid operation between str and i64");
    return res;
  }

  char buffer[200];
  snprintf(buffer, 64L, "%ld", left.i64);
  size_t new_size = strlen(buffer) + strlen(left.str);
  char *new_str = malloc(new_size + 1);
  if (new_str == NULL) {
    free(new_str);
    make_error(res, "no more memory");
    return res;
  }

  memset(new_str, 0L, new_size + 1);
  sprintf(new_str, "%s%s", left.str, buffer);

  res.str = new_str;
  return res;
}

struct value handle_str_f64(struct value left, struct value right,
                            enum bin_op op) {
  struct value res = {.type = TYPE_STRING};
  if (op != OP_ADD) {
    make_error(res, "unsupported or invalid operation between str and f64");
    return res;
  }

  char buffer[200];
  snprintf(buffer, 64L, "%lf", left.f64);
  size_t new_size = strlen(buffer) + strlen(left.str);
  char *new_str = malloc(new_size + 1);
  if (new_str == NULL) {
    free(new_str);
    make_error(res, "no more memory");
    return res;
  }

  memset(new_str, 0L, new_size + 1);
  sprintf(new_str, "%s%s", left.str, buffer);

  res.str = new_str;
  return res;
}

struct value handle_str_str(struct value left, struct value right,
                            enum bin_op op) {
  struct value res;
  res.type = TYPE_STRING;
  size_t left_size = strlen(left.str);
  size_t right_size = strlen(right.str);
  size_t new_size = left_size + right_size + 1;
  res.str = malloc(new_size);
  memset(res.str, 0L, new_size);
  sprintf(res.str, "%s%s", left.str, right.str);
  return res;
}

struct value handle_bin_op(struct value left, struct value right,
                           enum bin_op op) {
  struct value res;
  int ltype = left.type;
  int rtype = right.type;
  if (ltype == TYPE_U64 && rtype == TYPE_U64) {
    res = handle_u64_u64(left, right, op);
  } else if (ltype == TYPE_U64 && rtype == TYPE_I64) {
    res = handle_u64_i64(left, right, op);
  } else if (ltype == TYPE_U64 && rtype == TYPE_F64) {
    res = handle_u64_f64(left, right, op);
  } else if (ltype == TYPE_U64 && rtype == TYPE_STRING) {
    res = handle_u64_str(left, right, op);
  } else if (ltype == TYPE_I64 && rtype == TYPE_U64) {
    res = handle_i64_u64(left, right, op);
  } else if (ltype == TYPE_I64 && rtype == TYPE_I64) {
    res = handle_i64_i64(left, right, op);
  } else if (ltype == TYPE_I64 && rtype == TYPE_F64) {
    res = handle_i64_f64(left, right, op);
  } else if (ltype == TYPE_I64 && rtype == TYPE_STRING) {
    res = handle_i64_str(left, right, op);
  } else if (ltype == TYPE_F64 && rtype == TYPE_U64) {
    res = handle_f64_u64(left, right, op);
  } else if (ltype == TYPE_F64 && rtype == TYPE_I64) {
    res = handle_f64_i64(left, right, op);
  } else if (ltype == TYPE_F64 && rtype == TYPE_F64) {
    res = handle_f64_f64(left, right, op);
  } else if (ltype == TYPE_F64 && rtype == TYPE_STRING) {
    res = handle_f64_str(left, right, op);
  } else if (ltype == TYPE_STRING && rtype == TYPE_U64) {
    res = handle_str_u64(left, right, op);
  } else if (ltype == TYPE_STRING && rtype == TYPE_I64) {
    res = handle_str_i64(left, right, op);
  } else if (ltype == TYPE_STRING && rtype == TYPE_F64) {
    res = handle_str_f64(left, right, op);
  } else if (ltype == TYPE_STRING && rtype == TYPE_STRING) {
    res = handle_str_str(left, right, op);
  } else {
    make_errorf(res, "undefined binary operation between types: %d and %d\n",
                left.type, right.type);
  }

  // free_value(&left);
  // free_value(&right);
  return res;
}
