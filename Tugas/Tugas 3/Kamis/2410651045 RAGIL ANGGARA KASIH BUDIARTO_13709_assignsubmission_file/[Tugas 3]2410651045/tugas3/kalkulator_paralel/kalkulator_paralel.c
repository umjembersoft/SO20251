#include <stdio.h>
#include <pthread.h>

// Struktur untuk menyimpan data
typedef struct {
    double a, b;
    double result;
} Data;

// Fungsi-fungsi operasi
void* tambah(void* arg) {
    Data* d = (Data*) arg;
    d->result = d->a + d->b;
    pthread_exit(NULL);
}

void* kurang(void* arg) {
    Data* d = (Data*) arg;
    d->result = d->a - d->b;
    pthread_exit(NULL);
}

void* kali(void* arg) {
    Data* d = (Data*) arg;
    d->result = d->a * d->b;
    pthread_exit(NULL);
}

void* bagi(void* arg) {
    Data* d = (Data*) arg;
    if (d->b != 0)
        d->result = d->a / d->b;
    else
        d->result = 0; // hindari pembagian nol
    pthread_exit(NULL);
}

int main() {
    double x, y;
    printf("Masukkan dua angka: ");
    scanf("%lf %lf", &x, &y);

    pthread_t t1, t2, t3, t4;
    Data d1 = {x, y, 0}, d2 = {x, y, 0}, d3 = {x, y, 0}, d4 = {x, y, 0};

    // Buat thread
    pthread_create(&t1, NULL, tambah, &d1);
    pthread_create(&t2, NULL, kurang, &d2);
    pthread_create(&t3, NULL, kali, &d3);
    pthread_create(&t4, NULL, bagi, &d4);

    // Tunggu semua thread selesai
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    // Tampilkan hasil
    printf("\n=== Hasil Perhitungan ===\n");
    printf("Penjumlahan : %.2lf\n", d1.result);
    printf("Pengurangan : %.2lf\n", d2.result);
    printf("Perkalian   : %.2lf\n", d3.result);
    printf("Pembagian   : %.2lf\n", d4.result);

    return 0;
}
