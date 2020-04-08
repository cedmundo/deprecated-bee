%{
#include <stdio.h>
#include <string.h>
#include "y.tab.h"
extern int yylex();
int yyerror(char *s);
char *strval;
%}

%start def_exprs
%token T_ID T_STRING T_NUMBER
%token T_ASSIGN T_LPAR T_RPAR T_COMMA T_COLON T_DOT
%token T_ADD T_SUB T_MUL T_DIV T_MOD
%token T_DEF T_LET T_FOR T_IN T_IF T_ELSE

%right T_ASSIGN
%right T_IN
%left T_COMMA
%left T_LET
%left T_FOR T_IF T_ELSE
%left T_ADD T_SUB
%left T_MUL T_DIV T_MOD
%left T_DOT
%left T_ID T_STRING T_NUMBER
%left T_LPAR T_RPAR
%left T_DEF
%%

def_exprs: def_expr
         | def_exprs def_expr
         ;

id: T_ID

expr: T_LPAR expr T_RPAR
    | lit_expr
    | lookup_expr
    | bin_expr
    | call_expr
    | let_expr
    | def_expr
    | if_expr
    | for_expr
    ;

lit_expr: T_NUMBER | T_STRING

lookup_expr: id

bin_expr: expr T_ADD expr
        | expr T_SUB expr
        | expr T_MUL expr
        | expr T_DIV expr
        | expr T_MOD expr
        ;

call_args: %empty
         | expr
         | call_args T_COMMA expr
         ;

call_expr: id T_LPAR call_args T_RPAR

let_assigns: id T_ASSIGN expr
           | let_assigns T_COMMA id T_ASSIGN expr
           ;

let_expr: T_LET let_assigns T_IN expr

def_params: %empty
          | id
          | def_params T_COMMA id
          ;

def_expr: T_DEF id T_LPAR def_params T_RPAR T_ASSIGN expr

if_expr: expr T_IF expr T_ELSE expr

for_handles: id
           | for_handles T_COMMA id
           ;

for_expr: expr T_FOR for_handles T_IN expr

%%
int main() {
  return yyparse();
}

int yyerror(char *s) {
  fprintf(stderr, "%s\n",s);
  return 0;
}

int yywrap() {
  return 1;
}
