CC 	= cc
CFLAGS 	= -Wall -std=c11 -I. -I/usr/include
HEADERS	= ast.h
OBJ 	= ast.o y.tab.o lex.yy.o
YACC 	= bison
YFLAGS 	= -y -d
LEX 	= lex

all: minigun

minigun: $(OBJ)
	mkdir -p bin
	$(CC) -o bin/$@ $^ $(CFLAGS)

%.o: %.c %(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

y.tab.o: y.tab.c
	$(CC) -c -o $@ y.tab.c $(CFLAGS)

lex.yy.o: lex.yy.c y.tab.h
	$(CC) -c -o $@ lex.yy.c $(CFLAGS)

y.tab.c: parser.y lex.yy.c
	$(YACC) $(YFLAGS) parser.y

lex.yy.c: lexer.l
	$(LEX) $(LFLAGS) lexer.l

clean:
	rm -r bin
	rm *.o y.tab.h y.tab.c lex.yy.c

test: all
	./bin/minigun
