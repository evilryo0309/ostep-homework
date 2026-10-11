#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 4096

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input_file> <output_csum_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *in_fp = fopen(argv[1], "rb");
    if (!in_fp)
    {
        perror("Failed to open input file");
        exit(EXIT_FAILURE);
    }

    FILE *out_fp = fopen(argv[2], "wb");
    if (!out_fp)
    {
        perror("Failed to open output checksum file");
        fclose(in_fp);
        exit(EXIT_FAILURE);
    }

    unsigned char buffer[BLOCK_SIZE];
    size_t bytes_read;
    size_t block_count = 0;

    while ((bytes_read = fread(buffer, 1, BLOCK_SIZE, in_fp)) > 0)
    {
        unsigned char csum = 0;
        for (size_t i = 0; i < bytes_read; i++)
        {
            csum ^= buffer[i];
        }
        if (fwrite(&csum, 1, 1, out_fp) != 1)
        {
            perror("Failed to write checksum");
            fclose(in_fp);
            fclose(out_fp);
            exit(EXIT_FAILURE);
        }
        block_count++;
    }

    fclose(in_fp);
    fclose(out_fp);

    printf("Successfully wrote %zu checksum block(s) to %s\n", block_count, argv[2]);
    return 0;
}