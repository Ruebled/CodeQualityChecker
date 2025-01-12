%{
#include <stdio.h>
#include <stdlib.h>

// Global variables for tracking line numbers and scope nesting
int line_number = 1;
int scope_nest = 0;

// Function prototype for error reporting
void report_error(const char *error_message, int line, const char *error_text);

%}

%option noyywrap

%%

 /* Increment line number on encountering a newline */
\n {
    line_number++;
}

 /* Detect function declarations and track scope nesting */
(int|float|char|double|void)[ \t]+[a-zA-Z_][a-zA-Z0-9_]*\s*\([^)]*\)\s*\{ {
    scope_nest++;
}

 /* Decrease scope nesting on encountering a closing brace */
\} {
    scope_nest--;
}

 /* Detect global variable declarations and report errors */
(int|float|char|double)[ \t]+[a-zA-Z_][a-zA-Z0-9_]+[ \t]*; {
    if (scope_nest == 0) {
        report_error("Global variable declaration", line_number, yytext);
    }
}

 /* Detect multiple variable declarations on the same line */
(int|float|char|double)[ \t]+[a-zA-Z_][a-zA-Z0-9_]+[ \t]*(,[ \t]*[a-zA-Z_][a-zA-Z0-9_]+)+[ \t]*; {
    report_error("Multiple variable declarations on the same line", line_number, yytext);
}

 /* Detect #define constants with numeric values */
#define[ \t]+[a-zA-Z_][a-zA-Z0-9_]+[ \t]+([0-9]+(\.[0-9]+)?) {
    report_error("Use of #define for numeric constants", line_number, yytext);
}

 /* Detect magic number usage */
([^a-zA-Z_][0-9]+(\.[0-9]+)?) {
    report_error("Magic number usage", line_number, yytext);
}

 /* Detect goto usage */
goto[ \t]+[a-zA-Z_][a-zA-Z0-9_]*; {
	report_error("Use of goto statement", line_number, yytext);
}

 /* Ignore other characters and whitespace */
.|\n { /* No-op */ }
%%

	/* Function to report errors with clear output formatting */
void report_error(const char *error_message, int line, const char *error_text) {
    //fprintf(stderr, "Error [Line %d]: %s (Code: %s)\n", line, error_message, error_text);
    fprintf(stderr, "{\"message\": %s, \"line\": \"%d\"}\n", error_message, line);
}

