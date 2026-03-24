#include <stdio.h>
#include <stdlib.h>

// Define Vector structure
typedef struct
{
    int *data;    // Pointer to the memory block that stores actual data
    int size;     // Current number of elements stored
    int capacity; // Current allocated memory capacity
} Vector;

// Initialize Vector
void vector_init(Vector *v)
{
    v->capacity = 2; // Start with a small capacity to test realloc
    v->size = 0;
    v->data = (int *)malloc(v->capacity * sizeof(int));
    if (v->data == NULL)
    {
        printf("Memory allocation failed!\n");
        exit(1);
    }
}

// Add element to the end of Vector
void vector_add(Vector *v, int element)
{
    // If capacity is full, double the capacity
    if (v->size == v->capacity)
    {
        v->capacity *= 2;
        // realloc automatically handles memory relocation and old memory deallocation
        int *temp = (int *)realloc(v->data, v->capacity * sizeof(int));
        if (temp == NULL)
        {
            printf("Memory reallocation failed!\n");
            free(v->data); // realloc failure does not free old memory, must handle manually
            exit(1);
        }
        v->data = temp;
        printf("Reallocated memory! New capacity: %d\n", v->capacity);
    }

    v->data[v->size] = element;
    v->size++;
}

// Free Vector
void vector_free(Vector *v)
{
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

int main()
{
    Vector my_vec;
    vector_init(&my_vec);

    // Insert 10 elements to trigger multiple realloc calls
    for (int i = 0; i < 10; i++)
    {
        vector_add(&my_vec, i * 10);
        printf("Added %d, size: %d, capacity: %d\n", i * 10, my_vec.size, my_vec.capacity);
    }

    // Ensure no memory leak
    vector_free(&my_vec);

    return 0;
}