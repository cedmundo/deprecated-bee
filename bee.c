#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define IS_EOF(x) (x == '\0')
#define NOT_EOF(x) (x != '\0')

enum bee_highlight_notation {
  BEE_HN_INFIX,
  BEE_HN_PREFIX,
  BEE_HN_OPEN,
  BEE_HN_CLOSE,
};

enum bee_highlight_flag {
  BEE_HF_NO_FLAGS = 0,
  BEE_HF_BRACKET = 1,
  BEE_HF_SKIP_EOL = 2,
  BEE_HF_BALANCE_PAR = 4,
  BEE_HF_UNBLANCE_PAR = 8,
  BEE_HF_BALANCE_SBR = 16,
  BEE_HF_UNBLANCE_SBR = 32,
  BEE_HF_BALANCE_CBR = 64,
  BEE_HF_UNBLANCE_CBR = 128,
  BEE_HF_WRAP_EXPR = 256,
  BEE_HF_WRAP_BLOCK = 256,
};

struct bee_highlight {
  const char *sequence;
  struct bee_highlight *next;
  int32_t flags;
  enum bee_highlight_notation notation;
};

enum bee_token_type {
  BEE_TT_EOF,
  BEE_TT_EOL,
  BEE_TT_TEXT,
  BEE_TT_ID,
  BEE_TT_NUMBER,
  BEE_TT_STRING,
  BEE_TT_OPERATOR,
  BEE_TT_KEYWORD,
};

enum bee_num_base {
  BEE_NUM_BASE_NONUM = 0,
  BEE_NUM_BASE_BIN = 2,
  BEE_NUM_BASE_OCT = 8,
  BEE_NUM_BASE_DEC = 10,
  BEE_NUM_BASE_HEX = 16,
};

struct bee_token {
  struct bee_highlight *operators;
  struct bee_highlight *keywords;
  struct bee_highlight *sel_highlight;
  char *loc;
  size_t len;
  size_t col;
  size_t row;
  int32_t par_balance;
  int32_t sbr_balance;
  int32_t cbr_balance;
  enum bee_token_type type;
  enum bee_num_base num_base;
  bool is_decimal;
  bool skip_next_newline;
};

enum bee_ast_node_type {
  BEE_ANT_ERROR,
  BEE_ANT_APPLY,
  BEE_ANT_INFIX,
  BEE_ANT_PREFIX,
  BEE_ANT_IDENTITY,
  BEE_ANT_LITERAL,
  BEE_ANT_BLOCK,
};

struct bee_ast_node {
  struct bee_ast_node *left;
  struct bee_ast_node *right;
  struct bee_token token;
  enum bee_ast_node_type type;
};

struct bee_highlight *bee_highlight_new() {
  struct bee_highlight *highlight = malloc(sizeof(struct bee_highlight));
  highlight->sequence = NULL;
  highlight->next = NULL;
  return highlight;
}

void bee_highlight_push(struct bee_highlight *root,
                        struct bee_highlight highlight) {
  assert(root != NULL);
  struct bee_highlight *cur = root;
  struct bee_highlight *prev = NULL;

  while (cur != NULL) {
    if (cur->sequence == NULL) {
      prev = cur;
      cur = cur->next;
      continue;
    }

    if (strstr(highlight.sequence, cur->sequence) == highlight.sequence &&
        strlen(highlight.sequence) > strlen(cur->sequence)) {
      break;
    }

    prev = cur;
    cur = cur->next;
  }

  struct bee_highlight *ins = bee_highlight_new();
  memcpy(ins, &highlight, sizeof(struct bee_highlight));
  if (prev != NULL) {
    ins->next = prev->next;
    prev->next = ins;
  }
}

struct bee_highlight *bee_builtin_operators() {
  const struct bee_highlight operators[] = {
      (struct bee_highlight){.notation = BEE_HN_OPEN,
                             .sequence = "(",
                             .flags = BEE_HF_BRACKET | BEE_HF_UNBLANCE_PAR |
                                      BEE_HF_WRAP_EXPR},
      (struct bee_highlight){.notation = BEE_HN_CLOSE,
                             .sequence = ")",
                             .flags = BEE_HF_BRACKET | BEE_HF_BALANCE_PAR},
      (struct bee_highlight){.notation = BEE_HN_OPEN,
                             .sequence = "[",
                             .flags = BEE_HF_BRACKET | BEE_HF_UNBLANCE_SBR |
                                      BEE_HF_WRAP_EXPR},
      (struct bee_highlight){.notation = BEE_HN_CLOSE,
                             .sequence = "]",
                             .flags = BEE_HF_BRACKET | BEE_HF_BALANCE_SBR},
      (struct bee_highlight){.notation = BEE_HN_OPEN,
                             .sequence = "{",
                             .flags = BEE_HF_BRACKET | BEE_HF_UNBLANCE_CBR |
                                      BEE_HF_SKIP_EOL | BEE_HF_WRAP_BLOCK},
      (struct bee_highlight){.notation = BEE_HN_CLOSE,
                             .sequence = "}",
                             .flags = BEE_HF_BRACKET | BEE_HF_BALANCE_CBR},
      (struct bee_highlight){
          .notation = BEE_HN_INFIX, .sequence = ":", .flags = BEE_HF_SKIP_EOL},
      (struct bee_highlight){
          .notation = BEE_HN_INFIX, .sequence = "=", .flags = BEE_HF_SKIP_EOL},
      (struct bee_highlight){.sequence = NULL},
  };
  struct bee_highlight root = {.next = NULL, .sequence = NULL};
  for (int32_t i = 0; operators[i].sequence != NULL; i++) {
    bee_highlight_push(&root, operators[i]);
  }

  return root.next;
}

struct bee_highlight *bee_builtin_keywords() {
  const struct bee_highlight operators[] = {
      (struct bee_highlight){.sequence = NULL},
  };
  struct bee_highlight root = {.next = NULL, .sequence = NULL};
  for (int32_t i = 0; operators[i].sequence != NULL; i++) {
    bee_highlight_push(&root, operators[i]);
  }

  return root.next;
}

void bee_highlights_free(struct bee_highlight *op) {
  struct bee_highlight *tmp = NULL;
  while (op != NULL) {
    tmp = op->next;
    free(op);
    op = tmp;
  }
}

struct bee_token bee_starting_token(char *program) {
  return (struct bee_token){
      .type = BEE_TT_EOF,
      .operators = bee_builtin_operators(),
      .keywords = bee_builtin_keywords(),
      .sel_highlight = NULL,
      .loc = program,
      .len = 0L,
      .row = 0L,
      .col = 0L,
      .sbr_balance = 0,
      .cbr_balance = 0,
      .par_balance = 0,
      .skip_next_newline = false,
  };
}

bool bee_should_emit_eol(struct bee_token token) {
  return token.sbr_balance == 0 && token.par_balance == 0;
}

bool bee_handle_bracket(struct bee_token *token,
                        const struct bee_highlight *highlight) {
  assert(token != NULL);
  if ((highlight->flags & BEE_HF_BALANCE_PAR) && token->par_balance > 0) {
    token->par_balance--;
    return true;
  } else if (highlight->flags & BEE_HF_UNBLANCE_PAR) {
    token->par_balance++;
    return true;
  } else if ((highlight->flags & BEE_HF_BALANCE_SBR) &&
             token->par_balance > 0) {
    token->sbr_balance--;
    return true;
  } else if ((highlight->flags & BEE_HF_BALANCE_CBR) &&
             token->par_balance > 0) {
    token->cbr_balance--;
    return true;
  } else if (highlight->flags & BEE_HF_UNBLANCE_SBR) {
    token->sbr_balance++;
    return true;
  } else if (highlight->flags & BEE_HF_UNBLANCE_CBR) {
    token->cbr_balance++;
    return true;
  }

  return highlight->flags & BEE_HF_BRACKET;
}

bool bee_is_digit_on_base(char c, enum bee_num_base base) {
  switch (base) {
  case BEE_NUM_BASE_BIN:
    return c == '0' || c == '1';
  case BEE_NUM_BASE_OCT:
    return isdigit(c) && (c <= '7' || c == '0');
  case BEE_NUM_BASE_DEC:
    return isdigit(c);
  case BEE_NUM_BASE_HEX:
    return isdigit(c) || (c >= 'A' && c <= 'F');
  case BEE_NUM_BASE_NONUM:
    break;
  }

  return false;
}

// TODO(cedmundo): handle scape sequences and formatting
bool bee_try_read_as_string(char *cur, struct bee_token *token) {
  if (*cur == '"') {
    token->type = BEE_TT_STRING;
    token->len++;
    cur++;

    while (NOT_EOF(*cur) && *cur != '"' && *cur != '\n') {
      token->len++;
      cur++;
    }

    if (*cur == '\"') {
      token->len++;
      cur++;
    } else {
      // TODO(cedmundo): Error: not a well terminated string
    }
  }

  return token->type == BEE_TT_STRING;
}

bool bee_try_read_as_number(char *cur, struct bee_token *token) {
  // Number: Prefix
  if (strlen(cur) > 2 && strncmp("0x", cur, 2) == 0) {
    token->num_base = BEE_NUM_BASE_HEX;
    token->loc += 2;
    cur += 2;
  } else if (strlen(cur) > 2 && strncmp("0o", cur, 2) == 0) {
    token->num_base = BEE_NUM_BASE_OCT;
    token->loc += 2;
    cur += 2;
  } else if (strlen(cur) > 2 && strncmp("0b", cur, 2) == 0) {
    token->num_base = BEE_NUM_BASE_BIN;
    token->loc += 2;
    cur += 2;
  } else if (isdigit(*cur)) {
    token->num_base = BEE_NUM_BASE_DEC;
  }

  // Number: Integer part
  if (token->num_base != BEE_NUM_BASE_NONUM) {
    token->type = BEE_TT_NUMBER;

    while (NOT_EOF(*cur) && bee_is_digit_on_base(*cur, token->num_base)) {
      cur++;
      token->len++;
    }
  }

  // Number: Decimal part
  if (token->num_base == BEE_NUM_BASE_NONUM ||
      token->num_base == BEE_NUM_BASE_DEC) {
    if (NOT_EOF(*cur) && *cur == '.' && isdigit(*(cur + 1))) {
      token->type = BEE_TT_NUMBER;
      token->num_base = BEE_NUM_BASE_DEC;
      token->is_decimal = true;
      cur++;
      token->len++;

      while (NOT_EOF(*cur) && bee_is_digit_on_base(*cur, token->num_base)) {
        cur++;
        token->len++;
      }
    }
  }

  // Number: Suffix
  if (token->num_base != BEE_NUM_BASE_NONUM) {
    if (NOT_EOF(*cur) && *cur == 'e') {
      char nxt = *(cur + 1);
      if (NOT_EOF(nxt) && (nxt == '-' || nxt == '+' || isdigit(nxt))) {
        cur += 2;
        token->len += 2;

        while (NOT_EOF(*cur) && bee_is_digit_on_base(*cur, BEE_NUM_BASE_DEC)) {
          cur++;
          token->len++;
        }
      }
    }

    static char *suffixes[] = {"u8",  "u16", "u32", "u64", "i8", "i16",
                               "i32", "i64", "f32", "f64", NULL};
    for (int i = 0; suffixes[i] != NULL; i++) {
      size_t suffix_len = strlen(suffixes[i]);
      if (IS_EOF(*cur) || (strlen(cur) < suffix_len)) {
        continue;
      }

      if (memcmp(cur, suffixes[i], suffix_len) == 0) {
        cur += suffix_len;
        token->len += suffix_len;
        break;
      }
    }
  }

  return token->type == BEE_TT_NUMBER;
}

bool bee_try_read_as_operator(char *cur, struct bee_token *token) {
  // Try read as an operator
  struct bee_highlight *cur_op = token->operators;
  while (cur_op != NULL) {
    size_t seql = strlen(cur_op->sequence);
    if (memcmp(cur_op->sequence, cur, seql) == 0) {
      cur += seql;
      token->type = BEE_TT_OPERATOR;
      token->len += seql;
      token->skip_next_newline = cur_op->flags & BEE_HF_SKIP_EOL;
      token->sel_highlight = cur_op;

      // TODO(cedmundo): error on balancing on already balanced operands.
      bee_handle_bracket(token, cur_op);
      break;
    }

    cur_op = cur_op->next;
  }

  return token->type == BEE_TT_OPERATOR;
}

bool bee_try_read_as_keyword_or_id(char *cur, struct bee_token *token) {
  // Try read as an id or keyword
  while (NOT_EOF(*cur) && !isspace(*cur) &&
         (!ispunct(*cur) || *cur == '_' || *cur == '$')) {
    token->len++;
    cur++;
  }

  if (token->len > 0) {
    token->type = BEE_TT_ID;
  }

  // Check if text is defined as keyword
  struct bee_highlight *cur_kw = token->keywords;
  while (cur_kw != NULL) {
    size_t kwl = strlen(cur_kw->sequence);
    if (memcmp(cur_kw->sequence, token->loc, kwl) == 0 && kwl == token->len) {
      token->type = BEE_TT_KEYWORD;
      token->sel_highlight = cur_kw;
    }

    cur_kw = cur_kw->next;
  }

  return token->type == BEE_TT_ID || token->type == BEE_TT_KEYWORD;
}

struct bee_token bee_next_token(struct bee_token prev) {
  struct bee_token token = {
      .operators = prev.operators,
      .keywords = prev.keywords,
      .loc = prev.loc + prev.len,
      .len = 0L,
      .col = prev.col,
      .row = prev.row,
      .par_balance = prev.par_balance,
      .sbr_balance = prev.sbr_balance,
      .cbr_balance = prev.cbr_balance,
      .skip_next_newline = false,
      .type = BEE_TT_EOF,
  };

  char *cur = token.loc;
  bool start_endl = *cur == '\n';

  // Return if EOF
  if (IS_EOF(*cur)) {
    return token;
  }

  // FIXME(cedmundo): wrong col and row values
  if (start_endl && !prev.skip_next_newline && bee_should_emit_eol(prev)) {
    token.type = BEE_TT_EOL;
    token.len = 1L;
    token.row = prev.row;
    token.col = prev.col;
    return token;
  }

  // Consume heading space characters (tabs, spaces and newlines)
  size_t heading_spaces = 0L;
  while (NOT_EOF(*cur) && isspace(*cur)) {
    heading_spaces++;
    cur++;
  }

  // Offset the token start and re-initialize length to zero
  token.loc = token.loc + heading_spaces;
  token.len = 0L;
  token.type = BEE_TT_TEXT;

  if (bee_try_read_as_number(cur, &token)) {
    return token;
  }

  if (bee_try_read_as_string(cur, &token)) {
    return token;
  }

  if (bee_try_read_as_operator(cur, &token)) {
    return token;
  }

  if (bee_try_read_as_keyword_or_id(cur, &token)) {
    return token;
  }

  // Read as text
  while (NOT_EOF(*cur) && !isspace(*cur)) {
    token.len++;
    cur++;
  }

  token.type = BEE_TT_TEXT;
  return token;
}

struct bee_ast_node bee_parse_expr(struct bee_ast_node prev);
struct bee_ast_node bee_parse_infix(struct bee_ast_node prev);
struct bee_ast_node bee_parse_prefix(struct bee_ast_node prev);
struct bee_ast_node bee_parse_apply(struct bee_ast_node prev);
struct bee_ast_node bee_parse_lookup(struct bee_ast_node prev);
struct bee_ast_node bee_parse_value(struct bee_ast_node prev);
struct bee_ast_node bee_parse_block(struct bee_ast_node prev);
struct bee_ast_node bee_parse_exprs(struct bee_ast_node prev);

bool bee_check_token_type(struct bee_token *token, enum bee_token_type type) {
  assert(token != NULL);
  return token->type == type;
}

bool bee_match_token_type(struct bee_token *token, enum bee_token_type type) {
  assert(token != NULL);
  if (bee_check_token_type(token, type)) {
    *token = bee_next_token(*token);
    return true;
  }

  return false;
}

bool bee_check_token_highlight(struct bee_token *token, bool match_notation,
                               enum bee_highlight_notation notation,
                               bool match_flags, int flags, char *seq) {
  assert(token != NULL);
  struct bee_highlight *highlight = token->sel_highlight;
  assert(highlight != NULL);

  bool matches_tk_type = bee_check_token_type(token, BEE_TT_KEYWORD) ||
                         bee_check_token_type(token, BEE_TT_OPERATOR);
  bool matches_hl_notation =
      !match_notation || (match_notation && highlight->notation == notation);
  bool matches_hl_flags =
      !match_flags || (match_flags && ((highlight->flags & flags) != 0));
  bool matches_hl_seq = true;
  if (seq != NULL) {
    size_t seql = strlen(seq);
    matches_hl_seq = memcmp(highlight->sequence, seq, seql) == 0;
  }

  return matches_tk_type && matches_hl_notation && matches_hl_flags &&
         matches_hl_seq;
}

bool bee_match_token_highlight(struct bee_token *token, bool match_notation,
                               enum bee_highlight_notation notation,
                               bool match_flags, int flags, char *seq) {
  assert(token != NULL);
  if (bee_check_token_highlight(token, match_notation, notation, match_flags,
                                flags, seq)) {
    *token = bee_next_token(*token);
    return true;
  }

  return false;
}

struct bee_ast_node bee_parse_expr(struct bee_ast_node prev) {
  return bee_parse_infix(prev);
}

struct bee_ast_node bee_parse_infix(struct bee_ast_node prev) {
  struct bee_ast_node node = bee_parse_prefix(prev);

  // FIXME: Correct this value
  return node;
}

struct bee_ast_node bee_parse_prefix(struct bee_ast_node prev) {
  struct bee_ast_node node = bee_parse_apply(prev);

  // FIXME: Correct this value
  return node;
}

struct bee_ast_node bee_parse_apply(struct bee_ast_node prev) {
  struct bee_ast_node node = bee_parse_lookup(prev);

  // FIXME: Correct this value
  return node;
}

struct bee_ast_node bee_parse_lookup(struct bee_ast_node prev) {
  struct bee_ast_node node = bee_parse_value(prev);

  // FIXME: Correct this value
  return node;
}

struct bee_ast_node bee_parse_value(struct bee_ast_node prev) {
  struct bee_ast_node node = bee_parse_block(prev);

  // FIXME: Correct this value
  return node;
}

struct bee_ast_node bee_parse_block(struct bee_ast_node prev) {
  return bee_parse_exprs(prev);
}

struct bee_ast_node bee_parse_exprs(struct bee_ast_node prev) {
  struct bee_ast_node node = {.type = BEE_ANT_ERROR, .token = prev.token};
  return node;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: %s <bee source code>\n", argv[0]);
    return 1;
  }

  char *program = argv[1];
  struct bee_token token = bee_starting_token(program);

  const char *token_types[] = {"EOF", "EOL", "TEXT", "ID",
                               "NUM", "STR", "OPER", "KWRD"};
  do {
    token = bee_next_token(token);
    if (token.type != BEE_TT_EOF) {
      if (token.len > 0 && token.type != BEE_TT_EOL) {
        char buf[255];
        memset(buf, 0L, 255);
        memcpy(buf, token.loc, token.len);

        printf("%ld:%ld:%ld\t %s\t| %s\n", token.row + 1, token.col + 1,
               token.len, token_types[token.type], buf);
      } else {
        printf("%ld:%ld:%ld\t %s\t|\n", token.row + 1, token.col + 1, token.len,
               token_types[token.type]);
      }
    }
  } while (token.type != BEE_TT_EOF);

  bee_highlights_free(token.operators);
  bee_highlights_free(token.keywords);
  return 0;
}
