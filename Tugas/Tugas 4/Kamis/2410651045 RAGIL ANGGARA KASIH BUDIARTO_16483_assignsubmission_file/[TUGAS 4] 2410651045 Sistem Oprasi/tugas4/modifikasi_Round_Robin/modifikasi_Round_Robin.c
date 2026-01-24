#include <stdio.h>
#include <stdlib.h>

struct Process {
    char id[5];
    int arrival, burst, remaining;
    int waiting, turnaround, completion;
};

int main() {
    FILE *fin = fopen("Round_Robin.txt", "r");
    FILE *fout = fopen("hasil_rr.txt", "w");

    if (!fin) {
        printf("Gagal membuka file input.txt!\n");
        return 1;
    }

    int n, quantum = 16;
    fscanf(fin, "%d", &n);

    struct Process p[n];
    for (int i = 0; i < n; i++) {
        fscanf(fin, "%s %d %d", p[i].id, &p[i].arrival, &p[i].burst);
        p[i].remaining = p[i].burst;
    }
    fclose(fin);

    int completed = 0, current_time = 0;
    float total_wt = 0, total_tat = 0;
    int done[n];
    for (int i = 0; i < n; i++) done[i] = 0;

    printf("\n=== Hasil Penjadwalan Round Robin (Q=16) ===\n");
    fprintf(fout, "=== Hasil Penjadwalan Round Robin (Q=16) ===\n");

    printf("\nGantt Chart:\n|");
    fprintf(fout, "\nGantt Chart:\n|");

    while (completed < n) {
        int idle = 1;
        for (int i = 0; i < n; i++) {
            if (p[i].arrival <= current_time && p[i].remaining > 0) {
                idle = 0;
                int start = current_time;
                if (p[i].remaining <= quantum) {
                    current_time += p[i].remaining;
                    p[i].remaining = 0;
                    p[i].completion = current_time;
                    completed++;
                } else {
                    current_time += quantum;
                    p[i].remaining -= quantum;
                }

                printf(" %s (%d-%d) |", p[i].id, start, current_time);
                fprintf(fout, " %s (%d-%d) |", p[i].id, start, current_time);
            }
        }

        if (idle) current_time++;
    }

    printf("\n\nID\tAT\tBT\tCT\tTAT\tWT\n");
    fprintf(fout, "\n\nID\tAT\tBT\tCT\tTAT\tWT\n");

    for (int i = 0; i < n; i++) {
        p[i].turnaround = p[i].completion - p[i].arrival;
        p[i].waiting = p[i].turnaround - p[i].burst;
        total_tat += p[i].turnaround;
        total_wt += p[i].waiting;

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
    printf("\n\nHasil juga disimpan di file hasil_rr.txt\n");
    return 0;
}
