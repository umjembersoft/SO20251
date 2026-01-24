#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

pid_t child[3];

double time_diff(struct timeval a, struct timeval b) {
    return (b.tv_sec - a.tv_sec) + (b.tv_usec - a.tv_usec) / 1e6;
}

int main() {
    printf("\n=== Tugas 1: Process Manager (NIM GANJIL – Shared Memory) ===\n");
    printf("[Parent] Program dimulai\n\n");

    int pipes[2][2];
    for (int i = 0; i < 2; i++) pipe(pipes[i]); // hanya 2 pipe (child 1 & 2)

    // Untuk shared memory untuk child 3
    key_t key = 1234;
    int shmid = shmget(key, 256, IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }

    struct timeval start, end;
    int status; char buf[512];

    // Untukk Child 2
    gettimeofday(&start, NULL);
    if ((child[0] = fork()) == 0) {
        close(pipes[0][0]);
        int n; long long fakt = 1;
        printf("[Child 1] Masukkan angka untuk faktorial: ");
        scanf("%d", &n);
        for (int i = 1; i <= n; i++) fakt *= i;
        sprintf(buf, "%lld", fakt);
        write(pipes[0][1], buf, strlen(buf) + 1);
        exit(0);
    }
    waitpid(child[0], &status, 0);
    gettimeofday(&end, NULL);
    close(pipes[0][1]); read(pipes[0][0], buf, sizeof(buf));
    printf("\n[Parent] Child 1 selesai (PID=%d) | Exit=%d | Time=%.4f detik\n",
           child[0], WEXITSTATUS(status), time_diff(start, end));
    printf("[Parent] Hasil faktorial = %s\n\n", buf);

    // Untuk Child 2
    gettimeofday(&start, NULL);
    if ((child[1] = fork()) == 0) {
        close(pipes[1][0]);
        int batas; char hasil[512] = "";
        printf("[Child 2] Masukkan batas bilangan prima: ");
        scanf("%d", &batas);
        for (int i = 2; i <= batas; i++) {
            int p = 1;
            for (int j = 2; j * j <= i; j++)
                if (i % j == 0) p = 0;
            if (p) sprintf(hasil + strlen(hasil), "%d ", i);
        }
        write(pipes[1][1], hasil, strlen(hasil) + 1);
        exit(0);
    }
    waitpid(child[1], &status, 0);
    gettimeofday(&end, NULL);
    close(pipes[1][1]); read(pipes[1][0], buf, sizeof(buf));
    printf("[Parent] Child 2 selesai (PID=%d) | Exit=%d | Time=%.4f detik\n",
           child[1], WEXITSTATUS(status), time_diff(start, end));
    printf("[Parent] Bilangan prima = %s\n\n", buf);

    // Untuk Child 3 Sekaligus Shared Memory
    gettimeofday(&start, NULL);
    if ((child[2] = fork()) == 0) {
        char *shm = (char *)shmat(shmid, NULL, 0);
        char str[128];
        printf("[Child 3] Masukkan string: ");
        scanf("%s", str);
        int len = strlen(str);
        for (int i = 0; i < len / 2; i++) {
            char tmp = str[i]; str[i] = str[len - 1 - i]; str[len - 1 - i] = tmp;
        }
        strcpy(shm, str); // ini untuk kirim hasil ke shared memory
        shmdt(shm);
        exit(0);
    }
    waitpid(child[2], &status, 0);
    gettimeofday(&end, NULL);

    // Untuk Parent membaca dari shared memory
    char *shm = (char *)shmat(shmid, NULL, 0);
    printf("[Parent] Child 3 selesai (PID=%d) | Exit=%d | Time=%.4f detik\n",
           child[2], WEXITSTATUS(status), time_diff(start, end));
    printf("[Parent] String terbalik = %s\n", shm);
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL); // Untuk Menghapus Shared Memory

    printf("\n=== Semua Proses Telah Diselesaikan ===\n");
    return 0;
}
