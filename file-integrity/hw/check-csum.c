#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 4096

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <data_file> <csum_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *data_fp = fopen(argv[1], "rb");
    if (!data_fp)
    {
        perror("Failed to open data file");
        exit(EXIT_FAILURE);
    }

    FILE *csum_fp = fopen(argv[2], "rb");
    if (!csum_fp)
    {
        perror("Failed to open checksum file");
        fclose(data_fp);
        exit(EXIT_FAILURE);
    }

    unsigned char buffer[BLOCK_SIZE];
    unsigned char stored_csum;
    size_t bytes_read;
    size_t block_index = 0;
    int corruptions_found = 0;

    while ((bytes_read = fread(buffer, 1, BLOCK_SIZE, data_fp)) > 0)
    {
        if (fread(&stored_csum, 1, 1, csum_fp) != 1)
        {
            fprintf(stderr, "Error: Checksum file is shorter than expected at block %zu!\n", block_index);
            corruptions_found++;
            break;
        }

        unsigned char computed_csum = 0;
        for (size_t i = 0; i < bytes_read; i++)
        {
            computed_csum ^= buffer[i];
        }

        if (computed_csum != stored_csum)
        {
            printf("CORRUPTION DETECTED at block %zu! (computed: 0x%02x, stored: 0x%02x)\n",
                   block_index, computed_csum, stored_csum);
            corruptions_found++;
        }
        block_index++;
    }

    // 檢查 checksum 檔案是否還有多餘資料
    if (fread(&stored_csum, 1, 1, csum_fp) == 1)
    {
        fprintf(stderr, "Warning: Checksum file contains extra entries beyond data file size!\n");
        corruptions_found++;
    }

    fclose(data_fp);
    fclose(csum_fp);

    if (corruptions_found == 0)
    {
        printf("Verification OK: All %zu block(s) match perfectly.\n", block_index);
        return 0;
    }
    else
    {
        printf("Verification FAILED: File has been corrupted with %d mismatched block(s)!\n", corruptions_found);
        return 1;
    }
}