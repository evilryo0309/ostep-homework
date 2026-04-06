#include <stdio.h>
#include <stdlib.h>

int main()
{
    // Create an integer array of size 100
    int *data = (int *)malloc(100 * sizeof(int));

    // Set the 100th element to 0
    data[100] = 0;

    // To develop good habits (although memory leak is not the focus of this exercise), we free it this time
    free(data);

    return 0;
}
