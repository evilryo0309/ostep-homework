#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>  // For using open() and its related flags (e.g., O_CREAT)
#include <string.h> // For using strlen()

/// @brief Write a program that opens a file (with the open() system call) and then calls fork() to create a new process.
/// Can both the child and parent access the file descriptor returned by open()?
/// What happens when they are writing to the file concurrently, i.e., at the same time?
/// @return 0 on success, 1 on failure
int main()
{
    // 1. Before fork, use open() to open (or create) a file
    // O_CREAT: Create the file if it does not exist
    // O_WRONLY: Write-only mode
    // O_TRUNC: Truncate the file if it exists
    // 0644: Set file permissions (owner can read/write, others read-only)
    int fd = open("q2_output.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
    {
        fprintf(stderr, "Failed to open file\n");
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
        printf("Child process PID: %d\n", getpid());

        // Child process can access the file descriptor and write to the file
        const char *child_msg = "This is the child process writing to the file.\n";
        write(fd, child_msg, strlen(child_msg));
    }
    else
    {
        // Parent process
        printf("Parent process PID: %d\n", getpid());

        // Parent process can also access the file descriptor and write to the file
        const char *parent_msg = "This is the parent process writing to the file.\n";
        write(fd, parent_msg, strlen(parent_msg));

        // Wait for the child process to finish (optional, but good practice)
        // wait(NULL);
    }

    return 0;
}