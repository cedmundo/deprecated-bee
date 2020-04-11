%{
#include <stdio.h>
#include "run.h"
#include "y.tab.h"
int fileno(FILE *stream);
extern char* strval;
%}

%option nounput
%option noinput
%%
" "         ;
"\n"        ;
"\t"        ;
"="         { return T_ASSIGN; }
"=="        { return T_EQ; }
"!="        { return T_NEQ; }
">"         { return T_GT; }
">="        { return T_GE; }
"<"         { return T_LT; }
"<="        { return T_LE; }
"("         { return T_LPAR; }
")"         { return T_RPAR; }
"{"         { return T_LCB; }
"}"         { return T_RCB; }
"["         { return T_LSB; }
"]"         { return T_RSB; }
"&"         { return T_AND; }
"&&"        { return T_ANDS; }
"|"         { return T_OR; }
"||"        { return T_ORS; }
"^"         { return T_XOR; }
"!"         { return T_NOT; }
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

\-?[0-9ABCDEF]+(\.[0-9ABCDEF]+)? {
  strval = yytext;
  return T_NUMBER;
}
