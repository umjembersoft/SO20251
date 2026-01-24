#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

typedef struct {
    char *text;
    long start;
    long end;
    long count;
} ThreadData;

long count_words_in_range(const char *text, long start, long end) {
    long count = 0;
    int in_word = 0;
    for (long i = start; i < end; i++) {
        if (text[i] == ' ' || text[i] == '\n' || text[i] == '\t')
            in_word = 0;
        else if (in_word == 0) {
            in_word = 1;
            count++;
        }
    }
    return count;
}

void* count_words_thread(void* arg) {
    ThreadData *data = (ThreadData*) arg;
    data->count = count_words_in_range(data->text, data->start, data->end);
    pthread_exit(NULL);
}

long count_words_single(const char *text, long size) {
    return count_words_in_range(text, 0, size);
}

int main() {
    FILE *fp = fopen("input.txt", "r");
    if (!fp) {
        perror("Gagal membuka file");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *buffer = (char*) malloc(size + 1);
    fread(buffer, 1, size, fp);
    buffer[size] = '\0';
    fclose(fp);

    struct timeval start, end;
    double time_single, time_multi;

    // --- Single-thread ---
    gettimeofday(&start, NULL);
    long single_count = count_words_single(buffer, size);
    gettimeofday(&end, NULL);
    time_single = (end.tv_sec - start.tv_sec) * 1000.0 +
                  (end.tv_usec - start.tv_usec) / 1000.0;

    // --- Multi-thread ---
    ThreadData d1 = {buffer, 0, size / 2, 0};
    ThreadData d2 = {buffer, size / 2, size, 0};

    pthread_t t1, t2;
    gettimeofday(&start, NULL);
    pthread_create(&t1, NULL, count_words_thread, &d1);
    pthread_create(&t2, NULL, count_words_thread, &d2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    gettimeofday(&end, NULL);
    time_multi = (end.tv_sec - start.tv_sec) * 1000.0 +
                 (end.tv_usec - start.tv_usec) / 1000.0;

    long total_words = d1.count + d2.count;

    printf("=== HASIL PERHITUNGAN ===\n");
    printf("Jumlah kata (single-thread) : %ld\n", single_count);
    printf("Jumlah kata (multi-thread)  : %ld\n", total_words);
    printf("\n=== WAKTU EKSEKUSI ===\n");
    printf("Single-thread : %.3f ms\n", time_single);
    printf("Multi-thread  : %.3f ms\n", time_multi);

    free(buffer);
    return 0;
}
