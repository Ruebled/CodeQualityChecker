#include <stdio.h>
#include <stdlib.h>

extern int yylex();
extern FILE *yyin;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    // Open the input file
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror("Error opening file");
        return 1;
    }

    // Run the lexer
    printf("Analyzing file: %s\n", argv[1]);
    yylex();

    fclose(yyin);
    return 0;
}
