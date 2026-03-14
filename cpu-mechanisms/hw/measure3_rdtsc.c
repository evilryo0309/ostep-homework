#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

// Define a function to read the 64-bit TSC value
static inline uint64_t rdtsc()
{
    unsigned int lo, hi;
    // Use inline assembly to call rdtsc
    // The lower 32 bits of the result are stored in EAX (lo), the upper 32 bits in EDX (hi)
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

int main()
{
    long interations = 1000000; // Number of iterations to average over

    uint64_t total_cycles = 0;
    uint64_t start, end;

    for (long i = 0; i < interations; i++)
    {
        // Measure the CPU cycles for a single system call
        start = rdtsc();
        read(0, NULL, 0);
        end = rdtsc();

        total_cycles += (end - start);
    }

    double avg_cycles = (double)total_cycles / interations;
    double rounded_cycles = (avg_cycles > 0) ? (double)((long long)(avg_cycles + 0.5)) : (double)((long long)(avg_cycles - 0.5));
    printf("A single read(0, NULL, 0) took %.0f CPU cycles on average\n", rounded_cycles);

    return 0;
}