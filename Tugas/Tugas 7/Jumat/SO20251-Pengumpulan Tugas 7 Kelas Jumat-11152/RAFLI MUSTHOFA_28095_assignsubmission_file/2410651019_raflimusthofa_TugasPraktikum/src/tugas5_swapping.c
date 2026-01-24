#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PROCESS 10
#define PHYSICAL_MEMORY 1024   // MB

typedef struct {
    int pid;
    int size;
    int priority;
    int in_memory;
    int swap_count;
    time_t last_access;
} Process;

Process processes[MAX_PROCESS];
int used_memory = 0;

// ---------- Initialization ----------
void init_processes() {
    srand(time(NULL));
    used_memory = 0;

    for (int i = 0; i < MAX_PROCESS; i++) {
        processes[i].pid = i + 1;
        processes[i].size = 50 + rand() % 151; // 50–200 MB
        processes[i].priority = 1 + rand() % 5;
        processes[i].in_memory = 0;
        processes[i].swap_count = 0;
        processes[i].last_access = time(NULL);
    }
}

// ---------- Swap Operations ----------
void swap_out(int idx) {
    processes[idx].in_memory = 0;
    used_memory -= processes[idx].size;
    processes[idx].swap_count++;
}

void swap_in(int idx) {
    processes[idx].in_memory = 1;
    used_memory += processes[idx].size;
    processes[idx].last_access = time(NULL);
}

// ---------- Victim Selection ----------
int select_fifo() {
    int victim = -1;
    time_t oldest = time(NULL);

    for (int i = 0; i < MAX_PROCESS; i++) {
        if (processes[i].in_memory &&
            processes[i].last_access <= oldest) {
            oldest = processes[i].last_access;
            victim = i;
        }
    }
    return victim;
}

int select_lru() {
    int victim = -1;
    time_t least = time(NULL);

    for (int i = 0; i < MAX_PROCESS; i++) {
        if (processes[i].in_memory &&
            processes[i].last_access <= least) {
            least = processes[i].last_access;
            victim = i;
        }
    }
    return victim;
}

int select_priority() {
    int victim = -1;
    int lowest = 999;

    for (int i = 0; i < MAX_PROCESS; i++) {
        if (processes[i].in_memory &&
            processes[i].priority < lowest) {
            lowest = processes[i].priority;
            victim = i;
        }
    }
    return victim;
}

// ---------- Simulation ----------
void simulate(int strategy) {
    init_processes();
    int swap_in_count = 0, swap_out_count = 0;

    for (int t = 0; t < 100; t++) {
        int p = rand() % MAX_PROCESS;

        if (!processes[p].in_memory) {
            while (used_memory + processes[p].size > PHYSICAL_MEMORY) {
                int victim;
                if (strategy == 0) victim = select_fifo();
                else if (strategy == 1) victim = select_lru();
                else victim = select_priority();

                swap_out(victim);
                swap_out_count++;
            }
            swap_in(p);
            swap_in_count++;
        }
        processes[p].last_access = time(NULL);
    }

    printf("\nStrategy: %s\n",
           strategy == 0 ? "FIFO" :
           strategy == 1 ? "LRU" : "PRIORITY");

    printf("Swap In Count : %d\n", swap_in_count);
    printf("Swap Out Count: %d\n", swap_out_count);
}

// ---------- Main ----------
int main() {
    simulate(0); // FIFO
    simulate(1); // LRU
    simulate(2); // Priority
    return 0;
}
