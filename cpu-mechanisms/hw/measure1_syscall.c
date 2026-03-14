#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

int main()
{
    struct timeval start, end;
    long iterations = 1000000; // Execute one million times

    // 1. Record start time
    gettimeofday(&start, NULL);

    // 2. Loop to perform system calls
    for (long i = 0; i < iterations; i++)
    {
        // Read 0 bytes from Standard Input (File Descriptor 0)
        // This is a very lightweight system call and does not actually involve disk I/O
        read(0, NULL, 0);
    }

    // 3. Record end time
    gettimeofday(&end, NULL);

    // 4. Calculate total elapsed time (microseconds)
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    long total_time_in_micros = (seconds * 1000000) + microseconds;

    // 5. Calculate and print average time per call
    double avg_time = (double)total_time_in_micros / iterations;

    printf("Total time for %ld syscalls: %ld microseconds\n", iterations, total_time_in_micros);
    printf("Average time per syscall: %f microseconds\n", avg_time);

    return 0;
}