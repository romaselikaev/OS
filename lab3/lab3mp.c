#include <stdio.h>
#include <omp.h>

#define N 100000000
#define BLOCK_SIZE 4314200        

void run_test(int thread_count) {
    double sum = 0.0;
    omp_set_num_threads(thread_count);
    double start_time = omp_get_wtime();
    #pragma omp parallel for reduction(+:sum) schedule(dynamic, BLOCK_SIZE)
    for (int i = 0; i < N; i++) {
        double x = (i + 0.5) / N;
        sum += 4.0 / (1.0 + x * x);
    }
    double pi = sum * (1.0 / N);
    double time_ms = (omp_get_wtime() - start_time) * 1000.0;
    printf("Потоков: %2d | pi = %.15f | Время: %.2f мс\n", thread_count, pi, time_ms);
}

int main() {
    int threads[] = {1, 2, 4, 8, 12, 16};
    for (int i = 0; i < sizeof(threads) / sizeof(threads[0]); i++) {
        run_test(threads[i]);
    }

    return 0;
}