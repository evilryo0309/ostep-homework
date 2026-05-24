#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common_threads.h"

//
// Your code goes in the structure and functions below
//

typedef struct __rwlock_t
{
    sem_t lock;      // protects readers
    sem_t writelock; // protects writers
    int readers;     // number of active readers
} rwlock_t;

void rwlock_init(rwlock_t *rw)
{
    Sem_init(&rw->lock, 1);      // Initialize to 1 to allow one thread to access the readers count
    Sem_init(&rw->writelock, 1); // Initialize to 1 to allow one thread to access the writers
    rw->readers = 0;             // Initialize the number of active readers to 0
}

void rwlock_acquire_readlock(rwlock_t *rw)
{
    Sem_wait(&rw->lock); // Ready to modify the readers count, so acquire the lock

    rw->readers++;
    if (rw->readers == 1)         // If this is the first reader, it needs to acquire the writelock to block writers
        Sem_wait(&rw->writelock); // Block writers if this is the first reader

    Sem_post(&rw->lock); // Done modifying the readers count, so release the lock
}

void rwlock_release_readlock(rwlock_t *rw)
{
    Sem_wait(&rw->lock); // Ready to modify the readers count, so acquire the lock

    rw->readers--;
    if (rw->readers == 0)         // If this is the last reader, it needs to release the writelock to allow writers
        Sem_post(&rw->writelock); // Allow writers if this is the last reader

    Sem_post(&rw->lock); // Done modifying the readers count, so release the lock
}

void rwlock_acquire_writelock(rwlock_t *rw)
{
    Sem_wait(&rw->writelock); // Writers need to acquire the writelock to block both readers and other writers
}

void rwlock_release_writelock(rwlock_t *rw)
{
    Sem_post(&rw->writelock); // Writers release the writelock to allow other readers and writers
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

        // Let the reader hold the lock to let the other readers have time to reach to increase the number of active readers.
        sleep(1);

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
