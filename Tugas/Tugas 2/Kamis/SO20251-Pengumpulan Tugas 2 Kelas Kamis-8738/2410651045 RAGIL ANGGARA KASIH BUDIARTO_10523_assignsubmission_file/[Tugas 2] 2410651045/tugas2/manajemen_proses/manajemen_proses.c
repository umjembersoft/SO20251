#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void sorting_task() {
    int arr[] = {9, 5, 1, 7, 3};
    int n = 5, temp;
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (arr[i] > arr[j]) {
                temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }
    printf("Child 1 (Sorting): hasil = ");
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");
}

void searching_task() {
    int arr[] = {1, 3, 5, 7, 9};
    int target = 7;
    int found = 0;
    for (int i = 0; i < 5; i++) {
        if (arr[i] == target) {
            found = 1;
            break;
        }
    }
    printf("Child 2 (Searching): elemen %d %s ditemukan\n",
           target, found ? "" : "tidak");
}

void calculation_task() {
    long sum = 0;
    for (int i = 1; i <= 1000000; i++)
        sum += i;
    printf("Child 3 (Calculation): hasil penjumlahan = %ld\n", sum);
}

int main() {
    pid_t pid[3];
    double start, end;

    printf("=== Sistem Manajemen Proses ===\n");

    for (int i = 0; i < 3; i++) {
        start = get_time();
        pid[i] = fork();

        if (pid[i] == 0) {
            printf("\n[Child Process %d] PID = %d, Parent = %d\n",
                   i + 1, getpid(), getppid());
            switch (i) {
                case 0: sorting_task(); break;
                case 1: searching_task(); break;
                case 2: calculation_task(); break;
            }
            end = get_time();
            printf("[Child %d] Execution time = %.6f detik\n",
                   i + 1, end - start);
            exit(0);
        }
    }

    // Parent menunggu semua child selesai
    for (int i = 0; i < 3; i++) {
        int status;
        waitpid(pid[i], &status, 0);
        printf("[Parent] Child %d (PID = %d) selesai dengan status %d\n",
               i + 1, pid[i], WEXITSTATUS(status));
    }

    printf("\nSemua proses selesai!\n");
    return 0;
}
