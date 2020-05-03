%{
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

struct scope;
#include "y.tab.h"
#include "ast.h"
#include "run.h"
#include "scope.h"

extern int yylex();
char *strval;
int yyerror(struct scope *scope, char *s);
%}
%start program
%token T_ID T_STRING T_NUMBER
%token T_ASSIGN T_LPAR T_RPAR T_COMMA T_COLON T_DOT T_LCB T_RCB T_LSB T_RSB
%token T_EQ T_NEQ T_GT T_GE T_LT T_LE
%token T_ANDS T_ORS T_AND T_OR T_XOR T_NOT
%token T_ADD T_SUB T_MUL T_DIV T_MOD
%token T_DEF T_LET T_FOR T_IN T_IF T_ELSE T_REDUCE T_WITH

%union {
  int token;
  char *str;
  struct def_exprs *def_exprs;
  struct expr *expr;
  struct lit_expr *lit_expr;
  struct lookup_expr *lookup_expr;
  struct bin_expr *bin_expr;
  struct unit_expr *unit_expr;
  struct call_args *call_args;
  struct call_expr *call_expr;
  struct let_assigns *let_assigns;
  struct let_expr *let_expr;
  struct def_params *def_params;
  struct def_expr *def_expr;
  struct if_expr *if_expr;
  struct for_handles *for_handles;
  struct for_expr *for_expr;
  struct list_expr *list_expr;
}

%type<str> id
%type<def_exprs> def_exprs
%type<expr> expr
%type<lit_expr> lit_expr
%type<lookup_expr> lookup_expr
%type<bin_expr> bin_expr
%type<unit_expr> unit_expr
%type<call_args> call_args
%type<call_expr> call_expr
%type<let_assigns> let_assigns
%type<let_expr> let_expr
%type<def_params> def_params
%type<def_expr> def_expr
%type<if_expr> if_expr
%type<for_handles> for_handles
%type<for_expr> for_expr
%type<list_expr> list_expr list_items

%parse-param {struct scope *scope}

%right T_ASSIGN
%right T_IN
%left T_COMMA
%left T_LET
%left T_FOR T_IF T_ELSE
%left T_EQ T_NEQ T_LT T_LE T_GT T_GE
%left T_ANDS T_ORS T_AND T_OR T_XOR T_NOT
%left T_ADD T_SUB
%left T_MUL T_DIV T_MOD
%left T_DOT
%left T_ID T_STRING T_NUMBER
%left T_LSB T_RSB
%left T_LCB T_RCB
%left T_LPAR T_RPAR
%left T_DEF
%%

program: def_exprs { run_all_def_exprs(scope, $1); }

def_exprs: def_expr             { $$ = make_def_exprs($1); }
         | def_exprs def_expr   { $$ = append_def_exprs($1, $2); }
         ;

id: T_ID { $$ = strdup(strval); }

expr: T_LPAR expr T_RPAR  { $$ = $2; }
    | lit_expr            { $$ = make_expr_from_lit($1); }
    | lookup_expr         { $$ = make_expr_from_lookup($1); }
    | bin_expr            { $$ = make_expr_from_bin($1);}
    | unit_expr           { $$ = make_expr_from_unit($1);}
    | call_expr           { $$ = make_expr_from_call($1); }
    | let_expr            { $$ = make_expr_from_let($1); }
    | def_expr            { $$ = make_expr_from_def($1); }
    | if_expr             { $$ = make_expr_from_if($1); }
    | for_expr            { $$ = make_expr_from_for($1); }
    | list_expr           { $$ = make_expr_from_list($1); }
    | reduce_expr         { $$ = NULL; }
    ;

lit_expr: T_NUMBER { $$ = make_lit_expr(LIT_NUMBER, strdup(strval)); }
        | T_STRING { $$ = make_lit_expr(LIT_STRING, strdup(strval)); }
        ;

lookup_expr: id { $$ = make_lookup_expr($1); }

bin_expr: expr T_ADD expr  { $$ = make_bin_expr($1, $3, OP_ADD); }
        | expr T_SUB expr  { $$ = make_bin_expr($1, $3, OP_SUB); }
        | expr T_MUL expr  { $$ = make_bin_expr($1, $3, OP_MUL); }
        | expr T_DIV expr  { $$ = make_bin_expr($1, $3, OP_DIV); }
        | expr T_MOD expr  { $$ = make_bin_expr($1, $3, OP_MOD); }
        | expr T_ANDS expr { $$ = make_bin_expr($1, $3, OP_ANDS); }
        | expr T_ORS expr  { $$ = make_bin_expr($1, $3, OP_ORS); }
        | expr T_AND expr  { $$ = make_bin_expr($1, $3, OP_AND); }
        | expr T_OR expr   { $$ = make_bin_expr($1, $3, OP_OR); }
        | expr T_XOR expr  { $$ = make_bin_expr($1, $3, OP_XOR); }
        | expr T_EQ expr   { $$ = make_bin_expr($1, $3, OP_EQ); }
        | expr T_NEQ expr  { $$ = make_bin_expr($1, $3, OP_NEQ); }
        | expr T_LT expr   { $$ = make_bin_expr($1, $3, OP_LT); }
        | expr T_LE expr   { $$ = make_bin_expr($1, $3, OP_LE); }
        | expr T_GT expr   { $$ = make_bin_expr($1, $3, OP_GT); }
        | expr T_GE expr   { $$ = make_bin_expr($1, $3, OP_GE); }
        ;

unit_expr: T_NOT expr               { $$ = make_unit_expr($2, OP_NOT); }
         | T_SUB expr %prec T_SUB   { $$ = make_unit_expr($2, OP_NEG); }
         ;

call_args: %empty                   { $$ = NULL; }
         | expr                     { $$ = make_call_args($1); }
         | call_args T_COMMA expr   { $$ = append_call_args($1, $3); }
         ;

call_expr: id T_LPAR call_args T_RPAR { $$ = make_call_expr($1, $3); }

let_assigns: id T_ASSIGN expr
              { $$ = make_let_assigns($1, $3); }
           | let_assigns T_COMMA id T_ASSIGN expr
              { $$ = append_let_assigns($1, $3, $5); }
           ;

let_expr: T_LET let_assigns T_IN expr { $$ = make_let_expr($2, $4); }

def_params: %empty                  { $$ = NULL; }
          | id                      { $$ = make_def_params($1); }
          | def_params T_COMMA id   { $$ = append_def_params($1, $3); }
          ;

def_expr: T_DEF id T_LPAR def_params T_RPAR T_ASSIGN expr
          { $$ = make_def_expr($2, $4, $7); }

if_expr: expr T_IF expr T_ELSE expr   { $$ = make_if_expr($1, $3, $5); }

for_handles: id                       { $$ = make_for_handles($1); }
           | for_handles T_COMMA id   { $$ = append_for_handles($1, $3); }
           ;

for_expr: expr T_FOR for_handles T_IN expr
            { $$ = make_for_expr($1, $3, $5); }
        | expr T_FOR for_handles T_IN expr T_IF expr
            {}
        ;

reduce_expr: T_REDUCE for_expr T_WITH id T_ASSIGN expr
            {}
           ;

list_items: %empty                    { $$ = NULL; }
          | expr                      { $$ = make_list_expr($1); }
          | list_items T_COMMA expr   { $$ = append_list_expr($1, $3); }
          ;

list_expr: T_LSB list_items T_RSB     { $$ = $2; }

%%
int main() {
  struct scope *globals = malloc(sizeof(struct scope));
  scope_init(globals);
  globals->global = globals; // Weird but needed
  scope_builtins(globals);
  int res = yyparse(globals);
  run_main(globals);
  scope_leave(globals);
  free(globals);
  return res;
}

int yyerror(struct scope *scope, char *s) {
  fprintf(stderr, "%s\n",s);
  return 0;
}

int yywrap() {
  return 1;
}
