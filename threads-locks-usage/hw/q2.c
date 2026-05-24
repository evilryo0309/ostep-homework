#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h> // Here we use gettimeofday for macroscopic time measurement

// 1. Define concurrent counter structure
typedef struct
{
    long long value;
    pthread_mutex_t lock;
} counter_t;

// Declare a global counter
counter_t my_counter;

// Number of increment operations each thread should perform (Workload)
// Set it larger (e.g., one million) to make time differences more observable
int loops = 1000000;

// 2. Basic counter operation functions
void init(counter_t *c)
{
    c->value = 0;
    pthread_mutex_init(&c->lock, NULL);
}

void increment(counter_t *c)
{
    pthread_mutex_lock(&c->lock);
    c->value++;
    pthread_mutex_unlock(&c->lock);
}

// 3. Worker thread function
void *worker(void *arg)
{
    for (int i = 0; i < loops; i++)
    {
        increment(&my_counter);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    // Allow reading the number of threads from command line, default is 1
    int num_threads = 1;
    if (argc > 1)
    {
        num_threads = atoi(argv[1]);
    }

    // TODO 1: Call init to initialize counter my_counter
    init(&my_counter);

    // TODO 2: Insert your start timer here (gettimeofday)
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // TODO 3: Create num_threads threads
    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&threads[i], NULL, worker, NULL);
    }

    // TODO 4: Wait for all threads to complete
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // TODO 5: Insert your end timer and calculate elapsed time
    gettimeofday(&end, NULL);
    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

    // Print results to verify correctness and performance
    printf("Threads: %d\n", num_threads);
    printf("Final Counter Value: %lld (Expected: %lld)\n", my_counter.value, (long long)num_threads * loops);
    printf("Elapsed Time: %.3f ms\n", elapsed_ms);
    printf("Average Time per Increment: %.6f ms\n", elapsed_ms / (num_threads * loops));

    return 0;
}