#include <stdio.h>
#include <stdlib.h>

int main()
{
    // Allocate an integer array of size 100
    int *data = (int *)malloc(100 * sizeof(int));

    // Assign a value first
    data[0] = 123;

    // Free the memory back to the system
    free(data);

    // [DANGER!] Attempt to print the contents of already freed memory
    // [This is a use-after-free vulnerability, as we are accessing memory that has already been deallocated. The behavior of this code is undefined and may lead to crashes or security issues.]
    printf("data[0] is: %d\n", data[0]);

    return 0;
}
