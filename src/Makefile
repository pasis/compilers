CFLAGS = -g3 -Wall -DYY_NO_UNPUT -DYY_NO_INPUT
LDFLAGS = -lfl

target = c
src = c.c compile.c eval.c
headers = c.h compile.h eval.h

lex_target = l.yy.c
lex_src = l.l
yacc_target = y.tab.c
yacc_src = y.y

all: $(target)

$(target): $(src) $(headers) $(lex_target) $(yacc_target)
	gcc $(CFLAGS) -o $(target) $(src) $(lex_target) $(yacc_target) $(LDFLAGS)

$(lex_target): $(lex_src)
	flex -o $(lex_target) $(lex_src)

$(yacc_target): $(yacc_src)
	bison -d $(yacc_src)
