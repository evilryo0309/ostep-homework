#include <stdio.h>
#include <unistd.h>
#include "common_threads.h"

// If done correctly, each child should print their "before" message
// before either prints their "after" message. Test by adding sleep(1)
// calls in various locations.

sem_t s1, s2;

void *child_1(void *arg)
{
    printf("child 1: before\n");

    sleep(1); // test by adding sleep(1)

    // what goes here?
    Sem_post(&s1); // Signal child 2 that child 1 has printed "before".

    sleep(1); // test by adding sleep(1)

    Sem_wait(&s2); // Wait for child 2 to print "before".

    printf("child 1: after\n");
    return NULL;
}

void *child_2(void *arg)
{
    sleep(1); // test by adding sleep(1)

    printf("child 2: before\n");

    Sem_post(&s2); // Signal child 1 that child 2 has printed "before".
    Sem_wait(&s1); // Wait for child 1 to print "before".

    printf("child 2: after\n");

    sleep(1); // test by adding sleep(1)

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t p1, p2;
    printf("parent: begin\n");

    // init semaphores here
    Sem_init(&s1, 0);
    Sem_init(&s2, 0);

    Pthread_create(&p1, NULL, child_1, NULL);
    Pthread_create(&p2, NULL, child_2, NULL);

    Pthread_join(p1, NULL);
    Pthread_join(p2, NULL);

    printf("parent: end\n");
    return 0;
}
