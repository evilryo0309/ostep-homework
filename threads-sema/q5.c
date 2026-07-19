#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common_threads.h"

//
// Your code goes in the structure and functions below
//

typedef struct __rwlock_t
{
    sem_t mutex;     // Mutex lock to protect the counter
    sem_t writelock; // Semaphore to allow writers exclusive access
    sem_t turnstile; // [NEW] Used as a queue gate to block endless stream of Readers
    int readers;     // Track how many readers are currently reading
} rwlock_t;

void rwlock_init(rwlock_t *rw)
{
    Sem_init(&rw->mutex, 1);
    Sem_init(&rw->writelock, 1);
    Sem_init(&rw->turnstile, 1); // [NEW] Initialize to 1, allowing the first person to pass
    rw->readers = 0;
}

void rwlock_acquire_readlock(rwlock_t *rw)
{
    Sem_wait(&rw->turnstile); // [NEW] Reader must pass through turnstile first

    Sem_wait(&rw->mutex);
    rw->readers++;
    if (rw->readers == 1)
    {
        Sem_wait(&rw->writelock); // First reader is responsible for locking writelock
    }
    Sem_post(&rw->mutex);

    Sem_post(&rw->turnstile); // [NEW] After reader passes, immediately open the gate for others to queue
}

void rwlock_release_readlock(rwlock_t *rw)
{
    Sem_wait(&rw->mutex);
    rw->readers--;
    if (rw->readers == 0)
    {
        Sem_post(&rw->writelock); // Last reader is responsible for releasing writelock
    }
    Sem_post(&rw->mutex);
}

void rwlock_acquire_writelock(rwlock_t *rw)
{
    Sem_wait(&rw->turnstile); // [NEW] Writer must also pass through turnstile first!
                              // Note: Writer will NOT immediately release the turnstile
                              // This causes all new Readers to block on Sem_wait(&rw->turnstile)

    Sem_wait(&rw->writelock); // Writer waits for existing Readers to gradually leave until readers == 0
}

void rwlock_release_writelock(rwlock_t *rw)
{
    Sem_post(&rw->writelock); // Writer finished writing, release write lock
    Sem_post(&rw->turnstile); // [NEW] When writer leaves, release the gate to allow queued Readers or Writers to proceed
}

//
// Don't change the code below (just use it!)
//

int loops;
int value = 0;

rwlock_t lock;

void *reader(void *arg)
{
    int i;
    for (i = 0; i < loops; i++)
    {
        rwlock_acquire_readlock(&lock);
        printf("read %d\n", value);
        rwlock_release_readlock(&lock);
    }
    return NULL;
}

void *writer(void *arg)
{
    int i;
    for (i = 0; i < loops; i++)
    {
        rwlock_acquire_writelock(&lock);
        value++;
        printf("write %d\n", value);
        rwlock_release_writelock(&lock);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    assert(argc == 4);
    int num_readers = atoi(argv[1]);
    int num_writers = atoi(argv[2]);
    loops = atoi(argv[3]);

    pthread_t pr[num_readers], pw[num_writers];

    rwlock_init(&lock);

    printf("begin\n");

    int i;
    for (i = 0; i < num_readers; i++)
        Pthread_create(&pr[i], NULL, reader, NULL);
    for (i = 0; i < num_writers; i++)
        Pthread_create(&pw[i], NULL, writer, NULL);

    for (i = 0; i < num_readers; i++)
        Pthread_join(pr[i], NULL);
    for (i = 0; i < num_writers; i++)
        Pthread_join(pw[i], NULL);

    printf("end: value %d\n", value);

    return 0;
}
