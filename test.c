// Includes
#include <stdio.h>

// Constants
#define MAX 100

// Global variable
int globalVar;

// Multiple variables declared in one line
int var1, var2;

// Function missing documentation
void undocumentedFunction() {
	// Local variable // Magic number
    int a = 42;
	goto label;
	int var_here = 36;
label:
	return;
}

// Function missing documentation
int main() {
    printf("Hello, World!");

	// Magic number
    return 0; 
}
