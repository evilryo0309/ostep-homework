#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h> // For using wait() to wait for the child process to finish

int main(int argc, char *argv[])
{
    int rc = fork();

    if (rc < 0)
    {
        fprintf(stderr, "fork failed\n");
        return 1;
    }
    else if (rc == 0)
    {
        // ---------------- Child process ----------------
        printf("Child process (PID: %d): preparing to close STDOUT_FILENO...\n", getpid());

        // Close standard output (i.e., close FD 1)
        close(STDOUT_FILENO);

        // Try to call printf after closing STDOUT
        // We use a variable to capture the return value of printf to observe what happens
        int ret = printf("You will never see this line on the terminal!\n");

        // Since STDOUT is closed, we use STDERR (FD 2) to print the test result
        // This allows us to observe what happened with printf
        fprintf(stderr, "Child process STDERR report: printf return value was %d\n", ret);
    }
    else
    {
        // ---------------- Parent process ----------------
        wait(NULL);
        printf("Parent process finished.\n");
    }

    return 0;
}