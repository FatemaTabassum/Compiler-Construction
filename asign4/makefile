cexpr: lex.yy.c y.tab.c
	gcc -g lex.yy.c y.tab.c -o cexpr

lex.yy.c: y.tab.c scan.l
	lex scan.l

y.tab.c: cexpr.y
	yacc -d cexpr.y

clean: 
	rm -rf lex.yy.c y.tab.c y.tab.h cexpr cexpr.dSYM
