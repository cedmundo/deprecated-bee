#include <ctype.h>
#include <jit/jit-common.h>
#include <jit/jit-context.h>
#include <jit/jit-insn.h>
#include <jit/jit-value.h>
#include <jit/jit.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum token_type {
  TT_EOF,
  TT_STRING,
  TT_NUMBER,
  TT_PUNCT,
  TT_INDENT,
  TT_KEYWORD,
};

enum notation {
  NOTATION_NONUM = 0,
  NOTATION_BIN = 2,
  NOTATION_OCT = 8,
  NOTATION_DEC = 10,
  NOTATION_HEX = 16,
};

struct token {
  char *pos;
  size_t col;
  size_t row;
  size_t len;
  enum notation num_notation;
  enum token_type type;
};

enum node_type {
  NT_ADD,
  NT_SUB,
  NT_MUL,
  NT_DIV,
  NT_REM,
  NT_NEG,
  NT_NOT,
  NT_AND,
  NT_SAND,
  NT_OR,
  NT_SOR,
  NT_XOR,
  NT_EQ,
  NT_NE,
  NT_LT,
  NT_LE,
  NT_GT,
  NT_GE,
  NT_I64,
  NT_APPLY,
};

struct node {
  enum node_type type;
  union {
    struct {
      struct node *left;
      struct node *right;
    };
    int64_t val_i64;
    char *val_str;
  };
};

struct bind {
  struct bind *next;
  char *name;
  jit_type_t type;
  jit_value_t value;
};

struct scope {
  jit_function_t func;
  jit_type_t r_type;
  jit_value_t r_value;

  struct bind *binds;
  struct scope *parent;
  struct scope *sibling;
  struct scope *children;
};

void error(struct token token, char *msg, ...) {
  fprintf(stderr, "at %ld:%ld: ", token.row, token.col);
  va_list ap;
  va_start(ap, msg);
  vfprintf(stderr, msg, ap);
  va_end(ap);
  fprintf(stderr, "\n");
  exit(1);
}

// === Lexer ===

bool isdigit_with_notation(char v, enum notation n) {
  switch (n) {
  case NOTATION_BIN:
    return v == '0' || v == '1';
  case NOTATION_OCT:
    return isdigit(v) && v != '8' && v != '9';
  case NOTATION_DEC:
    return isdigit(v);
  case NOTATION_HEX:
    return isdigit(v) || (v >= 'A' && v <= 'F');
  case NOTATION_NONUM:
    break;
  }

  return false;
}

bool iskeyword(char *v, size_t len) {
  const char *keywords[] = {
      "module", "def",   "let",       "in",   "where", "with", "lambda", "type",
      "of",     "match", "otherwise", "true", "false", "nil",  "unit",   NULL};

  for (int i = 0; keywords[i] != NULL; i++) {
    const char *keyword = keywords[i];
    size_t kwl = strlen(keyword);
    if (memcmp(keyword, v, kwl) == 0 && len == kwl) {
      return true;
    }
  }

  return false;
}

size_t read_operator(char *v) {
  const char *operators[] = {
      "!=", "==", ">=", "<=", "&&", "||", "(", ")", "+", "-",
      "*",  "%",  "/",  "|",  "&",  ">",  "<", "~", "^", NULL,
  };
  for (int i = 0; operators[i] != NULL; i++) {
    const char *operator= operators[i];
    size_t opl = strlen(operator);
    if (memcmp(operator, v, opl) == 0) {
      return opl;
    }
  }

  return 0;
}

struct token next_token(struct token prev) {
  struct token token = {
      .row = prev.row,
      .col = prev.col + prev.len,
      .pos = prev.pos + prev.len,
      .len = 0,
  };
  char *cur = token.pos;
  if (*cur == '\0') {
    return token;
  }

  while (isspace(*cur)) {
    cur++;
    token.pos++;
    token.col++;

    if (*cur == '\n') {
      token.row++;
      token.col = 0;
    }
  }

  if (*cur == '\0') {
    return token;
  }

  enum notation notation = NOTATION_NONUM;
  if (strncmp("0x", cur, 2) == 0) {
    notation = NOTATION_HEX;
    token.pos += 2;
    cur += 2;
  } else if (strncmp("0o", cur, 2) == 0) {
    notation = NOTATION_OCT;
    token.pos += 2;
    cur += 2;
  } else if (strncmp("0b", cur, 2) == 0) {
    notation = NOTATION_BIN;
    token.pos += 2;
    cur += 2;
  } else if (isdigit(*cur)) {
    notation = NOTATION_DEC;
  }

  if (notation != NOTATION_NONUM) {
    while (isdigit_with_notation(*cur, notation)) {
      cur++;
      token.len++;
    }

    token.type = TT_NUMBER;
    token.num_notation = notation;
    return token;
  }

  if (*cur == '"') {
    do {
      cur++;
      token.len++;
      // TODO: Handle escape sequences
    } while (*cur != '"' && *cur != '\0');

    if (*cur != '"') {
      error(token, "string not closed");
      return token;
    } else {
      cur++;
      token.len++;
    }

    token.type = TT_STRING;
    return token;
  }

  size_t opl = read_operator(cur);
  if (opl > 0) {
    cur += opl;
    token.len += opl;
    token.type = TT_PUNCT;
    return token;
  }

  if (!isspace(*cur) && (!ispunct(*cur) || *cur == '_' || *cur == '$')) {
    while (!isspace(*cur) && (!ispunct(*cur) || *cur == '_' || *cur == '$') &&
           *cur != '\0') {
      cur++;
      token.len++;
    }

    token.type = iskeyword(token.pos, token.len) ? TT_KEYWORD : TT_INDENT;
    return token;
  }

  error(token, "unknown token %c", *token.pos);
  return token;
}

bool token_equals(struct token token, enum token_type type, const char *v) {
  if (token.type == type) {
    if (v != NULL) {
      size_t s = strlen(v);
      if (s != token.len || memcmp(v, token.pos, s) != 0) {
        return false;
      }
    }

    return true;
  }

  return false;
}

bool match(struct token *current, enum token_type type, const char *v) {
  struct token candidate = next_token(*current);
  if (token_equals(candidate, type, v)) {
    *current = candidate;
    return true;
  }

  return false;
}

// === Parser ===

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

struct node *new_l_node(enum node_type type, struct node *left) {
  struct node *node = new_node();
  node->type = type;
  node->left = left;
  return node;
}

struct node *parse_expr(struct token *token);
struct node *parse_rel(struct token *token);
struct node *parse_disj(struct token *token);
struct node *parse_conj(struct token *token);
struct node *parse_sum(struct token *token);
struct node *parse_term(struct token *token);
struct node *parse_factor(struct token *token);
struct node *parse_primary(struct token *token);
struct node *parse_literal(struct token *token);
struct node *parse_apply(struct token *token);

struct node *parse_expr(struct token *token) {
  return parse_rel(token);
}

// rel = disj ("==" disj | "!=" disj | ">=" disj | ">" disj | "<=" disj | "<"
// disj)*
struct node *parse_rel(struct token *token) {
  struct node *node = parse_disj(token);

  for (;;) {
    if (match(token, TT_PUNCT, "==")) {
      node = new_lr_node(NT_EQ, node, parse_disj(token));
      continue;
    }

    if (match(token, TT_PUNCT, "!=")) {
      node = new_lr_node(NT_NE, node, parse_disj(token));
      continue;
    }

    if (match(token, TT_PUNCT, ">=")) {
      node = new_lr_node(NT_GE, node, parse_disj(token));
      continue;
    }

    if (match(token, TT_PUNCT, "<=")) {
      node = new_lr_node(NT_LE, node, parse_disj(token));
      continue;
    }

    if (match(token, TT_PUNCT, ">")) {
      node = new_lr_node(NT_GT, node, parse_disj(token));
      continue;
    }

    if (match(token, TT_PUNCT, "<")) {
      node = new_lr_node(NT_LT, node, parse_disj(token));
      continue;
    }

    return node;
  }
}

// disj = conj ("|" conj | "||" conj)*
struct node *parse_disj(struct token *token) {
  struct node *node = parse_conj(token);

  for (;;) {
    if (match(token, TT_PUNCT, "||")) {
      node = new_lr_node(NT_SOR, node, parse_conj(token));
      continue;
    }

    if (match(token, TT_PUNCT, "|")) {
      node = new_lr_node(NT_OR, node, parse_conj(token));
      continue;
    }

    if (match(token, TT_PUNCT, "^")) {
      node = new_lr_node(NT_XOR, node, parse_sum(token));
      continue;
    }

    return node;
  }
}

// conj = sum ("&" sum | "&&" sum)*
struct node *parse_conj(struct token *token) {
  struct node *node = parse_sum(token);

  for (;;) {
    if (match(token, TT_PUNCT, "&&")) {
      node = new_lr_node(NT_SAND, node, parse_sum(token));
      continue;
    }

    if (match(token, TT_PUNCT, "&")) {
      node = new_lr_node(NT_AND, node, parse_sum(token));
      continue;
    }

    return node;
  }
}

// sum = term ("+" term | "-" term)*
struct node *parse_sum(struct token *token) {
  struct node *node = parse_term(token);

  for (;;) {
    if (match(token, TT_PUNCT, "+")) {
      node = new_lr_node(NT_ADD, node, parse_term(token));
      continue;
    }

    if (match(token, TT_PUNCT, "-")) {
      node = new_lr_node(NT_SUB, node, parse_term(token));
      continue;
    }

    return node;
  }
}

// term = factor ("*" factor | "/" factor | "%" factor)*
struct node *parse_term(struct token *token) {
  struct node *node = parse_factor(token);

  for (;;) {
    if (match(token, TT_PUNCT, "*")) {
      node = new_lr_node(NT_MUL, node, parse_factor(token));
      continue;
    }

    if (match(token, TT_PUNCT, "/")) {
      node = new_lr_node(NT_DIV, node, parse_factor(token));
      continue;
    }

    if (match(token, TT_PUNCT, "%")) {
      node = new_lr_node(NT_REM, node, parse_factor(token));
      continue;
    }

    return node;
  }
}

// factor = ( "+" | "-" | "~" ) factor
//       | primary
struct node *parse_factor(struct token *token) {
  if (match(token, TT_PUNCT, "+")) {
    return parse_factor(token);
  }

  if (match(token, TT_PUNCT, "-")) {
    return new_l_node(NT_NEG, parse_factor(token));
  }

  if (match(token, TT_PUNCT, "~")) {
    return new_l_node(NT_NOT, parse_factor(token));
  }

  return parse_primary(token);
}

// primary = "(" expr ")" | literal | apply
struct node *parse_primary(struct token *token) {
  struct node *node;

  if (match(token, TT_PUNCT, "(")) {
    node = parse_expr(token);
    if (!match(token, TT_PUNCT, ")")) {
      error(*token, "expected ')' found: %d", next_token(*token).type);
    }

    return node;
  }

  node = parse_literal(token);
  if (node != NULL) {
    return node;
  }

  node = parse_apply(token);
  if (node != NULL) {
    return node;
  }

  return node;
}

// literal = int literal
struct node *parse_literal(struct token *token) {
  if (match(token, TT_NUMBER, NULL)) {
    struct node *node = new_node();
    if (token->len < 1) {
      error(*token, "unexpected empty int literal");
      return node;
    }

    char *tmp = calloc(sizeof(char), token->len);
    memcpy(tmp, token->pos, token->len);
    node->type = NT_I64;
    node->val_i64 = strtol(tmp, NULL, token->num_notation);
    free(tmp);

    return node;
  }

  return NULL;
}

struct node *parse_apply(struct token *token) {
  if (match(token, TT_INDENT, NULL)) {
    struct node *node = new_node();
    node->val_str = calloc(sizeof(char), token->len + 1);
    node->type = NT_APPLY;
    memcpy(node->val_str, token->pos, token->len);
    return node;
  }

  return NULL;
}

// JIT & Running

jit_value_t build_bin_add(struct scope *scope, struct node *node);
jit_value_t build_bin_sub(struct scope *scope, struct node *node);
jit_value_t build_bin_mul(struct scope *scope, struct node *node);
jit_value_t build_bin_div(struct scope *scope, struct node *node);
jit_value_t build_bin_rem(struct scope *scope, struct node *node);
jit_value_t build_bin_and(struct scope *scope, struct node *node);
jit_value_t build_bin_sand(struct scope *scope, struct node *node);
jit_value_t build_bin_or(struct scope *scope, struct node *node);
jit_value_t build_bin_sor(struct scope *scope, struct node *node);
jit_value_t build_bin_xor(struct scope *scope, struct node *node);
jit_value_t build_bin_eq(struct scope *scope, struct node *node);
jit_value_t build_bin_ne(struct scope *scope, struct node *node);
jit_value_t build_bin_le(struct scope *scope, struct node *node);
jit_value_t build_bin_lt(struct scope *scope, struct node *node);
jit_value_t build_bin_ge(struct scope *scope, struct node *node);
jit_value_t build_bin_gt(struct scope *scope, struct node *node);
jit_value_t build_apply(struct scope *scope, struct node *node);
jit_value_t build_expr(struct scope *scope, struct node *node);
struct scope *new_scope(jit_function_t func);
struct scope *scope_fork(struct scope *parent);
struct bind *scope_push_bind(struct scope *scope, char *name, jit_type_t type,
                             jit_value_t value);
struct bind *scope_find_bind(struct scope *scope, char *name);

jit_value_t build_expr(struct scope *scope, struct node *node) {
  switch (node->type) {
  case NT_I64:
    return jit_value_create_nint_constant(scope->func, jit_type_int,
                                          node->val_i64);
  case NT_NEG:
    return jit_insn_neg(scope->func, build_expr(scope, node->left));
  case NT_NOT:
    return jit_insn_not(scope->func, build_expr(scope, node->left));
  case NT_ADD:
    return build_bin_add(scope, node);
  case NT_SUB:
    return build_bin_sub(scope, node);
  case NT_MUL:
    return build_bin_mul(scope, node);
  case NT_DIV:
    return build_bin_div(scope, node);
  case NT_REM:
    return build_bin_rem(scope, node);
  case NT_AND:
    return build_bin_and(scope, node);
  case NT_SAND:
    return build_bin_sand(scope, node);
  case NT_OR:
    return build_bin_or(scope, node);
  case NT_SOR:
    return build_bin_sor(scope, node);
  case NT_XOR:
    return build_bin_xor(scope, node);
  case NT_EQ:
    return build_bin_eq(scope, node);
  case NT_NE:
    return build_bin_ne(scope, node);
  case NT_LT:
    return build_bin_lt(scope, node);
  case NT_LE:
    return build_bin_le(scope, node);
  case NT_GT:
    return build_bin_gt(scope, node);
  case NT_GE:
    return build_bin_ge(scope, node);
  case NT_APPLY:
    return build_apply(scope, node);
  }

  // error(token, "could not build expression");
  return NULL;
}

jit_value_t build_bin_add(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right = build_expr(scope, node->right);
  return jit_insn_add(scope->func, left, right);
}

jit_value_t build_bin_sub(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right = build_expr(scope, node->right);
  return jit_insn_sub(scope->func, left, right);
}

jit_value_t build_bin_mul(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right = build_expr(scope, node->right);
  return jit_insn_mul(scope->func, left, right);
}

jit_value_t build_bin_div(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right = build_expr(scope, node->right);
  return jit_insn_div(scope->func, left, right);
}

jit_value_t build_bin_rem(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right = build_expr(scope, node->right);
  return jit_insn_rem(scope->func, left, right);
}

jit_value_t build_bin_and(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right = build_expr(scope, node->right);
  return jit_insn_and(scope->func, left, right);
}

jit_value_t build_bin_sand(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right =
      jit_value_create_nint_constant(scope->func, jit_type_int, 0);
  jit_label_t skip_right = jit_label_undefined;
  jit_insn_branch_if_not(scope->func, left, &skip_right);
  right = build_expr(scope, node->right);
  jit_insn_label(scope->func, &skip_right);
  return jit_insn_and(scope->func, left, right);
}

jit_value_t build_bin_or(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right = build_expr(scope, node->right);
  return jit_insn_or(scope->func, left, right);
}

jit_value_t build_bin_sor(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right =
      jit_value_create_nint_constant(scope->func, jit_type_int, 1);
  jit_label_t skip_right = jit_label_undefined;
  jit_insn_branch_if(scope->func, left, &skip_right);
  right = build_expr(scope, node->right);
  jit_insn_label(scope->func, &skip_right);
  return jit_insn_or(scope->func, left, right);
}

jit_value_t build_bin_xor(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right = build_expr(scope, node->right);
  return jit_insn_xor(scope->func, left, right);
}

jit_value_t build_bin_eq(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right = build_expr(scope, node->right);
  return jit_insn_eq(scope->func, left, right);
}

jit_value_t build_bin_ne(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right = build_expr(scope, node->right);
  return jit_insn_ne(scope->func, left, right);
}

jit_value_t build_bin_le(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right = build_expr(scope, node->right);
  return jit_insn_le(scope->func, left, right);
}

jit_value_t build_bin_lt(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right = build_expr(scope, node->right);
  return jit_insn_lt(scope->func, left, right);
}

jit_value_t build_bin_ge(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right = build_expr(scope, node->right);
  return jit_insn_ge(scope->func, left, right);
}

jit_value_t build_bin_gt(struct scope *scope, struct node *node) {
  jit_value_t left = build_expr(scope, node->left);
  jit_value_t right = build_expr(scope, node->right);
  return jit_insn_gt(scope->func, left, right);
}

jit_value_t build_apply(struct scope *scope, struct node *node) {
  struct bind *bind = scope_find_bind(scope, node->val_str);
  if (bind == NULL) {
    fprintf(stderr, "undefined: %s\n", node->val_str);
    exit(1);
  }

  return jit_insn_load(scope->func, bind->value);
}

struct scope *new_scope(jit_function_t func) {
  struct scope *scope = calloc(sizeof(struct scope), 1);
  scope->func = func;
  return scope;
}

struct scope *scope_fork(struct scope *parent) {
  struct scope *scope = new_scope(parent->func);
  scope->parent = parent;

  if (parent->children == NULL) {
    parent->children = scope;
  } else {
    struct scope *last = parent->children;
    while (last->sibling != NULL) {
      last = last->sibling;
    }

    last->sibling = scope;
  }

  return scope;
}

struct bind *scope_push_bind(struct scope *scope, char *name, jit_type_t type,
                             jit_value_t value) {
  jit_value_t dest = jit_value_create(scope->func, type);
  jit_insn_store(scope->func, dest, value);

  struct bind *bind = calloc(sizeof(struct bind), 1);
  bind->name = name;
  bind->type = type;
  bind->value = dest;

  if (scope->binds == NULL) {
    scope->binds = bind;
  } else {
    struct bind *last = scope->binds;
    while (last->next != NULL) {
      last = last->next;
    }

    last->next = bind;
  }

  return bind;
}

struct bind *scope_find_bind(struct scope *scope, char *name) {
  struct bind *cur = NULL;

  while (scope != NULL) {
    cur = scope->binds;
    while (cur != NULL) {
      if (strcmp(cur->name, name) == 0) {
        break;
      }

      cur = cur->next;
    }

    scope = scope->parent;
  }

  return cur;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: %s <bee source code>\n", argv[0]);
    return 1;
  }

  char *program = argv[1];
  struct token start = {
      .col = 0, .row = 1, .len = 0, .type = TT_EOF, .pos = program};

  struct node *node = parse_expr(&start);
  if (next_token(start).type != TT_EOF) {
    error(start, "remaining bytes after parsing");
  }

  jit_context_t context = jit_context_create();
  jit_context_build_start(context);

  jit_type_t main_signature =
      jit_type_create_signature(jit_abi_cdecl, jit_type_int, NULL, 0, 1);
  jit_function_t main = jit_function_create(context, main_signature);

  struct scope *main_scope = new_scope(main);
  jit_value_t main_id = jit_value_create_nint_constant(main, jit_type_int, 0x2);
  scope_push_bind(main_scope, "_bee_build_version$", jit_type_int, main_id);

  jit_insn_return(main, build_expr(main_scope, node));
  jit_function_compile(main);

  jit_int result;
  jit_function_apply(main, NULL, &result);
  printf("%d", result);

  jit_context_build_end(context);
  jit_context_destroy(context);
  return 0;
}
