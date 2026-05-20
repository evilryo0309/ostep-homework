#include <sys/time.h>
#include <stdio.h>

int main()
{
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Your code to be timed goes here
    for (volatile long long i = 0; i < 1000000000; i++)
        ; // Example workload

    gettimeofday(&end, NULL);

    long long start_usec = start.tv_sec * 1000000 + start.tv_usec;
    long long end_usec = end.tv_sec * 1000000 + end.tv_usec;
    long long elapsed_usec = end_usec - start_usec;

    printf("Elapsed time: %lld us (%.3f ms)\n", elapsed_usec, elapsed_usec / 1000.0);

    return 0;
}
