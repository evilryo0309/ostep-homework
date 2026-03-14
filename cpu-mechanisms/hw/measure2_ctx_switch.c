#define _GNU_SOURCE // Must define this to use Linux-specific macros like CPU_SET
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sched.h>

int main()
{
    int pipe1[2]; // Pipe from Parent to Child
    int pipe2[2]; // Pipe from Child to Parent

    // Create pipes
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1)
    {
        perror("Pipe creation failed");
        exit(1);
    }

    // Bind the following processes to CPU core 0
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &set) == -1)
    {
        perror("sched_setaffinity failed");
        exit(1);
    }

    pid_t pid = fork();
    int iterations = 1000000; // Number of switches
    char token = 'X';         // 1 byte dummy data to transfer

    if (pid < 0)
    {
        perror("Fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        // === Child Process ===
        // Child inherits CPU affinity setting, so it is also bound to CPU 0

        // Close unused pipe ends
        close(pipe1[1]); // Child does not write to pipe1
        close(pipe2[0]); // Child does not read from pipe2

        for (int i = 0; i < iterations; i++)
        {
            read(pipe1[0], &token, 1);  // Wait to read from Parent (Context Switch occurs here)
            write(pipe2[1], &token, 1); // After reading, immediately write back to Parent
        }
        exit(0);
    }
    else
    {
        // === Parent Process ===
        struct timeval start, end;

        // Close unused pipe ends
        close(pipe1[0]); // Parent does not read from pipe1
        close(pipe2[1]); // Parent does not write to pipe2

        gettimeofday(&start, NULL);

        for (int i = 0; i < iterations; i++)
        {
            write(pipe1[1], &token, 1); // Write to Child
            read(pipe2[0], &token, 1);  // Wait to read from Child (Context Switch occurs here)
        }

        gettimeofday(&end, NULL);
        wait(NULL); // Wait for Child to finish

        long seconds = end.tv_sec - start.tv_sec;
        long microseconds = end.tv_usec - start.tv_usec;
        long total_time_in_micros = (seconds * 1000000) + microseconds;

        // A total of iterations * 2 context switches occurred (there and back)
        double avg_time = (double)total_time_in_micros / (iterations * 2);

        printf("Total time for %d ping-pongs: %ld microseconds\n", iterations, total_time_in_micros);
        printf("Average context switch time: %f microseconds\n", avg_time);
    }

    return 0;
}