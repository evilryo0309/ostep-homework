#include <stdio.h>
#include <stdlib.h>

int main()
{
    // Create an integer array of size 100
    int *data = (int *)malloc(100 * sizeof(int));

    // Create a "funny value": let the pointer point to the middle of the array (the 50th element)
    int *funny_ptr = data + 50;

    // [DANGEROUS!] Try to free this middle address
    free(funny_ptr);

    return 0;
}