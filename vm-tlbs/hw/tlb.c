#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h> // For gettimeofday() to measure time

#define PAGE_SIZE 4096

int main(int argc, char *argv[])
{
    // 1. The program expects two command-line arguments: the size of the TLB (in number of pages) and the number of tests to perform.
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <tlb_size> <num of tests>\n", argv[0]);
        return 1;
    }

    // 2. Parse the command-line arguments to get the TLB size and the number of tests.
    int num_pages = atoi(argv[1]);
    int num_tests = atoi(argv[2]);

    volatile int *large_ary = malloc(num_pages * PAGE_SIZE); // Allocate a large array to test TLB behavior
    if (large_ary == NULL)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        return 1;
    }

    int jump = PAGE_SIZE / sizeof(int);

    // 3. Warm up the TLB by accessing each page in the array before starting the timer. This ensures that the pages are loaded into memory and the TLB is populated with the relevant entries.
    for (int j = 0; j < num_pages * jump; j += jump)
    {
        // Access each page in the array
        large_ary[j] += 1; // Write to the page to ensure it is loaded into memory
    }

    // 4. Ready for stopwatching the TLB behavior. The program should access a large array (e.g., 10 times the size of the TLB) and measure the time taken to access each page in the array.
    struct timeval start, end;
    gettimeofday(&start, NULL); // Start the timer

    for (int i = 0; i < num_tests; i++)
    {
        for (int j = 0; j < num_pages * jump; j += jump)
        {
            // Access each page in the array
            large_ary[j] += 1; // Write to the page to ensure it is loaded into memory
        }
    }

    gettimeofday(&end, NULL); // Stop the timer

    // 5. Calculate the elapsed time
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double total_seconds = seconds + microseconds * 1e-6;

    // 6. Calculate the total number of accesses
    long long total_accesses = (long long)num_tests * num_pages;

    // 7. Calculate the average time per access(ns)
    double time_per_access_ns = (total_seconds * 1e9) / total_accesses;

    // 8. Format the output as specified: page number, average access time in nanoseconds
    // Note: The page number is not explicitly calculated here, but the num_pages variable represents the size of the TLB in terms of the number of pages. The average access time is printed in nanoseconds.
    printf("%d, %f\n", num_pages, time_per_access_ns);

    free((void *)large_ary);
    return 0;
}