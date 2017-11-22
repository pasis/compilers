%{
  #define YYSTYPE struct atom *

  #include "y.tab.h"
  #include "c.h"
%}

DIGIT	[0-9]
IDENT	[a-z][a-z0-9]*

%%

true|false	{ yylval = atom_bool(yytext); return BOOL; }
skip		{ return SKIP; }
if		{ return IF; }
else		{ return ELSE; }
while		{ return WHILE; }
"=="		{ return EQUALS; }
"="		{ return yytext[0]; }
"+"|"*"|";"|"<"	{ return yytext[0]; }
"{"|"}"|"("|")"	{ return yytext[0]; }
{DIGIT}+	{ yylval = atom_int(yytext); return INT; }
{IDENT}		{ yylval = atom_id(yytext); return ID; }
[ \t\n]		/* whilespaces */

%%
