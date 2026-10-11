#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#define BUFFER_SIZE 4096
#define CRC16_POLYNOMIAL 0x1021

static uint16_t crc_table[256];

// 初始化 CRC-16 查表 (CCITT: 0x1021)
void init_crc16_table(void)
{
    for (int i = 0; i < 256; i++)
    {
        uint16_t curr = (uint16_t)(i << 8);
        for (int j = 0; j < 8; j++)
        {
            if (curr & 0x8000)
            {
                curr = (curr << 1) ^ CRC16_POLYNOMIAL;
            }
            else
            {
                curr = (curr << 1);
            }
        }
        crc_table[i] = curr;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init_crc16_table();

    struct timeval start, end;
    gettimeofday(&start, NULL);

    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        perror("fopen failed");
        exit(EXIT_FAILURE);
    }

    uint16_t crc = 0x0000;
    unsigned char buffer[BUFFER_SIZE];
    size_t bytes_read;
    size_t total_bytes = 0;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
    {
        total_bytes += bytes_read;
        for (size_t i = 0; i < bytes_read; i++)
        {
            uint8_t index = (uint8_t)((crc >> 8) ^ buffer[i]);
            crc = (uint16_t)((crc << 8) ^ crc_table[index]);
        }
    }

    fclose(fp);
    gettimeofday(&end, NULL);

    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 +
                        (end.tv_usec - start.tv_usec) / 1000.0;

    printf("[CRC-16] Size: %zu bytes, Checksum: 0x%04x, Time: %.3f ms\n",
           total_bytes, crc, elapsed_ms);

    return 0;
}