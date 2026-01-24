#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/statvfs.h>
#include <arpa/inet.h>

/* === CPU INFO === */
void cpu_info() {
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (!f) { perror("Error opening /proc/cpuinfo"); return; }

    char line[256], model[128] = "", mhz[32] = "";
    int cores = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "model name", 10) == 0 && !*model)
            sscanf(line, "model name\t: %[^\n]", model);
        else if (strncmp(line, "cpu MHz", 7) == 0 && !*mhz)
            sscanf(line, "cpu MHz\t: %[^\n]", mhz);
        else if (strncmp(line, "processor", 9) == 0)
            cores++;
    }
    fclose(f);

    printf("=== CPU INFO ===\n");
    printf("Model : %s\nCores : %d\nClock : %s MHz\n\n", model, cores, mhz);
}

/* === MEMORY INFO === */
void mem_info() {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) { perror("Error opening /proc/meminfo"); return; }

    char label[32];
    unsigned long val;
    char unit[8];
    unsigned long total = 0, avail = 0;

    while (fscanf(f, "%31s %lu %7s\n", label, &val, unit) == 3) {
        if (!strcmp(label, "MemTotal:")) total = val;
        if (!strcmp(label, "MemAvailable:")) avail = val;
    }
    fclose(f);

    printf("=== MEMORY INFO ===\n");
    printf("Total : %.2f MB\nUsed  : %.2f MB\n\n",
           total / 1024.0, (total - avail) / 1024.0);
}

/* === DISK INFO === */
void disk_info() {
    struct statvfs s;
    if (statvfs("/", &s) != 0) { perror("Error statvfs()"); return; }

    unsigned long long total = s.f_blocks * s.f_frsize;
    unsigned long long freeb = s.f_bavail * s.f_frsize;
    unsigned long long used = total - freeb;

    printf("=== DISK INFO ===\n");
    printf("Total : %.2f GB\nUsed  : %.2f GB\n\n", total / 1e9, used / 1e9);
}

/* === PROCESS LIST === */
void list_process() {
    DIR *d = opendir("/proc");
    if (!d) { perror("Error opening /proc"); return; }

    struct dirent *e;
    printf("=== RUNNING PROCESSES ===\nPID\tNAME\n");

    while ((e = readdir(d))) {
        if (!isdigit(e->d_name[0])) continue;

        char path[256];
        char name[128];
        snprintf(path, sizeof(path) - 1, "/proc/%.200s/comm", e->d_name);
        path[sizeof(path) - 1] = '\0'; // pastikan null-terminated

        FILE *f = fopen(path, "r");
        if (f) {
            if (fgets(name, sizeof(name), f)) {
                name[strcspn(name, "\n")] = 0;
                printf("%s\t%s\n", e->d_name, name);
            }
            fclose(f);
        }
    }
    closedir(d);
    printf("\n");
}

/* === NETWORK INTERFACE === */
void net_dev() {
    FILE *f = fopen("/proc/net/dev", "r");
    if (!f) { perror("Error opening /proc/net/dev"); return; }

    char line[256];
    fgets(line, sizeof(line), f);
    fgets(line, sizeof(line), f);

    printf("=== NETWORK INTERFACES ===\nIface\tRX(Bytes)\tTX(Bytes)\n");
    while (fgets(line, sizeof(line), f)) {
        char iface[32];
        unsigned long rx, tx;
        if (sscanf(line, " %31[^:]: %lu %*s %*s %*s %*s %*s %*s %*s %lu",
                   iface, &rx, &tx) == 2)
            printf("%s\t%lu\t%lu\n", iface, rx, tx);
    }
    fclose(f);
    printf("\n");
}

/* === VARIASI NIM GANJIL: TCP CONNECTIONS === */
void net_tcp() {
    FILE *f = fopen("/proc/net/tcp", "r");
    if (!f) { perror("Error opening /proc/net/tcp"); return; }

    char line[512];
    fgets(line, sizeof(line), f); // skip header

    printf("=== TCP CONNECTIONS (NIM GANJIL) ===\n");
    printf("Local Address\tPort\tState\n");

    while (fgets(line, sizeof(line), f)) {
        char local[64];
        int state;
        if (sscanf(line, "%*d: %63s %*s %x", local, &state) != 2) continue;

        char ip_hex[9] = {0}, port_hex[5] = {0};
        strncpy(ip_hex, local, 8);
        strncpy(port_hex, local + 9, 4);

        unsigned int ip_raw = strtoul(ip_hex, NULL, 16);
        unsigned char *b = (unsigned char *)&ip_raw;
        unsigned int port = strtoul(port_hex, NULL, 16);

        printf("%u.%u.%u.%u\t%u\t%x\n",
               b[0], b[1], b[2], b[3], port, state);
    }
    fclose(f);
    printf("\n");
}

/* === MAIN === */
int main() {
    printf("==== SYSTEM INFORMATION TOOL (TUGAS 4) ====\n\n");

    cpu_info();
    mem_info();
    disk_info();
    list_process();
    net_dev();
    net_tcp();

    printf("==== END OF REPORT ====\n");
    return 0;
}
