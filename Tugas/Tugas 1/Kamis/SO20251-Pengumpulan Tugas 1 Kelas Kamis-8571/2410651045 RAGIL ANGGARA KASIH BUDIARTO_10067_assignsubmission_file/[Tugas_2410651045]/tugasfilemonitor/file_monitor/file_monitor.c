#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define EVENT_SIZE     (sizeof(struct inotify_event))
#define EVENT_BUF_LEN  (1024 * (EVENT_SIZE + 16))
#define LOG_FILE       "file_activity.log"

int inotify_fd, watch_fd, log_fd;

void handle_signal(int sig) {
    printf("\nSinyal %d diterima. Melakukan pembersihan...\n", sig);
    if (watch_fd >= 0)
        inotify_rm_watch(inotify_fd, watch_fd);
    if (inotify_fd >= 0)
        close(inotify_fd);
    if (log_fd >= 0)
        close(log_fd);
    printf("Inotify file descriptor ditutup.\n");
    printf("Watch descriptor dihapus.\n");
    printf("Log file ditutup dengan bersih.\n");
    printf("Program pemantauan dihentikan dengan bersih.\n");
    exit(0);
}

void log_event(const char *event_type, const char *filename, pid_t pid) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    char log_entry[256];

    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
    snprintf(log_entry, sizeof(log_entry),
             "[%s] [%s] %s [PID: %d]\n",
             timestamp, event_type, filename, pid);

    write(log_fd, log_entry, strlen(log_entry));
    printf("%s", log_entry);

    printf("📧 [EMAIL NOTIF] File '%s' mengalami event %s oleh PID %d\n", filename, event_type, pid);
    printf("→ Mengirim email notifikasi (simulasi)\n\n");
}

int main() {
    char dir_path[256];
    char buffer[EVENT_BUF_LEN];
    int length, i = 0;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    printf("=== Pemantau File System Dimulai ===\n");
    printf("Masukkan direktori yang ingin anda pantau: ");
    scanf("%255s", dir_path);

    log_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd < 0) {
        perror("Gagal membuka file log");
        exit(EXIT_FAILURE);
    }

    inotify_fd = inotify_init();
    if (inotify_fd < 0) {
        perror("inotify_init gagal");
        exit(EXIT_FAILURE);
    }

    watch_fd = inotify_add_watch(inotify_fd, dir_path,
                                 IN_CREATE | IN_MODIFY | IN_DELETE);
    if (watch_fd < 0) {
        if (errno == ENOSPC)
            fprintf(stderr, "[ERROR] Batas maksimum inotify watch tercapai (ENOSPC).\n");
        else
            perror("inotify_add_watch gagal");
        exit(EXIT_FAILURE);
    }

    printf("Direktori yang dipantau: %s\n", dir_path);
    printf("Log dicatat ke: %s\n", LOG_FILE);
    printf("Mode: NIM GANJIL → Simulasi email notifikasi aktif.\n");
    printf("Tekan Ctrl+C untuk menghentikan.\n\n");

    while (1) {
        i = 0;
        length = read(inotify_fd, buffer, EVENT_BUF_LEN);
        if (length < 0) {
            perror("read gagal");
            break;
        }

        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len > 0) {
                // 🔥 Abaikan event dari file log agar tidak looping
                if (strcmp(event->name, LOG_FILE) == 0) {
                    i += EVENT_SIZE + event->len;
                    continue;
                }

                if (event->mask & IN_CREATE)
                    log_event("CREATED", event->name, getpid());
                else if (event->mask & IN_MODIFY)
                    log_event("MODIFIED", event->name, getpid());
                else if (event->mask & IN_DELETE)
                    log_event("DELETED", event->name, getpid());
            }
            i += EVENT_SIZE + event->len;
        }
    }

    handle_signal(0);
    return 0;
}

