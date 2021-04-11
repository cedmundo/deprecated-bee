#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define IS_EOF(x) (x == '\0')
#define NOT_EOF(x) (x != '\0')

enum bee_token_type {
  BTT_EOF,
  BTT_TEXT,
  BTT_INDENT,
  BTT_DEDENT,
};

struct bee_token {
  int16_t *level_stack;
  char *loc;
  size_t len;
  size_t col;
  size_t row;
  int16_t indent;
  int16_t level_stack_top;
  int16_t level_stack_max;
  enum bee_token_type type;
};

struct bee_token bee_starting_token(char *program, int16_t *indent_levels) {
  return (struct bee_token){
      .type = BTT_EOF,
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

struct bee_token bee_next_token(struct bee_token prev) {
  struct bee_token token = {
      .loc = prev.loc + prev.len,
      .len = 0L,
      .col = 0L,
      .row = 0L,
      .indent = prev.indent,
      .level_stack = prev.level_stack,
      .level_stack_max = prev.level_stack_max,
      .level_stack_top = prev.level_stack_top,
      .type = BTT_EOF,
  };

  char *cur = token.loc;
  bool start_endl = *cur == '\n';

  // Return if EOF
  if (IS_EOF(*cur)) {
    return token;
  }

  // If required, emit an INDENT or a DEDENT instead
  if (start_endl) {
    char *tmp = cur + 1;
    bool skip_line = true;
    int16_t line_indent = 0L;

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
      token.type = BTT_INDENT;

      // Push into level stack (to pop dedents after)
      token.level_stack[++token.level_stack_top] = line_indent;
      token.indent = line_indent;
      return token;
    } else if (line_indent < prev.indent) {
      token.indent = token.level_stack[token.level_stack_top];
      if (line_indent < token.indent && token.level_stack_top > -1) {
        token.type = BTT_DEDENT;
        token.level_stack_top--;
        return token;
      }
    }

    token.indent = line_indent;
  }

  // Consume heading space characters (tabs, spaces and newlines)
  size_t heading_spaces = 0L;
  while (NOT_EOF(*cur) && isspace(*cur)) {
    heading_spaces++;
    cur++;
  }

  // Consume text while is not space
  size_t text_len = 0L;
  while (NOT_EOF(*cur) && !isspace(*cur)) {
    text_len++;
    cur++;
  }

  // Build the token
  token.type = BTT_TEXT;
  token.len = text_len;
  token.loc = token.loc + heading_spaces;
  if (start_endl) {
    token.row = prev.row + 1;
    token.col = 0L;
  } else {
    token.row = prev.row;
    token.col = prev.col + prev.len + heading_spaces;
  }

  return token;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: %s <bee source code>\n", argv[0]);
    return 1;
  }

  char *program = argv[1];
  int16_t indent_levels[32];
  struct bee_token token = bee_starting_token(program, indent_levels);

  const char *token_types[] = {"EOF", "TEXT", "INDENT", "DEDENT"};
  do {
    token = bee_next_token(token);
    if (token.type != BTT_EOF) {
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
  } while (token.type != BTT_EOF);
  return 0;
}
