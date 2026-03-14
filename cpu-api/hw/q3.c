#include <stdio.h>
#include <unistd.h>
#include <fcntl.h> // For using open() and its related flags (e.g., O_CREAT)

/// @brief Write another program using fork().
/// The child process should print “hello”; the parent process should print “goodbye”.
/// You should try to ensure that the child process always prints first;
/// can you do this without calling wait() in the parent?
/// @return 0 on success, 1 on failure
int main()
{
    int pipefd[2];
    char buffer[1];

    // 1. Create a pipe
    // pipefd[0] is the read end of the pipe, and pipefd[1] is the write end
    if (pipe(pipefd) == -1)
    {
        fprintf(stderr, "pipe failed\n");
        return 1;
    }

    // 2. Call fork()
    int rc = fork();
    if (rc < 0)
    {
        fprintf(stderr, "fork failed\n");
        return 1;
    }
    else if (rc == 0)
    {
        // Child process
        printf("hello\n");

        // Close the read end of the pipe in the child process
        close(pipefd[0]);

        // Write to the pipe to signal the parent process that the child has printed "hello"
        write(pipefd[1], "x", 1);

        // Close the write end of the pipe in the child process
        close(pipefd[1]);
    }
    else
    {
        // Parent process

        // Close the write end of the pipe in the parent process
        close(pipefd[1]);

        // Read from the pipe to wait for the signal from the child process
        read(pipefd[0], buffer, 1);

        printf("goodbye\n");

        // Close the read end of the pipe in the parent process
        close(pipefd[0]);
    }

    return 0;
}