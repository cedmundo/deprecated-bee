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

struct bee_highlight {
  const char *sequence;
  struct bee_highlight *next;
};

enum bee_token_type {
  BEE_TT_EOF,
  BEE_TT_TEXT,
  BEE_TT_ID,
  BEE_TT_NUMBER,
  BEE_TT_STRING,
  BEE_TT_INDENT,
  BEE_TT_DEDENT,
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
  int32_t *level_stack;
  char *loc;
  size_t len;
  size_t col;
  size_t row;
  int32_t indent;
  int32_t level_stack_top;
  int32_t level_stack_max;
  enum bee_token_type type;
  enum bee_num_base base;
  bool is_floating;
};

struct bee_highlight *bee_highlight_new() {
  struct bee_highlight *highlight = malloc(sizeof(struct bee_highlight));
  highlight->sequence = NULL;
  highlight->next = NULL;
  return highlight;
}

void bee_highlight_push(struct bee_highlight *root, const char *sequence) {
  struct bee_highlight *cur = root;
  struct bee_highlight *prev = NULL;

  while (cur != NULL) {
    if (cur->sequence == NULL) {
      prev = cur;
      cur = cur->next;
      continue;
    }

    if (strstr(sequence, cur->sequence) == sequence &&
        strlen(sequence) > strlen(cur->sequence)) {
      break;
    }

    prev = cur;
    cur = cur->next;
  }

  struct bee_highlight *ins = bee_highlight_new();
  ins->sequence = sequence;
  ins->next = cur;
  if (prev != NULL) {
    prev->next = ins;
  }
}

struct bee_highlight *bee_builtin_operators() {
  const char *defaults[] = {
      "(", ")", "[",  "]",  "{", "}",  ":",  "=",  "=>", "->",
      "+", "-", "*",  "/",  "%", "&",  "&&", "|",  "||", "^",
      "~", "!", "==", "!=", ">", ">=", "<",  "<=", ",",  NULL};
  struct bee_highlight root = {.next = NULL, .sequence = NULL};

  for (int32_t i = 0; defaults[i] != NULL; i++) {
    bee_highlight_push(&root, defaults[i]);
  }

  return root.next;
}

struct bee_highlight *bee_builtin_keywords() {
  const char *defaults[] = {"match",  "with",   "unmatched", "handle",
                            "module", "import", NULL};
  struct bee_highlight root = {.next = NULL, .sequence = NULL};

  for (int32_t i = 0; defaults[i] != NULL; i++) {
    bee_highlight_push(&root, defaults[i]);
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

struct bee_token bee_starting_token(char *program, int32_t *indent_levels) {
  return (struct bee_token){
      .type = BEE_TT_EOF,
      .operators = bee_builtin_operators(),
      .keywords = bee_builtin_keywords(),
      .level_stack = indent_levels,
      .level_stack_max = 32,
      .level_stack_top = -1,
      .loc = program,
      .len = 0L,
      .row = 0L,
      .col = 0L,
      .indent = 0,
  };
}

bool isdigitonbase(char c, enum bee_num_base base) {
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

struct bee_token bee_next_token(struct bee_token prev) {
  struct bee_token token = {
      .operators = prev.operators,
      .keywords = prev.keywords,
      .loc = prev.loc + prev.len,
      .len = 0L,
      .col = 0L,
      .row = 0L,
      .indent = prev.indent,
      .level_stack = prev.level_stack,
      .level_stack_max = prev.level_stack_max,
      .level_stack_top = prev.level_stack_top,
      .type = BEE_TT_EOF,
  };

  char *cur = token.loc;
  bool start_endl = *cur == '\n';

  // Return if EOF
  if (IS_EOF(*cur)) {
    // TODO: Pop all dedents
    return token;
  }

  // If required, emit an INDENT or a DEDENT instead
  if (start_endl) {
    char *tmp = cur + 1;
    bool skip_line = true;
    int32_t line_indent = 0L;

    while (skip_line) {
      line_indent = 0L;
      while (NOT_EOF(*tmp) && *tmp == ' ') {
        line_indent++;
        tmp++;
      }

      skip_line = *tmp == '\n';
      if (skip_line) {
        tmp++;
      }
    }

    if (line_indent > prev.indent) {
      // Push into level stack (to pop dedents after)
      token.type = BEE_TT_INDENT;
      token.level_stack[++token.level_stack_top] = line_indent;
      token.indent = line_indent;
      token.len = 0L;
      token.row = prev.row;
      token.col = prev.col;
      return token;
    } else if (line_indent < prev.indent) {
      // Pop a level from stack
      token.indent = token.level_stack[token.level_stack_top];
      if (line_indent < token.indent && token.level_stack_top > -1) {
        token.type = BEE_TT_DEDENT;
        token.level_stack_top--;
        token.len = 0L;
        token.row = prev.row;
        token.col = prev.col;
        return token;
      }
    }

    token.indent = line_indent;
    // TODO:
    //  - Emit EOL when '\n' or ';' is reached and not skipped
    //  - Don't emit EOL when token between '(' and ')' or '[' and ']'
    // FIXME:
    //  - When using INDENT or DEDENT it appears that current row is screwed
  }

  // Consume heading space characters (tabs, spaces and newlines)
  size_t heading_spaces = 0L;
  while (NOT_EOF(*cur) && isspace(*cur)) {
    heading_spaces++;
    cur++;
  }

  size_t text_len = 0L;
  bool is_floating = false;
  enum bee_token_type type = BEE_TT_TEXT;
  enum bee_num_base base = BEE_NUM_BASE_NONUM;

  // Try to read as a number
  // FIXME: Make this a function
  // TODO: (Prefix) Consume prefix operand if no spaces (+0,-0)
  // TODO: Should throw error if not operator/keyword between text?
  {
    // Number: Prefix
    if (strncmp("0x", cur, 2) == 0) {
      base = BEE_NUM_BASE_HEX;
      heading_spaces += 2;
      cur += 2;
    } else if (strncmp("0o", cur, 2) == 0) {
      base = BEE_NUM_BASE_OCT;
      heading_spaces += 2;
      cur += 2;
    } else if (strncmp("0b", cur, 2) == 0) {
      base = BEE_NUM_BASE_BIN;
      heading_spaces += 2;
      cur += 2;
    } else if (isdigit(*cur)) {
      base = BEE_NUM_BASE_DEC;
    }

    // Number: Integer part
    if (base != BEE_NUM_BASE_NONUM) {
      type = BEE_TT_NUMBER;

      while (NOT_EOF(*cur) && isdigitonbase(*cur, base)) {
        cur++;
        text_len++;
      }
    }

    // Number: Decimal part
    if (base == BEE_NUM_BASE_NONUM || base == BEE_NUM_BASE_DEC) {
      if (NOT_EOF(*cur) && *cur == '.' && isdigit(*(cur + 1))) {
        type = BEE_TT_NUMBER;
        base = BEE_NUM_BASE_DEC;
        is_floating = true;
        cur++;
        text_len++;

        while (NOT_EOF(*cur) && isdigitonbase(*cur, base)) {
          cur++;
          text_len++;
        }
      }
    }

    // Number: Suffix
    if (base != BEE_NUM_BASE_NONUM) {
      if (NOT_EOF(*cur) && *cur == 'e') {
        char nxt = *(cur + 1);
        if (NOT_EOF(nxt) && (nxt == '-' || nxt == '+' || isdigit(nxt))) {
          cur += 2;
          text_len += 2;

          while (NOT_EOF(*cur) && isdigitonbase(*cur, BEE_NUM_BASE_DEC)) {
            cur++;
            text_len++;
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
          text_len += suffix_len;
          break;
        }
      }
    }
  }

  // Try read as a string
  // FIXME: Make this a function
  // TODO: Handle bad terminated strings and escape sequences
  {
    if (type == BEE_TT_TEXT && *cur == '"') {
      type = BEE_TT_STRING;
      text_len++;
      cur++;

      while (NOT_EOF(*cur) && *cur != '"' && *cur != '\n') {
        text_len++;
        cur++;
      }

      if (*cur == '\"') {
        text_len++;
        cur++;
      } else {
        // TODO: Error: not a well terminated string
      }
    }
  }

  // Try read as an operator
  if (type == BEE_TT_TEXT) {
    struct bee_highlight *cur_op = token.operators;
    while (cur_op != NULL) {
      size_t seql = strlen(cur_op->sequence);
      if (memcmp(cur_op->sequence, cur, seql) == 0) {
        type = BEE_TT_OPERATOR;
        text_len += seql;
        cur += seql;
        break;
      }

      cur_op = cur_op->next;
    }
  }

  // Error: this operator is not recognized
  if (type == BEE_TT_TEXT && ispunct(*cur)) {
    while (NOT_EOF(*cur) && !isspace(*cur)) {
      text_len++;
      cur++;
    }
  } else if (type == BEE_TT_TEXT) {
    // Try read as an id or keyword
    while (NOT_EOF(*cur) && !isspace(*cur) &&
           (!ispunct(*cur) || *cur == '_' || *cur == '$')) {
      text_len++;
      cur++;
    }

    type = BEE_TT_ID;

    // Check if text is defined as keyword
    struct bee_highlight *cur_kw = token.keywords;
    while (cur_kw != NULL) {
      size_t kwl = strlen(cur_kw->sequence);
      if (memcmp(cur_kw->sequence, token.loc + heading_spaces, kwl) == 0 &&
          kwl == text_len) {
        type = BEE_TT_KEYWORD;
      }

      cur_kw = cur_kw->next;
    }
  }

  // Build the token
  token.type = type;
  token.len = text_len;
  token.loc = token.loc + heading_spaces;
  token.row = prev.row;
  token.col = prev.col + prev.len + heading_spaces;
  token.is_floating = is_floating;
  token.base = base;
  return token;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: %s <bee source code>\n", argv[0]);
    return 1;
  }

  char *program = argv[1];
  int32_t indent_levels[32];
  struct bee_token token = bee_starting_token(program, indent_levels);

  const char *token_types[] = {"EOF",  "TEXT", "ID",   "NUM", "STR",
                               "INDT", "DNDT", "OPER", "KWRD"};
  do {
    token = bee_next_token(token);
    if (token.type != BEE_TT_EOF) {
      if (token.len > 0) {
        char buf[255];
        memset(buf, 0L, 255);
        memcpy(buf, token.loc, token.len);

        printf("%ld:%ld:%ld\t| %s\t %s\n", token.row + 1, token.col + 1,
               token.len, token_types[token.type], buf);
      } else {
        printf("%ld:%ld:%ld\t| %s\t\n", token.row + 1, token.col + 1, token.len,
               token_types[token.type]);
      }
    }
  } while (token.type != BEE_TT_EOF);

  bee_highlights_free(token.operators);
  bee_highlights_free(token.keywords);
  return 0;
}
