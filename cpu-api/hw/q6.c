#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h> // For using wait() to wait for the child process to finish

/// @brief Write a slight modification of the previous program, this time using waitpid() instead of wait().
/// When would waitpid() be useful?
/// @return 0 on success, 1 on failure
int main()
{
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

        int wait_rc_child = waitpid(rc, NULL, 0);

        printf("Child process: wait() returned %d\n", wait_rc_child);

        if (wait_rc_child == -1)
        {
            fprintf(stderr, "wait failed in child process\n");
            return 1;
        }
    }
    else
    {
        // Parent process
        printf("Parent process PID: %d\n", getpid());

        int wait_rc_parent = wait(NULL);

        printf("Parent process: wait() returned %d\n", wait_rc_parent);
        if (wait_rc_parent == -1)
        {
            fprintf(stderr, "wait failed in parent process\n");
            return 1;
        }
    }

    return 0;
}