#include <stdio.h>
#include <unistd.h>

/// @brief Write a program that calls fork().
/// Before calling fork(), have the main process access a variable (e.g., x) and set its value to something (e.g., 100).
/// What value is the variable in the child process?
/// What happens to the variable when both the child and parent change the value of x?
/// @return 0 on success, 1 on failure
int main()
{
    int x = 100;

    int rc = fork();
    if (rc < 0)
    {
        fprintf(stderr, "fork failed\n");
        return 1;
    }
    else if (rc == 0)
    {
        // Child process
        printf("Child process PID: %d\n", getpid());
        printf("Child process: x = %d\n", x);
        x += 50; // Change the value of x in the child process
    }
    else
    {
        // Parent process
        printf("Parent process PID: %d\n", getpid());
        printf("Parent process: x = %d\n", x);
        x += 20; // Change the value of x in the parent process
    }

    printf("Final value of x in process %d: %d\n", getpid(), x);

    return 0;
}