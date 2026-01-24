#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#define TOTAL_ROOMS 5
#define TOTAL_CUSTOMERS 10
#define MAX_TRIES 3

#define BOOKING_MIN 1
#define BOOKING_MAX 3
#define STAY_TIME 5
#define RETRY_WAIT 2

// Shared data
int rooms[TOTAL_ROOMS]; // 0 = kosong, 1 = terisi
pthread_mutex_t rooms_lock; // mutex untuk rooms + menjaga print rapi
sem_t rooms_sem; // semaphore untuk mengatur jumlah kamar tersisa

// Ambil timestamp [HH:MM:SS]
void timestamp(char *buf, size_t len) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strftime(buf, len, "[%H:%M:%S]", &tm);
}

// print aman (dengan lock agar tidak saling bercampur)
void print_safe(const char *fmt, ...) {
    pthread_mutex_lock(&rooms_lock);
    char timebuf[16];
    timestamp(timebuf, sizeof(timebuf));
    printf("%s ", timebuf);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
    fflush(stdout);
    pthread_mutex_unlock(&rooms_lock);
}

// Cari kamar kosong dan tandai terisi. Panggil ini **dengan** rooms_lock dipegang.
int find_and_occupy_room_locked(int customer_id) {
    for (int i = 0; i < TOTAL_ROOMS; ++i) {
        if (rooms[i] == 0) {
            rooms[i] = 1;
            return i; // index kamar 0-based
        }
    }
    return -1; // mustahil jika semaphore konsisten
}

int available_rooms_count_locked() {
    int cnt = 0;
    for (int i = 0; i < TOTAL_ROOMS; ++i) if (rooms[i] == 0) ++cnt;
    return cnt;
}

void free_room_locked(int index) {
    if (index >= 0 && index < TOTAL_ROOMS) rooms[index] = 0;
}

void *customer_thread(void *arg) {
    int id = *((int*)arg);
    free(arg); // free alokasi dari main
    char name[32];
    snprintf(name, sizeof(name), "Customer-%d", id);

    for (int attempt = 1; attempt <= MAX_TRIES; ++attempt) {
        print_safe("🤵🏻‍♂️ %s mencoba booking... (percobaan %d)", name, attempt);

        // Simulasi waktu proses booking (1..3 detik)
        int booking_time = (rand() % (BOOKING_MAX - BOOKING_MIN + 1)) + BOOKING_MIN;
        sleep(booking_time);

        // Coba ambil semaphore non-blocking
        if (sem_trywait(&rooms_sem) == 0) {
            // Ada slot — temukan nomor kamar secara aman
            pthread_mutex_lock(&rooms_lock);
            int room_index = find_and_occupy_room_locked(id);
            int remaining = available_rooms_count_locked();
            pthread_mutex_unlock(&rooms_lock);

            if (room_index >= 0) {
                print_safe("✔  %s berhasil booking! (kamar #%d) (%d kamar tersisa)", name, room_index + 1, remaining);
                // Menginap
                sleep(STAY_TIME);
                // Check-out
                pthread_mutex_lock(&rooms_lock);
                free_room_locked(room_index);
                int remaining_after = available_rooms_count_locked();
                pthread_mutex_unlock(&rooms_lock);

                // Release semaphore slot
                sem_post(&rooms_sem);

                print_safe("🏨 %s check-out (kamar #%d) (%d kamar tersisa)", name, room_index + 1, remaining_after);
                return NULL;
            } else {
                // Seharusnya tak terjadi — tapi untuk kesigapan:
                sem_post(&rooms_sem);
                print_safe("❗️ %s: unexpected: no free room found even after sem acquired", name);
                return NULL;
            }
        } else {
            // Gagal karena kamar penuh
            print_safe("✖️ %s gagal booking (kamar penuh), menunggu %d detik lalu coba lagi...", name, RETRY_WAIT);
            sleep(RETRY_WAIT);
        }
    }

    print_safe("🚫 %s gagal booking setelah %d percobaan.", name, MAX_TRIES);
    return NULL;
}

int main(void) {
    srand(time(NULL) ^ getpid());

    // Init rooms
    for (int i = 0; i < TOTAL_ROOMS; ++i) rooms[i] = 0;
    pthread_mutex_init(&rooms_lock, NULL);
    sem_init(&rooms_sem, 0, TOTAL_ROOMS); // nilai awal = jumlah kamar

    printf("=== SISTEM BOOKING HOTEL ===\n");
    printf("Total Kamar: %d\n\n", TOTAL_ROOMS);

    pthread_t threads[TOTAL_CUSTOMERS];

    for (int i = 0; i < TOTAL_CUSTOMERS; ++i) {
        int *arg = malloc(sizeof(int));
        if (!arg) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        *arg = i + 1;
        if (pthread_create(&threads[i], NULL, customer_thread, arg) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        // sedikit jeda agar output awal lebih berurutan saat demo
        usleep(50000); // 50 ms
    }

    for (int i = 0; i < TOTAL_CUSTOMERS; ++i) {
        pthread_join(threads[i], NULL);
    }

    printf("\nSemua proses booking selesai.\n");

    sem_destroy(&rooms_sem);
    pthread_mutex_destroy(&rooms_lock);
    return 0;
}
