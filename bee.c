#include <ctype.h>
#include <jit/jit-context.h>
#include <jit/jit-insn.h>
#include <jit/jit-value.h>
#include <jit/jit.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum node_type {
  NT_ADD,
  NT_SUB,
  NT_I64,
};

struct node {
  enum node_type type;
  union {
    struct {
      struct node *left;
      struct node *right;
    };
    int64_t val_i64;
  };
};

void error(char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

bool match(char *expected, char *input, char **rest) {
  if (strstr(input, expected) != input) {
    return false;
  }

  *rest += strlen(expected);
  return true;
}

struct node *new_node() {
  return calloc(sizeof(struct node), 1);
}

struct node *new_lr_node(enum node_type type, struct node *left,
                         struct node *right) {
  struct node *node = new_node();
  node->type = type;
  node->left = left;
  node->right = right;
  return node;
}

struct node *parse_primary(char *input, char **rest);
struct node *parse_sum(char *input, char **rest);

// sum = primary ("+" primary | "-" primary)*
struct node *parse_sum(char *input, char **rest) {
  struct node *node = parse_primary(input, rest);

  for (;;) {
    if (match("+", *rest, rest)) {
      node = new_lr_node(NT_ADD, node, parse_primary(*rest, rest));
      continue;
    }

    if (match("-", *rest, rest)) {
      node = new_lr_node(NT_SUB, node, parse_primary(*rest, rest));
      continue;
    }

    return node;
  }
}

// primary = i64
struct node *parse_primary(char *input, char **rest) {
  struct node *node = new_node();
  node->type = NT_I64;
  node->val_i64 = strtol(input, rest, 10);
  return node;
}

jit_value_t build_bin_add(jit_function_t f, struct node *node);
jit_value_t build_bin_sub(jit_function_t f, struct node *node);
jit_value_t build_expr(jit_function_t f, struct node *node);

jit_value_t build_expr(jit_function_t f, struct node *node) {
  switch (node->type) {
  case NT_I64:
    return jit_value_create_nint_constant(f, jit_type_int, node->val_i64);
  case NT_ADD:
    return build_bin_add(f, node);
  case NT_SUB:
    return build_bin_sub(f, node);
  }

  error("could not build expression\n");
  return NULL;
}

jit_value_t build_bin_add(jit_function_t f, struct node *node) {
  jit_value_t left = build_expr(f, node->left);
  jit_value_t right = build_expr(f, node->right);
  return jit_insn_add(f, left, right);
}

jit_value_t build_bin_sub(jit_function_t f, struct node *node) {
  jit_value_t left = build_expr(f, node->left);
  jit_value_t right = build_expr(f, node->right);
  return jit_insn_sub(f, left, right);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: %s <bee source code>\n", argv[0]);
  }

  char *program = argv[1];
  struct node *node = parse_sum(program, &program);

  if (*program != '\0') {
    error("bytes remaining after end of program");
  }

  jit_context_t context = jit_context_create();
  jit_context_build_start(context);

  jit_type_t main_signature =
      jit_type_create_signature(jit_abi_cdecl, jit_type_int, NULL, 0, 1);
  jit_function_t main = jit_function_create(context, main_signature);
  jit_insn_return(main, build_expr(main, node));

  jit_function_compile(main);

  jit_int result;
  jit_function_apply(main, NULL, &result);
  printf("%d", result);

  jit_context_build_end(context);
  jit_context_destroy(context);
  return 0;
}
