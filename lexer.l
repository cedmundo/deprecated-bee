%{
#include <stdio.h>
#include "y.tab.h"
int fileno(FILE *stream);
extern char* strval;
%}

%%
" "         ;
"\n"        ;
"\t"        ;
"="         { return T_ASSIGN; }
"("         { return T_LPAR; }
")"         { return T_RPAR; }
"+"         { return T_ADD; }
"-"         { return T_SUB; }
"*"         { return T_MUL; }
"/"         { return T_DIV; }
","         { return T_COMMA; }
"."         { return T_DOT; }
"def"       { return T_DEF; }
"let"       { return T_LET; }
"for"       { return T_FOR; }
"in"        { return T_IN; }
"if"        { return T_IF; }
"else"      { return T_ELSE; }

[a-zA-Z_\$]+[a-zA-Z0-9_]* {
  strval = yytext;
  return T_ID;
}

\"(([^\"]|\\\")*[^\\])?\" {
  strval = yytext;
  return T_STRING;
}

\-?(0x|0b|0o)?[0-9ABCDEF]+(\.[0-9ABCDEF]+)?(u32|u64|i32|i64|f32|f64)? {
  strval = yytext;
  return T_NUMBER;
}
