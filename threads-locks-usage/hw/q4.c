#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

// --- Define Node structure ---
typedef struct __node_t
{
    int key;
    struct __node_t *next;
    pthread_mutex_t lock; // Lock dedicated to this Node (for Hand-over-hand)
} node_t;

// --- Define List structure ---
typedef struct __list_t
{
    node_t *head;
    pthread_mutex_t lock; // Global Lock (for Coarse-grained)
} list_t;

list_t my_list;
int use_hand_over_hand = 0; // 0 = Coarse Lock, 1 = Hand-over-hand Lock
int list_size = 10000;      // Initial length of the List
int lookup_ops = 10000;     // Number of search operations each Thread performs

// --- Basic operations ---
void List_Init(list_t *L)
{
    L->head = NULL;
    pthread_mutex_init(&L->lock, NULL);
}

// Simple insertion (always insert at Head, used for initializing test data)
void List_Insert(list_t *L, int key)
{
    node_t *new_node = malloc(sizeof(node_t));
    if (new_node == NULL)
    {
        perror("malloc");
        return;
    }
    new_node->key = key;
    pthread_mutex_init(&new_node->lock, NULL);

    // Protect insertion with global lock during initialization phase
    pthread_mutex_lock(&L->lock);
    new_node->next = L->head;
    L->head = new_node;
    pthread_mutex_unlock(&L->lock);
}

// --- Traverse search: Single global lock mode ---
int List_Lookup_Coarse(list_t *L, int key)
{
    int rv = -1;
    pthread_mutex_lock(&L->lock);
    node_t *curr = L->head;
    while (curr)
    {
        if (curr->key == key)
        {
            rv = 0; // Found
            break;
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&L->lock);
    return rv;
}

// --- Traverse search: Hand-over-hand mode ---
int List_Lookup_HandOverHand(list_t *L, int key)
{
    int rv = -1;

    // First lock the global lock to safely get the Head
    pthread_mutex_lock(&L->lock);
    node_t *curr = L->head;
    if (curr == NULL)
    {
        pthread_mutex_unlock(&L->lock);
        return rv;
    }

    // After locking the first Node, we can release the global lock
    pthread_mutex_lock(&curr->lock);
    pthread_mutex_unlock(&L->lock);

    while (curr)
    {
        if (curr->key == key)
        {
            rv = 0;
            pthread_mutex_unlock(&curr->lock);
            return rv;
        }

        node_t *next = curr->next;
        if (next != NULL)
        {
            // Hand-over-hand core logic: lock next before unlocking current
            pthread_mutex_lock(&next->lock);
        }
        pthread_mutex_unlock(&curr->lock);
        curr = next;
    }
    return rv;
}

// --- Worker Thread ---
void *worker(void *arg)
{
    for (int i = 0; i < lookup_ops; i++)
    {
        // Randomly pick a number to ensure Thread traverses the List
        int target = rand() % list_size;

        if (use_hand_over_hand)
        {
            List_Lookup_HandOverHand(&my_list, target);
        }
        else
        {
            List_Lookup_Coarse(&my_list, target);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int num_threads = 1;

    // Parse arguments: ./q4 <threads> <mode: 0 or 1>
    if (argc > 1)
        num_threads = atoi(argv[1]);
    if (argc > 2)
        use_hand_over_hand = atoi(argv[2]);

    List_Init(&my_list);

    // Initialize List with data from 0 to list_size-1
    for (int i = 0; i < list_size; i++)
    {
        List_Insert(&my_list, i);
    }

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
    printf("Mode: %s\n", use_hand_over_hand ? "Hand-over-hand" : "Coarse Lock");
    printf("Elapsed Time: %.3f ms\n", elapsed_ms);

    return 0;
}