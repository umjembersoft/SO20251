#include <stdio.h>
#include <stdlib.h>

#define NUM_FRAMES 4
#define MAX_PAGES 100

typedef struct {
    int page;
    int time;
    int dirty;
} Frame;

typedef struct {
    int total_access;
    int page_fault;
    int page_hit;
    int disk_write;
} PageStats;

Frame frames[NUM_FRAMES];
PageStats stats;
int clock = 0;

// ---------- Utility ----------
void init_system() {
    for (int i = 0; i < NUM_FRAMES; i++) {
        frames[i].page = -1;
        frames[i].time = 0;
        frames[i].dirty = 0;
    }
    stats = (PageStats){0,0,0,0};
    clock = 0;
}

// ---------- FIFO ----------
int fifo_replace(int page, int write) {
    int victim = 0;
    for (int i = 1; i < NUM_FRAMES; i++)
        if (frames[i].time < frames[victim].time)
            victim = i;

    if (frames[victim].dirty)
        stats.disk_write++;

    frames[victim] = (Frame){page, clock++, write};
    return victim;
}

// ---------- LRU ----------
int lru_replace(int page, int write) {
    int victim = 0;
    for (int i = 1; i < NUM_FRAMES; i++)
        if (frames[i].time < frames[victim].time)
            victim = i;

    if (frames[victim].dirty)
        stats.disk_write++;

    frames[victim] = (Frame){page, clock++, write};
    return victim;
}

// ---------- Access ----------
void access_page(int page, int write, int policy) {
    stats.total_access++;

    for (int i = 0; i < NUM_FRAMES; i++) {
        if (frames[i].page == page) {
            stats.page_hit++;
            frames[i].time = clock++;
            if (write) frames[i].dirty = 1;
            return;
        }
    }

    // PAGE FAULT
    stats.page_fault++;

    for (int i = 0; i < NUM_FRAMES; i++) {
        if (frames[i].page == -1) {
            frames[i] = (Frame){page, clock++, write};
            return;
        }
    }

    if (policy == 0)
        fifo_replace(page, write);
    else
        lru_replace(page, write);
}

void run_test(int policy) {
    int ref[] = {7,0,1,2,0,3,0,4,2,3,0,3,2,1,2,0,1,7,0,1};
    int n = sizeof(ref)/sizeof(ref[0]);

    init_system();

    for (int i = 0; i < n; i++)
        access_page(ref[i], 0, policy);

    printf("\nPolicy: %s\n", policy == 0 ? "FIFO" : "LRU");
    printf("Total Access   : %d\n", stats.total_access);
    printf("Page Fault     : %d\n", stats.page_fault);
    printf("Page Hit       : %d\n", stats.page_hit);
    printf("Hit Ratio      : %.2f\n",
           (float)stats.page_hit / stats.total_access);
    printf("Disk Writes    : %d\n", stats.disk_write);
}

int main() {
    run_test(0); // FIFO
    run_test(1); // LRU
    return 0;
}
