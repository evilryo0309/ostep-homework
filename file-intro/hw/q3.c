#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#define CHUNK_SIZE 1024

int main(int argc, char *argv[])
{
    // Expected input format: ./mytail -n file, e.g., ./mytail -10 test.txt
    if (argc != 3 || argv[1][0] != '-')
    {
        fprintf(stderr, "Usage: %s -<n> <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Parse the number of lines to print (skip the leading '-')
    int num_lines = atoi(argv[1] + 1);
    if (num_lines <= 0)
        return 0;

    // Open file
    int fd = open(argv[2], O_RDONLY);
    if (fd < 0)
    {
        perror("open failed");
        exit(EXIT_FAILURE);
    }

    // Get file size
    struct stat fileStat;
    if (fstat(fd, &fileStat) < 0)
    {
        perror("fstat failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    off_t file_size = fileStat.st_size;
    if (file_size == 0)
    {
        close(fd);
        return 0; // Empty file, exit directly
    }

    off_t current_pos = file_size;
    int lines_found = 0;
    char buffer[CHUNK_SIZE];

    // Core logic: read chunks backwards from the end of the file
    while (current_pos > 0 && lines_found <= num_lines)
    {
        off_t read_size = CHUNK_SIZE;
        // If remaining file size is less than a chunk, read only the remaining part
        if (current_pos < CHUNK_SIZE)
        {
            read_size = current_pos;
        }

        current_pos -= read_size;
        lseek(fd, current_pos, SEEK_SET);

        ssize_t bytes_read = read(fd, buffer, read_size);
        if (bytes_read <= 0)
            break;

        // Scan for newline characters backwards within the chunk
        for (int i = bytes_read - 1; i >= 0; i--)
        {
            // Ignore the case where the last character of the file is a newline
            if (buffer[i] == '\n' && (current_pos + i) != (file_size - 1))
            {
                lines_found++;
                // Found target line count, seek to the character right after this newline
                if (lines_found == num_lines)
                {
                    current_pos += (i + 1);
                    break;
                }
            }
        }

        if (lines_found == num_lines)
            break;
    }

    // If total lines in the file are less than num_lines, print from the beginning
    if (lines_found < num_lines)
    {
        current_pos = 0;
    }

    // Read and print from target position to the end of file
    lseek(fd, current_pos, SEEK_SET);
    ssize_t n;
    while ((n = read(fd, buffer, CHUNK_SIZE)) > 0)
    {
        write(STDOUT_FILENO, buffer, n); // STDOUT_FILENO is standard output (fd = 1)
    }

    close(fd);
    return 0;
}