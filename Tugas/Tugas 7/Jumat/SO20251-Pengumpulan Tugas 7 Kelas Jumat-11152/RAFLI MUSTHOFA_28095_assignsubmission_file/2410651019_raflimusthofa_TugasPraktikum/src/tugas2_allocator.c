#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAX_BLOCKS 20
#define MEMORY_SIZE 1000

typedef struct {
    int id;
    int size;
    int start;
    int allocated;
} Block;

Block mem[MAX_BLOCKS];
int block_count;
int last_alloc = 0;   // untuk Next-Fit

void init_memory() {
    mem[0] = (Block){-1, MEMORY_SIZE, 0, 0};
    block_count = 1;
    last_alloc = 0;
}

void split_block(int idx, int pid, int size) {
    int old_size = mem[idx].size;
    int old_start = mem[idx].start;

    mem[idx] = (Block){pid, size, old_start, 1};

    if (old_size > size) {
        for (int j = block_count; j > idx + 1; j--)
            mem[j] = mem[j - 1];

        mem[idx + 1] = (Block){-1, old_size - size, old_start + size, 0};
        block_count++;
    }
}

int first_fit(int pid, int size) {
    for (int i = 0; i < block_count; i++)
        if (!mem[i].allocated && mem[i].size >= size) {
            split_block(i, pid, size);
            return 1;
        }
    return 0;
}

int best_fit(int pid, int size) {
    int best = -1, min = MEMORY_SIZE + 1;
    for (int i = 0; i < block_count; i++)
        if (!mem[i].allocated && mem[i].size >= size && mem[i].size < min) {
            min = mem[i].size;
            best = i;
        }
    if (best == -1) return 0;
    split_block(best, pid, size);
    return 1;
}

int worst_fit(int pid, int size) {
    int worst = -1, max = -1;
    for (int i = 0; i < block_count; i++)
        if (!mem[i].allocated && mem[i].size >= size && mem[i].size > max) {
            max = mem[i].size;
            worst = i;
        }
    if (worst == -1) return 0;
    split_block(worst, pid, size);
    return 1;
}

int next_fit(int pid, int size) {
    int count = 0;
    int i = last_alloc;

    while (count < block_count) {
        if (!mem[i].allocated && mem[i].size >= size) {
            split_block(i, pid, size);
            last_alloc = i;
            return 1;
        }
        i = (i + 1) % block_count;
        count++;
    }
    return 0;
}

void calculate_fragmentation() {
    int total_free = 0, holes = 0, largest = 0;
    for (int i = 0; i < block_count; i++) {
        if (!mem[i].allocated) {
            total_free += mem[i].size;
            holes++;
            if (mem[i].size > largest)
                largest = mem[i].size;
        }
    }
    float external_frag = (holes > 0)
        ? (float)(total_free - largest) / total_free * 100
        : 0;

    printf("Total Free Space     : %d\n", total_free);
    printf("Largest Free Block   : %d\n", largest);
    printf("Free Holes           : %d\n", holes);
    printf("External Fragmentation: %.2f%%\n", external_frag);
}

void benchmark(const char *name, int (*alloc)(int,int)) {
    struct timeval start, end;
    init_memory();

    gettimeofday(&start, NULL);

    alloc(1,100); alloc(2,200); alloc(3,300); alloc(4,250); alloc(5,150);
    alloc(6,180); alloc(7,220); alloc(8,100);

    gettimeofday(&end, NULL);

    long time = (end.tv_sec - start.tv_sec) * 1000000 +
                (end.tv_usec - start.tv_usec);

    printf("\n[%s]\nExecution Time: %ld µs\n", name, time);
    calculate_fragmentation();
}

int main() {
    benchmark("First-Fit", first_fit);
    benchmark("Best-Fit", best_fit);
    benchmark("Worst-Fit", worst_fit);
    benchmark("Next-Fit", next_fit);
    return 0;
}
