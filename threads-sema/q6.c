#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "common_threads.h"

//
// Here, you have to write (almost) ALL the code. Oh no!
// How can you show that a thread does not starve
// when attempting to acquire this mutex you build?
//

typedef struct __ns_mutex_t
{
    int room1;   // Count of threads waiting in Room 1
    int room2;   // Count of threads waiting in Room 2
    sem_t mutex; // Lock to protect the two counters
    sem_t t1;    // Turnstile 1 to control entry to Room 1
    sem_t t2;    // Turnstile 2 to control entry to Room 2
} ns_mutex_t;

void ns_mutex_init(ns_mutex_t *m)
{
    m->room1 = 0;
    m->room2 = 0;
    Sem_init(&m->mutex, 1);
    Sem_init(&m->t1, 1);
    Sem_init(&m->t2, 1);
}

void ns_mutex_acquire(ns_mutex_t *m)
{
    // 1. Enter Room 1 to check in: increment room1 to let the system know I'm queuing
    Sem_wait(&m->mutex);
    m->room1++;
    Sem_post(&m->mutex);

    // 2. Wait at turnstile t1
    Sem_wait(&m->t1);

    // 3. Transition phase: leave Room 1 and enter Room 2
    Sem_wait(&m->mutex);
    m->room1--;
    m->room2++;

    if (m->room1 == 0)
    {
        // If I'm the last person in Room 1, close t1 and open t2
        Sem_post(&m->t2);
    }
    else
    {
        // If there are still others in Room 1, let t1 continue to allow passage
        Sem_post(&m->t1);
    }
    Sem_post(&m->mutex);

    // 4. Queue in Room 2 and wait to enter the critical section
    Sem_wait(&m->t2);
    m->room2--;
    // After passing t2, enter the critical section!
}

void ns_mutex_release(ns_mutex_t *m)
{
    // When leaving Critical Section, check if there are still threads in Room 2
    if (m->room2 == 0)
    {
        // If Room 2 is empty, all threads in this batch have completed execution
        // Reopen t1 gate to allow the next batch of threads waiting in Room 1 to enter
        Sem_post(&m->t1);
    }
    else
    {
        // If there are still other threads in this batch in Room 2, open t2 to allow the next thread to enter Critical Section
        Sem_post(&m->t2);
    }
}

// Declare global lock and shared counter
ns_mutex_t m;
int counter = 0;

void *worker(void *arg)
{
    // Convert the input parameter to thread ID for easy identification
    int thread_id = (int)(long)arg;

    printf("Thread %d: attempting to acquire lock...\n", thread_id);

    // Attempt to acquire lock (enter Room 1 -> Room 2)
    ns_mutex_acquire(&m);

    // Enter Critical Section
    printf("Thread %d: acquired lock! Counter was %d, updating...\n", thread_id, counter);
    counter++;

    // Simulate some work time to give other threads a chance to queue
    usleep(1000);

    printf("Thread %d: releasing lock.\n", thread_id);

    // Leave Critical Section and release lock
    ns_mutex_release(&m);

    return NULL;
}

int main(int argc, char *argv[])
{
    printf("parent: begin\n");

    int num_threads = 10; // Create 10 threads to compete for the lock
    pthread_t threads[num_threads];

    // 1. Initialize the lock
    ns_mutex_init(&m);

    // 2. Create multiple threads
    for (long i = 0; i < num_threads; i++)
    {
        Pthread_create(&threads[i], NULL, worker, (void *)i);
    }

    // 3. Wait for all threads to complete
    for (int i = 0; i < num_threads; i++)
    {
        Pthread_join(threads[i], NULL);
    }

    // 4. Verify the final result
    printf("parent: end, final counter = %d (should be %d)\n", counter, num_threads);
    return 0;
}