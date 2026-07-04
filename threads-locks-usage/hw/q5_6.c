#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define NUM_BUCKETS 1024 // Number of buckets in the Hash Table

// --- Define Node ---
typedef struct __node_t
{
    int key;
    struct __node_t *next;
} node_t;

// --- Define Hash Table ---
typedef struct __hash_t
{
    node_t *buckets[NUM_BUCKETS];
    pthread_mutex_t global_lock;               // Used for Q5: global single lock
    pthread_mutex_t bucket_locks[NUM_BUCKETS]; // Used for Q6: one lock per bucket
} hash_t;

hash_t my_hash;
int use_bucket_locks = 0;    // 0 = single lock (Q5), 1 = bucket locks (Q6)
int ops_per_thread = 100000; // Number of insert operations per thread

// Initialize Hash Table
void Hash_Init(hash_t *H)
{
    pthread_mutex_init(&H->global_lock, NULL);
    for (int i = 0; i < NUM_BUCKETS; i++)
    {
        H->buckets[i] = NULL;
        pthread_mutex_init(&H->bucket_locks[i], NULL);
    }
}

// Insert operation
void Hash_Insert(hash_t *H, int key)
{
    int bucket = key % NUM_BUCKETS; // Simple hash function

    node_t *new_node = malloc(sizeof(node_t));
    if (new_node == NULL)
    {
        perror("malloc");
        return;
    }
    new_node->key = key;

    if (use_bucket_locks)
    {
        // Mode 1: Lock only the target bucket
        pthread_mutex_lock(&H->bucket_locks[bucket]);
        new_node->next = H->buckets[bucket];
        H->buckets[bucket] = new_node;
        pthread_mutex_unlock(&H->bucket_locks[bucket]);
    }
    else
    {
        // Mode 0: Lock the entire hash table
        pthread_mutex_lock(&H->global_lock);
        new_node->next = H->buckets[bucket];
        H->buckets[bucket] = new_node;
        pthread_mutex_unlock(&H->global_lock);
    }
}

// Worker thread
void *worker(void *arg)
{
    // Use different seeds to ensure random key distribution across threads
    unsigned int seed = (unsigned int)(unsigned long)pthread_self();

    for (int i = 0; i < ops_per_thread; i++)
    {
        // Generate random key
        int key = rand_r(&seed) % (NUM_BUCKETS * 10);
        Hash_Insert(&my_hash, key);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int num_threads = 1;

    // Parse arguments: ./q5_6 <threads> <mode: 0 or 1>
    if (argc > 1)
        num_threads = atoi(argv[1]);
    if (argc > 2)
        use_bucket_locks = atoi(argv[2]);

    Hash_Init(&my_hash);

    pthread_t threads[num_threads];
    struct timeval start, end;

    gettimeofday(&start, NULL);

    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&threads[i], NULL, worker, NULL);
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end, NULL);
    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

    printf("Threads: %d\n", num_threads);
    printf("Mode: %s\n", use_bucket_locks ? "Bucket Locks (Q6)" : "Single Global Lock (Q5)");
    printf("Total Operations: %d\n", num_threads * ops_per_thread);
    printf("Elapsed Time: %.3f ms\n", elapsed_ms);

    return 0;
}