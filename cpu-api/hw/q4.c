#define _GNU_SOURCE // For using execvpe() which is a GNU extension
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h> // For using wait() to wait for the child process to finish

/// @brief Write a program that calls fork() and then calls some form of exec() to run the program /bin/ls.
/// See if you can try all of the variants of exec(), including (on Linux) execl(), execle(), execlp(), execv(), execvp(), and execvpe().
/// Why do you think there are so many variants of the same basic call?
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

        // 1. execl (l: arguments are listed with commas, no p: needs absolute path)
        // Test feature: Add "-l" and "/" arguments to list detailed information of the system root directory
        // execl("/bin/ls", "ls", "-l", "/", NULL);

        // =====================================================================
        // 2. execle (l: arguments are listed with commas, no p: needs absolute path, e: custom environment variables)
        // Test feature: We pass in the timezone of New York, USA. You will find that the file times printed by ls become US time!
        // char *env_le[] = {"TZ=America/New_York", NULL};
        // execle("/bin/ls", "ls", "-l", "/", NULL, env_le);

        // =====================================================================
        // 3. execlp (l: arguments are listed with commas, p: finds path automatically via $PATH)
        // Test feature: No need to write /bin/ls, just write ls. We add "-a" to show hidden files.
        // execlp("ls", "ls", "-l", "-a", NULL);

        // =====================================================================
        // 4. execv (v: array arguments, no p: needs absolute path)
        // Test feature: We let it list the /tmp directory, and add "-1" (number 1) to force single-line display for files
        // char *args_v[] = {"ls", "-1", "/tmp", NULL};
        // execv("/bin/ls", args_v);

        // =====================================================================
        // 5. execvp (v: array arguments, p: finds path automatically via $PATH)
        // ⭐️ This is the most commonly used version in practical Shell development!
        // Test feature: We add "-h" (human-readable file size format, such as K, M, G) and "--color=auto" to enable colored output
        char *args_vp[] = {"ls", "-l", "-h", "--color=auto", NULL};
        execvp("ls", args_vp);

        // =====================================================================
        // 6. execvpe (v: array arguments, p: finds path automatically via $PATH, e: custom environment variables)
        // Test feature: array arguments + automatic path + London, UK timezone
        // char *args_vpe[] = {"ls", "-l", "--color=auto", NULL};
        // char *env_vpe[] = {"TZ=Europe/London", NULL};
        // execvpe("ls", args_vpe, env_vpe);

        // =====================================================================

        // If we see this line, it means exec failed.
        // perror can help us print more detailed error reasons (such as file not found or insufficient permissions)
        perror("exec failed");
        return 1;
    }
    else
    {
        // Parent process
        printf("Parent process PID: %d\n", getpid());

        // Wait for the child process to finish (optional, but good practice)
        wait(NULL);
    }

    return 0;
}