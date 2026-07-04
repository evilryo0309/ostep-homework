#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_THREADS 64 // Default maximum number of threads supported

// 1. Define Approximate Counter structure
typedef struct __counter_t
{
    long long global;                   // Global counter
    pthread_mutex_t glock;              // Lock protecting global counter
    long long local[MAX_THREADS];       // Local counter for each thread
    pthread_mutex_t llock[MAX_THREADS]; // Lock protecting each local counter
    int threshold;                      // Synchronization threshold (S)
} counter_t;

counter_t my_counter;
int loops = 1000000; // Total increment workload for each thread

// 2. Initialization function
void init(counter_t *c, int threshold)
{
    c->threshold = threshold;
    c->global = 0;
    pthread_mutex_init(&c->glock, NULL);

    for (int i = 0; i < MAX_THREADS; i++)
    {
        c->local[i] = 0;
        pthread_mutex_init(&c->llock[i], NULL);
    }
}

// 3. Update function (core logic)
void update(counter_t *c, int threadID, int amt)
{
    int cpu = threadID % MAX_THREADS;

    // Lock the local lock for this thread and update local counter
    pthread_mutex_lock(&c->llock[cpu]);
    c->local[cpu] += amt;

    // Check if threshold S is reached
    if (c->local[cpu] >= c->threshold)
    {
        // Threshold reached, lock global lock and synchronize value
        pthread_mutex_lock(&c->glock);
        c->global += c->local[cpu];
        pthread_mutex_unlock(&c->glock);

        // Reset local counter to zero
        c->local[cpu] = 0;
    }
    pthread_mutex_unlock(&c->llock[cpu]);
}

// 4. Read function (returns approximate value of global counter only)
long long get(counter_t *c)
{
    pthread_mutex_lock(&c->glock);
    long long val = c->global;
    pthread_mutex_unlock(&c->glock);
    return val; // Note: This is not an exact sum since there are residual values in local counters
}

// Parameter structure passed to thread
typedef struct
{
    int threadID;
} thread_arg_t;

// 5. Worker thread function
void *worker(void *arg)
{
    thread_arg_t *t_arg = (thread_arg_t *)arg;
    int tid = t_arg->threadID;

    for (int i = 0; i < loops; i++)
    {
        update(&my_counter, tid, 1);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int num_threads = 1;
    int threshold = 1; // Default threshold is 1 (behavior degrades to single-lock performance)

    if (argc > 1)
        num_threads = atoi(argv[1]);
    if (argc > 2)
        threshold = atoi(argv[2]);

    if (num_threads > MAX_THREADS)
    {
        printf("Error: Number of threads exceeds MAX_THREADS (%d)\n", MAX_THREADS);
        return 1;
    }

    init(&my_counter, threshold);

    // Prepare threads and their arguments
    pthread_t threads[num_threads];
    thread_arg_t args[num_threads];

    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Create threads
    for (int i = 0; i < num_threads; i++)
    {
        args[i].threadID = i;
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end, NULL);
    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

    // Calculate final exact sum (after experiment ends, add residual local values to verify correctness)
    long long final_exact_value = my_counter.global;
    for (int i = 0; i < num_threads; i++)
    {
        final_exact_value += my_counter.local[i];
    }

    printf("Threads: %d, Threshold (S): %d\n", num_threads, threshold);
    printf("Approximate Global Value: %lld\n", get(&my_counter));
    printf("Final Exact Value: %lld (Expected: %lld)\n", final_exact_value, (long long)num_threads * loops);
    printf("Elapsed Time: %.3f ms\n", elapsed_ms);

    return 0;
}