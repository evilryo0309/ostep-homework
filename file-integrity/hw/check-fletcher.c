#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

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

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
    {
        for (size_t i = 0; i < bytes_read; i++)
        {
            a = (a + buffer[i]) % 255;
            b = (b + a) % 255;
        }
    }

    fclose(fp);

    printf("Fletcher Checksum (a, b): %u, %u (0x%02x, 0x%02x)\n", a, b, a, b);
    return 0;
}