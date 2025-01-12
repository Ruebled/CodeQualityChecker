CC = gcc
LEX = flex
CFLAGS = -Wall -g

all: analyzer

analyzer: lexer.c main.c
	$(CC) $(CFLAGS) -o analyzer lexer.c main.c

lexer.c: lexer.l
	$(LEX) -o lexer.c lexer.l

clean:
	rm -f analyzer lexer.c *.o
