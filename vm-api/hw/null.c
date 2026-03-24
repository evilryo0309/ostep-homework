#include <stdio.h>

int main()
{
    // Declare a pointer and initialize it to NULL
    int *p = NULL;

    // Dereferencing a NULL pointer will lead to undefined behavior
    printf("The value is: %d\n", *p);

    return 0;
}
