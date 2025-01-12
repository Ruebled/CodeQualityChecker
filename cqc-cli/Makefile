CC = gcc
LEX = flex
CFLAGS = -Wall -g

all: analyzer

analyzer: lexer.c main.c
	$(CC) $(CFLAGS) -DYY_NO_INPUT -DYY_NO_UNPUT -o analyzer lexer.c main.c
	cp analyzer ../bin

lexer.c: lexer.l
	$(LEX) -o lexer.c lexer.l

clean:
	rm -f analyzer lexer.c *.o
