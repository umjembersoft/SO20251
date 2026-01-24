#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int global_var = 100;        // Data segment
int uninit_var;              // BSS segment

// ---------- Multiple Functions ----------
void func1() {}
void func2() {}
void func3() {}

// ---------- Recursive Function ----------
void recursive_func(int level) {
    int local_var = level;
    printf("   Level %d local_var address: %p\n",
           level, (void*)&local_var);

    if (level > 0)
        recursive_func(level - 1);
}

int main() {
    printf("=== ENHANCED MEMORY LAYOUT ===\n\n");
    printf("PID: %d\n\n", getpid());

    // ---------- Text Segment ----------
    printf("Text Segment:\n");
    printf("   main() address : %p\n", (void*)main);
    printf("   func1() address: %p\n", (void*)func1);
    printf("   func2() address: %p\n", (void*)func2);
    printf("   func3() address: %p\n\n", (void*)func3);

    // ---------- Data Segment ----------
    static int static_array[5] = {1,2,3,4,5};
    char *str_literal = "Hello Memory";

    printf("Data Segment:\n");
    printf("   global_var address     : %p\n", (void*)&global_var);
    printf("   static_array[0] address: %p\n", (void*)&static_array[0]);
    printf("   string literal address : %p\n\n", (void*)str_literal);

    // ---------- Heap ----------
    int *dynamic_array = (int*)malloc(5 * sizeof(int));
    int *block1 = (int*)malloc(100);
    int *block2 = (int*)malloc(200);

    printf("Heap Segment:\n");
    printf("   dynamic_array[0] address: %p\n", (void*)&dynamic_array[0]);
    printf("   malloc block 1 address  : %p\n", (void*)block1);
    printf("   malloc block 2 address  : %p\n\n", (void*)block2);

    // ---------- Stack ----------
    int stack_var = 50;
    printf("Stack Segment:\n");
    printf("   stack_var address: %p\n\n", (void*)&stack_var);

    // ---------- Recursive Stack ----------
    printf("Stack (Recursive Calls):\n");
    recursive_func(3);
    printf("\n");

    // ---------- Memory Map ----------
    printf("Tekan Enter untuk melihat /proc/[pid]/maps...");
    getchar();

    char cmd[100];
    sprintf(cmd, "cat /proc/%d/maps", getpid());
    system(cmd);

    // Cleanup
    free(dynamic_array);
    free(block1);
    free(block2);

    return 0;
}
