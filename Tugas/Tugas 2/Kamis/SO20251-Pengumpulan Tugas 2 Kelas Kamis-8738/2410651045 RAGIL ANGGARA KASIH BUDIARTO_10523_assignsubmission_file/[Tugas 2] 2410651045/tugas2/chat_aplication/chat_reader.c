#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>

#define SHM_SIZE 1024
#define KEY 1234

void sem_lock(int semid) {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void sem_unlock(int semid) {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

int main() {
    int shmid = shmget(KEY, SHM_SIZE, 0666);
    int semid = semget(KEY, 1, 0666);

    char *shared = (char*) shmat(shmid, NULL, 0);

    while (1) {
        sem_lock(semid);
        system("clear");
        printf("%s", shared);
        sem_unlock(semid);
        sleep(1);
    }

    shmdt(shared);
    return 0;
}
