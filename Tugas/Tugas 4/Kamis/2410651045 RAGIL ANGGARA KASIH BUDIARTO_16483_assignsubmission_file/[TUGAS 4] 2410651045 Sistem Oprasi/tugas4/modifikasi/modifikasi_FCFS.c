#include <stdio.h>
#include <stdlib.h>

struct Process {
    char id[5];
    int arrival, burst;
    int waiting, turnaround, completion;
};

int main() {
    FILE *fin, *fout;
    fin = fopen("FCFS.txt", "r");
    fout = fopen("hasil_fcfs.txt", "w");

    if (fin == NULL) {
        printf("Gagal membuka file input.txt!\n");
        return 1;
    }

    int n;
    fscanf(fin, "%d", &n);

    struct Process p[n];

    for (int i = 0; i < n; i++) {
        fscanf(fin, "%s %d %d", p[i].id, &p[i].arrival, &p[i].burst);
    }

    fclose(fin);

    // FCFS Scheduling
    int current_time = 0;
    float total_wt = 0, total_tat = 0;

    printf("\n=== Hasil Penjadwalan FCFS ===\n");
    fprintf(fout, "=== Hasil Penjadwalan FCFS ===\n");

    printf("\nTabel Hasil:\n");
    printf("ID\tAT\tBT\tCT\tTAT\tWT\n");
    fprintf(fout, "\nID\tAT\tBT\tCT\tTAT\tWT\n");

    for (int i = 0; i < n; i++) {
        if (current_time < p[i].arrival)
            current_time = p[i].arrival;

        p[i].completion = current_time + p[i].burst;
        p[i].turnaround = p[i].completion - p[i].arrival;
        p[i].waiting = p[i].turnaround - p[i].burst;
        current_time = p[i].completion;

        total_wt += p[i].waiting;
        total_tat += p[i].turnaround;

        printf("%s\t%d\t%d\t%d\t%d\t%d\n",
               p[i].id, p[i].arrival, p[i].burst,
               p[i].completion, p[i].turnaround, p[i].waiting);
        fprintf(fout, "%s\t%d\t%d\t%d\t%d\t%d\n",
                p[i].id, p[i].arrival, p[i].burst,
                p[i].completion, p[i].turnaround, p[i].waiting);
    }

    float avg_wt = total_wt / n;
    float avg_tat = total_tat / n;

    printf("\nRata-rata Waiting Time: %.2f", avg_wt);
    printf("\nRata-rata Turnaround Time: %.2f\n", avg_tat);
    fprintf(fout, "\nRata-rata Waiting Time: %.2f", avg_wt);
    fprintf(fout, "\nRata-rata Turnaround Time: %.2f\n", avg_tat);

    // Gantt Chart lebih detail
    printf("\nGantt Chart:\n|");
    fprintf(fout, "\nGantt Chart:\n|");

    int start = 0;
    for (int i = 0; i < n; i++) {
        if (i == 0)
            start = p[i].arrival;
        printf(" %s (%d-%d) |", p[i].id, start, p[i].completion);
        fprintf(fout, " %s (%d-%d) |", p[i].id, start, p[i].completion);
        start = p[i].completion;
    }

    printf("\n\nHasil juga disimpan di file hasil_fcfs.txt\n");
    fprintf(fout, "\n");

    fclose(fout);
    return 0;
}
