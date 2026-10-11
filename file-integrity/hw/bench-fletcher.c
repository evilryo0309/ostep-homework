#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        perror("fopen failed");
        exit(EXIT_FAILURE);
    }

    uint16_t a = 0;
    uint16_t b = 0;
    unsigned char buffer[BUFFER_SIZE];
    size_t bytes_read;
    size_t total_bytes = 0;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
    {
        total_bytes += bytes_read;
        for (size_t i = 0; i < bytes_read; i++)
        {
            a = (a + buffer[i]) % 255;
            b = (b + a) % 255;
        }
    }

    fclose(fp);
    gettimeofday(&end, NULL);

    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 +
                        (end.tv_usec - start.tv_usec) / 1000.0;

    printf("[Fletcher] Size: %zu bytes, Checksum: (0x%02x, 0x%02x), Time: %.3f ms\n",
           total_bytes, a, b, elapsed_ms);

    return 0;
}