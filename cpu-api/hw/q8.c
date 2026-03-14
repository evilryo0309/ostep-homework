#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/// @brief Write a program that creates two children, and connects the standard output of one to the standard input of the other, using the pipe() system call.
/// @return 0 on success, 1 on failure
int main()
{
    int pipefd[2];

    // 1. Create the pipe before fork so that parent and children can share it
    // pipefd[0] is the read end, pipefd[1] is the write end
    if (pipe(pipefd) == -1)
    {
        perror("pipe failed");
        exit(1);
    }

    // 2. Create the first child process (responsible for writing data to the pipe)
    int rc1 = fork();
    if (rc1 < 0)
    {
        fprintf(stderr, "fork 1 failed\n");
        return 1;
    }
    else if (rc1 == 0)
    {
        // ---------------- Child Process 1 Block ----------------
        // Close unused read end (good practice)
        close(pipefd[0]);

        // Redirect standard output (STDOUT_FILENO) to the write end of the pipe (pipefd[1])
        dup2(pipefd[1], STDOUT_FILENO);

        // After redirection, the original pipefd[1] can be closed
        close(pipefd[1]);

        // Execute ls -l, which thinks it's printing to the screen, but actually all output goes into the pipe!
        char *args[] = {"ls", "-l", NULL};
        execvp("ls", args);

        perror("exec ls failed"); // If this line is reached, exec failed
        return 1;
    }

    // 3. Create the second child process (responsible for reading data from the pipe)
    int rc2 = fork();
    if (rc2 < 0)
    {
        fprintf(stderr, "fork 2 failed\n");
        return 1;
    }
    else if (rc2 == 0)
    {
        // ---------------- Child Process 2 Block ----------------
        // Close unused write end
        close(pipefd[1]);

        // Redirect standard input (STDIN_FILENO) to the read end of the pipe (pipefd[0])
        dup2(pipefd[0], STDIN_FILENO);

        // After redirection, the original pipefd[0] can be closed
        close(pipefd[0]);

        // Execute wc -l, which will obediently read data from the pipe until the other end is closed
        char *args[] = {"wc", "-l", NULL};
        execvp("wc", args);

        perror("exec wc failed");
        exit(1);
    }

    // ---------------- Parent Process Block ----------------
    // ⚠️ Extremely important: The parent process must close both ends of its pipe!
    // Because both children inherit the parent's FDs, if the parent doesn't close the write end,
    // child 2 (wc) will think "someone else might still write data" and will keep waiting for EOF (End of File).
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for both child processes to finish
    waitpid(rc1, NULL, 0);
    waitpid(rc2, NULL, 0);

    printf("Parent process finished, pipe execution complete.\n");

    return 0;
}