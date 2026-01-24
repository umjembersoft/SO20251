#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>

#define SHM_SIZE 1024
#define KEY 1234

// Struktur untuk semaphore
void sem_lock(int semid) {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void sem_unlock(int semid) {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

int main() {
    int shmid = shmget(KEY, SHM_SIZE, 0666 | IPC_CREAT);
    int semid = semget(KEY, 1, 0666 | IPC_CREAT);

    // Inisialisasi semaphore ke 1
    semctl(semid, 0, SETVAL, 1);

    char *shared = (char*) shmat(shmid, NULL, 0);
    strcpy(shared, "=== CHAT ROOM ===\n");  // Inisialisasi pesan

    printf("Server berjalan... Shared Memory ID = %d\n", shmid);
    printf("Menunggu client (reader/writer)...\n");

    while (1) {
        sleep(1);
    }

    shmdt(shared);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    return 0;
}
