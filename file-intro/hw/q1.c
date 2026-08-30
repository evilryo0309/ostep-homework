#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    // Ensure the user provided a file or directory name
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file_or_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct stat fileStat;

    // Call stat() and store the result in fileStat
    if (stat(argv[1], &fileStat) < 0)
    {
        perror("stat failed");
        exit(EXIT_FAILURE);
    }

    // Print information retrieved by stat()
    printf("File: %s\n", argv[1]);
    printf("Size: %ld bytes\n", fileStat.st_size);
    printf("Blocks allocated: %ld\n", fileStat.st_blocks);
    printf("Reference (link) count: %ld\n", fileStat.st_nlink);
    printf("Inode number: %ld\n", fileStat.st_ino);

    // Determine the file type using st_mode
    if (S_ISDIR(fileStat.st_mode))
    {
        printf("Type: Directory\n");
    }
    else if (S_ISREG(fileStat.st_mode))
    {
        printf("Type: Regular File\n");
    }
    else
    {
        printf("Type: Other\n");
    }

    return 0;
}