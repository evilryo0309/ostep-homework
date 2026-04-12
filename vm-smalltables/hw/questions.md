# Homework (Simulation)

This fun little homework tests if you understand how a multi-level page table works. And yes, there is some debate over the use of the term “fun” in the previous sentence. The program is called, perhaps unsurprisingly: paging-multilevel-translate.py; see the README for details.

## Questions

1. With a linear page table, you need a single register to locate the page table, assuming that hardware does the lookup upon a TLB miss. How many registers do you need to locate a two-level page table? A three-level table?

    > Regardless of whether the page table is two-level or three-level, you still only need one register (such as the Page Table Base Register, or CR3 in x86).
    >
    > This is because a multi-level page table operates like a tree structure. The single register only needs to hold the physical base address of the root, which is the top-level Page Directory. When traversing the page table, the hardware reads the Page Directory Entry (PDE) to find the physical address of the next level's page table, and so on. Since all subsequent pointers are stored in memory, one register is perfectly sufficient to locate the entire structure.

2. Use the simulator to perform translations given random seeds 0, 1, and 2, and check your answers using the -c flag. How many memory references are needed to perform each lookup?
3. Given your understanding of how cache memory works, how do you think memory references to the page table will behave in the cache? Will they lead to lots of cache hits (and thus fast accesses?) Or lots of misses (and thus slow accesses)?
