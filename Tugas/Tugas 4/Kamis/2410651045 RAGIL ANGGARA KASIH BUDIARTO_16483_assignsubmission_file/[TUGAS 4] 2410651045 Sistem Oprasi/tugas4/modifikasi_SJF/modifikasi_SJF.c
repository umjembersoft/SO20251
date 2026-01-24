#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct Process {
    char id[5];
    int arrival, burst;
    int waiting, turnaround, completion;
    bool done;
};

int main() {
    FILE *fin = fopen("SJF.txt", "r");
    FILE *fout = fopen("hasil_sjf.txt", "w");

    if (!fin) {
        printf("Gagal membuka file input.txt!\n");
        return 1;
    }

    int n;
    fscanf(fin, "%d", &n);
    struct Process p[n];

    for (int i = 0; i < n; i++) {
        fscanf(fin, "%s %d %d", p[i].id, &p[i].arrival, &p[i].burst);
        p[i].done = false;
    }
    fclose(fin);

    int completed = 0, current_time = 0;
    float total_wt = 0, total_tat = 0;

    printf("\n=== Hasil Penjadwalan SJF ===\n");
    fprintf(fout, "=== Hasil Penjadwalan SJF ===\n");

    printf("\nTabel Hasil:\nID\tAT\tBT\tCT\tTAT\tWT\n");
    fprintf(fout, "\nID\tAT\tBT\tCT\tTAT\tWT\n");

    printf("\nGantt Chart:\n|");
    fprintf(fout, "\nGantt Chart:\n|");

    while (completed < n) {
        int idx = -1, min_bt = 9999;

        for (int i = 0; i < n; i++) {
            if (!p[i].done && p[i].arrival <= current_time) {
                if (p[i].burst < min_bt) {
                    min_bt = p[i].burst;
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            current_time++;
            continue;
        }

        int start = current_time;
        current_time += p[idx].burst;
        p[idx].completion = current_time;
        p[idx].turnaround = p[idx].completion - p[idx].arrival;
        p[idx].waiting = p[idx].turnaround - p[idx].burst;
        p[idx].done = true;
        completed++;

        printf(" %s (%d-%d) |", p[idx].id, start, current_time);
        fprintf(fout, " %s (%d-%d) |", p[idx].id, start, current_time);

        total_wt += p[idx].waiting;
        total_tat += p[idx].turnaround;
    }

    printf("\n\nID\tAT\tBT\tCT\tTAT\tWT\n");
    fprintf(fout, "\n\nID\tAT\tBT\tCT\tTAT\tWT\n");

    for (int i = 0; i < n; i++) {
        printf("%s\t%d\t%d\t%d\t%d\t%d\n", p[i].id, p[i].arrival, p[i].burst,
               p[i].completion, p[i].turnaround, p[i].waiting);
        fprintf(fout, "%s\t%d\t%d\t%d\t%d\t%d\n", p[i].id, p[i].arrival, p[i].burst,
                p[i].completion, p[i].turnaround, p[i].waiting);
    }

    printf("\nRata-rata Waiting Time: %.2f", total_wt / n);
    printf("\nRata-rata Turnaround Time: %.2f\n", total_tat / n);
    fprintf(fout, "\nRata-rata Waiting Time: %.2f", total_wt / n);
    fprintf(fout, "\nRata-rata Turnaround Time: %.2f\n", total_tat / n);

    fclose(fout);
    printf("\n\nHasil juga disimpan di file hasil_sjf.txt\n");
    return 0;
}
