#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* =========================================================
   TWO-LEVEL PAGE TABLE (KODE ASLI ANDA)
   ========================================================= */

#define PAGE_SIZE 4096
#define VIRTUAL_ADDR_BITS 32
#define OFFSET_BITS 12

#define OUTER_PAGE_BITS 10
#define INNER_PAGE_BITS 10

typedef struct {
    int frame_number;
    int valid;
} InnerPageTableEntry;

typedef struct {
    InnerPageTableEntry *inner_table;
    int valid;
} OuterPageTableEntry;

OuterPageTableEntry *outer_page_table;
int num_outer_entries;
int num_inner_entries;

void initialize_two_level_paging() {
    num_outer_entries = 1 << OUTER_PAGE_BITS;
    num_inner_entries = 1 << INNER_PAGE_BITS;

    outer_page_table = calloc(num_outer_entries, sizeof(OuterPageTableEntry));

    printf("=== TWO-LEVEL PAGE TABLE INITIALIZED ===\n");
}

void parse_virtual_address(unsigned int vaddr,
                           int *outer, int *inner, int *offset) {
    *offset = vaddr & ((1 << OFFSET_BITS) - 1);
    *inner = (vaddr >> OFFSET_BITS) & ((1 << INNER_PAGE_BITS) - 1);
    *outer = (vaddr >> (OFFSET_BITS + INNER_PAGE_BITS)) &
             ((1 << OUTER_PAGE_BITS) - 1);
}

void allocate_page_two_level(unsigned int vaddr, int frame) {
    int o, i, off;
    parse_virtual_address(vaddr, &o, &i, &off);

    if (!outer_page_table[o].valid) {
        outer_page_table[o].inner_table =
            calloc(num_inner_entries, sizeof(InnerPageTableEntry));
        outer_page_table[o].valid = 1;
    }

    outer_page_table[o].inner_table[i].frame_number = frame;
    outer_page_table[o].inner_table[i].valid = 1;

    printf("2-Level: VA 0x%X -> Frame %d\n", vaddr, frame);
}

int translate_two_level(unsigned int vaddr) {
    int o, i, off;
    parse_virtual_address(vaddr, &o, &i, &off);

    if (!outer_page_table[o].valid ||
        !outer_page_table[o].inner_table[i].valid) {
        printf("2-Level PAGE FAULT\n");
        return -1;
    }

    int frame = outer_page_table[o].inner_table[i].frame_number;
    int pa = frame * PAGE_SIZE + off;

    printf("2-Level Translation: VA 0x%X -> PA 0x%X\n", vaddr, pa);
    return pa;
}

/* =========================================================
   THREE-LEVEL PAGE TABLE (TAMBAHAN WAJIB)
   ========================================================= */

#define L1_BITS 16
#define L2_BITS 8
#define L3_BITS 8
#define OFFSET3_BITS 12

typedef struct {
    int frame;
    int valid;
    int r, w, x;
} L3_Entry;

typedef struct {
    L3_Entry *entries;
    int valid;
} L2_Entry;

typedef struct {
    L2_Entry *entries;
    int valid;
} L1_Entry;

L1_Entry *l1_table;

void init_3level() {
    l1_table = calloc(1 << L1_BITS, sizeof(L1_Entry));
    printf("\n=== THREE-LEVEL PAGE TABLE INITIALIZED ===\n");
}

void allocate_3level(unsigned long vaddr, int frame) {
    int l1 = (vaddr >> (L2_BITS + L3_BITS + OFFSET3_BITS)) & ((1 << L1_BITS) - 1);
    int l2 = (vaddr >> (L3_BITS + OFFSET3_BITS)) & ((1 << L2_BITS) - 1);
    int l3 = (vaddr >> OFFSET3_BITS) & ((1 << L3_BITS) - 1);

    if (!l1_table[l1].valid) {
        l1_table[l1].entries = calloc(1 << L2_BITS, sizeof(L2_Entry));
        l1_table[l1].valid = 1;
    }

    if (!l1_table[l1].entries[l2].valid) {
        l1_table[l1].entries[l2].entries =
            calloc(1 << L3_BITS, sizeof(L3_Entry));
        l1_table[l1].entries[l2].valid = 1;
    }

    L3_Entry *e = &l1_table[l1].entries[l2].entries[l3];
    e->frame = frame;
    e->valid = 1;
    e->r = e->w = e->x = 1;

    printf("3-Level: VA 0x%lx -> Frame %d\n", vaddr, frame);
}

int translate_3level(unsigned long vaddr) {
    int offset = vaddr & ((1 << OFFSET3_BITS) - 1);
    int l1 = (vaddr >> (L2_BITS + L3_BITS + OFFSET3_BITS)) & ((1 << L1_BITS) - 1);
    int l2 = (vaddr >> (L3_BITS + OFFSET3_BITS)) & ((1 << L2_BITS) - 1);
    int l3 = (vaddr >> OFFSET3_BITS) & ((1 << L3_BITS) - 1);

    if (!l1_table[l1].valid ||
        !l1_table[l1].entries[l2].valid ||
        !l1_table[l1].entries[l2].entries[l3].valid) {
        printf("3-Level PAGE FAULT\n");
        return -1;
    }

    int frame = l1_table[l1].entries[l2].entries[l3].frame;
    int pa = (frame << OFFSET3_BITS) | offset;

    printf("3-Level Translation: VA 0x%lx -> PA 0x%x\n", vaddr, pa);
    return pa;
}

/* =========================================================
   INVERTED PAGE TABLE (TAMBAHAN WAJIB)
   ========================================================= */

#define NUM_FRAMES 64

typedef struct {
    int pid;
    int page;
    int valid;
} InvertedEntry;

InvertedEntry inverted_table[NUM_FRAMES];

int search_inverted(int pid, int page) {
    for (int i = 0; i < NUM_FRAMES; i++)
        if (inverted_table[i].valid &&
            inverted_table[i].pid == pid &&
            inverted_table[i].page == page)
            return i;
    return -1;
}

void insert_inverted(int pid, int page, int frame) {
    inverted_table[frame] = (InvertedEntry){pid, page, 1};
    printf("Inverted: PID %d Page %d -> Frame %d\n", pid, page, frame);
}

/* =========================================================
   DEMO
   ========================================================= */

int main() {
    initialize_two_level_paging();
    init_3level();

    printf("\n--- TWO LEVEL DEMO ---\n");
    allocate_page_two_level(0x00001000, 5);
    translate_two_level(0x00001234);

    printf("\n--- THREE LEVEL DEMO ---\n");
    allocate_3level(0x123456789ABC, 12);
    translate_3level(0x123456789ABC);

    printf("\n--- INVERTED PAGE TABLE DEMO ---\n");
    insert_inverted(1, 10, 3);
    printf("Search PID 1 Page 10 -> Frame %d\n",
           search_inverted(1, 10));

    return 0;
}
