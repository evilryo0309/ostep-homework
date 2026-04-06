#include <stdio.h>
#include <stdlib.h>

int main()
{
    // Allocate memory for 100 integers.
    int *data = (int *)malloc(sizeof(int) * 100);

    // Use the allocated memory, make sure compiler does not optimize it away.
    data[0] = 42;
    printf("data[0] is %d\n", data[0]);

    // free(data); // This line is intentionally commented out to create a memory leak.
    return 0;
}
