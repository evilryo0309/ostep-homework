#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    // Check if the user provided the required argument
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <megabytes>\n", argv[0]);
        return 1;
    }

    // Convert the input argument from string to integer
    int mb = atoi(argv[1]);
    if (mb <= 0)
    {
        fprintf(stderr, "Please enter a valid positive number for MB.\n");
        return 1;
    }

    // Convert MB to bytes
    size_t length = (size_t)mb * 1024 * 1024;

    // Allocate memory
    int *array = (int *)malloc(length);
    if (array == NULL)
    {
        fprintf(stderr, "Memory allocation failed for %d MB.\n", mb);
        return 1;
    }

    // Q6: Print your own PID for later use with pmap
    printf("PID: %d\n", getpid());
    printf("Allocated %d MB. Touching memory indefinitely...\n", mb);

    // Calculate the number of integers that can fit in the allocated memory
    size_t num_ints = length / sizeof(int);

    // Q3: Continuously iterate through the array and modify it (Touch memory)
    // Operating systems usually use lazy allocation, and physical memory is only allocated when it is actually written to
    while (1)
    {
        for (size_t i = 0; i < num_ints; i++)
        {
            array[i] = 0; // Write to the memory to ensure it's allocated
        }
    }

    return 0; // This line will never be reached due to the infinite loop
}