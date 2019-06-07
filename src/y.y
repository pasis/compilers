%{
  #define YYSTYPE struct atom *

  #include <stdio.h>
  #include "c.h"
  #include "compile.h"
  #include "eval.h"

  int yylex(void);
  void yyerror(char const *s);
%}

%token INT
%token BOOL
%token ID
%token EQUALS
%token SKIP
%token IF
%token ELSE
%token WHILE

%left '+'
%left '*'

%%

program:
  cmd_list				{ atom_compile($1, ARCH_X86_64); /* eval or compile here */ }
;

cmd_list:
  cmd ';'				{ $$ = atom_list($1); }
| block					{ $$ = atom_list($1); }
| cmd_list cmd ';'			{ $$ = atom_list_add($1, $2); }
| cmd_list block			{ $$ = atom_list_add($1, $2); }
;

block:
  '{' '}'				{ $$ = atom_skip(); }
| '{' cmd_list '}'			{ $$ = $2; }
;

body:
  cmd					{ $$ = $1; }
| block					{ $$ = $1; }
;

cmd:
  SKIP					{ $$ = atom_skip(); }
| ID '=' aexpr				{ $$ = atom_assign($1, $3); }
| IF '(' cond ')' body ELSE body	{ $$ = atom_if($3, $5, $7); }
| WHILE '(' cond ')' body		{ $$ = atom_while($3, $5); }
;

aexpr:
  INT					{ $$ = $1; }
| ID					{ $$ = $1; }
| aexpr '+' aexpr			{ $$ = atom_expr(EXPR_ADD, $1, $3); }
| aexpr '*' aexpr			{ $$ = atom_expr(EXPR_MUL, $1, $3); }
| '(' aexpr ')'				{ $$ = $2; }
;

cond:
  BOOL					{ $$ = $1; }
| aexpr '<' aexpr			{ $$ = atom_cond(COND_LESS, $1, $3); }
| aexpr EQUALS aexpr			{ $$ = atom_cond(COND_EQUALS, $1, $3); }
;

%%

void yyerror(char const *s)
{
	fprintf(stderr, "%s\n", s);
}

int main()
{
	int rc;

	rc = atom_init();
	if (rc == 0) {
		rc = yyparse();
		atom_fini();
	}
	return rc;
}
