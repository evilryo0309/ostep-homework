#include <stdio.h>

// 封裝 rdtsc 指令的函式
unsigned long long rdtsc(void)
{
    unsigned int lo, hi;
    // __asm__ __volatile__ 防止編譯器優化或移動這行程式碼的位置
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long long)hi << 32) | lo;
}

int main()
{
    unsigned long long start, end;

    // 實驗 A：連續讀取，看硬體計時器的最小解析度
    start = rdtsc();
    end = rdtsc();
    printf("Consecutive rdtsc cycle overhead: %llu cycles\n", end - start);

    // 實驗 B：量測工作量
    start = rdtsc();

    // 這裡放你要量測的程式碼
    for (volatile long long i = 0; i < 100000000; i++)
        ; // 這次我們只跑一億次，因為 rdtsc 非常靈敏

    end = rdtsc();

    unsigned long long elapsed_cycles = end - start;
    printf("Workload elapsed time: %llu cycles\n", elapsed_cycles);

    return 0;
}