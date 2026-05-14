#include <windows.h>
#include <stdio.h>
#include <math.h>

#define N 100000000LL
#define BLOCK_SIZE 4314200LL

volatile long long total_processed = 0;
double sum = 0.0;
CRITICAL_SECTION cs;
int current_block = 0;
int total_blocks = 0;
int thread_count;
HANDLE *threads;
int *thread_has_work;
long long *thread_start;
long long *thread_end;

DWORD WINAPI worker(LPVOID arg) {
    int idx = *(int*)arg;
    double local_sum;
    
    while (1) {
        if (thread_has_work[idx]) {
            local_sum = 0.0;
            for (long long i = thread_start[idx]; i < thread_end[idx]; i++) {
                InterlockedIncrement64(&total_processed);
                double x = (i + 0.5) / N;
                local_sum += 4.0 / (1.0 + x * x);
            }
            
            EnterCriticalSection(&cs);
            sum += local_sum;
            LeaveCriticalSection(&cs);
            
            thread_has_work[idx] = 0;
        }
        
        SuspendThread(GetCurrentThread());
    }
    return 0;
}

void run_test(int tc) {
    thread_count = tc;
    sum = 0.0;
    current_block = 0;
    total_blocks = (int)((N + BLOCK_SIZE - 1) / BLOCK_SIZE);
    total_processed = 0;
    
    threads = (HANDLE*)malloc(thread_count * sizeof(HANDLE));
    thread_has_work = (int*)malloc(thread_count * sizeof(int));
    thread_start = (long long*)malloc(thread_count * sizeof(long long));
    thread_end = (long long*)malloc(thread_count * sizeof(long long));
    
    int* indices = (int*)malloc(thread_count * sizeof(int));
    
    for (int i = 0; i < thread_count; i++) {
        thread_has_work[i] = 0;
        thread_start[i] = 0;
        thread_end[i] = 0;
        indices[i] = i;
        threads[i] = CreateThread(NULL, 0, worker, &indices[i], CREATE_SUSPENDED, NULL);
    }
    
    for (int i = 0; i < thread_count && current_block < total_blocks; i++) {
        thread_start[i] = (long long)current_block * BLOCK_SIZE;
        thread_end[i] = thread_start[i] + BLOCK_SIZE;
        if (thread_end[i] > N) thread_end[i] = N;
        thread_has_work[i] = 1;
        current_block++;
        ResumeThread(threads[i]);
    }
    
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    
    while (current_block < total_blocks) {
        for (int i = 0; i < thread_count; i++) {
            if (thread_has_work[i] == 0) {
                thread_start[i] = (long long)current_block * BLOCK_SIZE;
                thread_end[i] = thread_start[i] + BLOCK_SIZE;
                if (thread_end[i] > N) thread_end[i] = N;
                thread_has_work[i] = 1;
                current_block++;
                ResumeThread(threads[i]);
            }
        }
        Sleep(0);
    }
    
    int completed;
    do {
        completed = 0;
        for (int i = 0; i < thread_count; i++) {
            if (thread_has_work[i]) completed++;
        }
        Sleep(0);
    } while (completed > 0);
    
    QueryPerformanceCounter(&end);
    double pi = sum / N;
    double time_ms = (end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
    
    printf("%d threads | pi = %.15f | time = %.2f ms | processed = %lld\n", 
           thread_count, pi, time_ms, total_processed);
    
    for (int i = 0; i < thread_count; i++) {
        TerminateThread(threads[i], 0);
        CloseHandle(threads[i]);
    }
    
    free(threads);
    free(thread_has_work);
    free(thread_start);
    free(thread_end);
    free(indices);
}

int main() {
    InitializeCriticalSection(&cs);
    
    int test_threads[] = {1, 2, 4, 8, 12, 16};
    
    for (int i = 0; i < 6; i++) {
        run_test(test_threads[i]);
    }
    
    DeleteCriticalSection(&cs);
    
    printf("\nDone. Press Enter to exit...");
    getchar();
    
    return 0;
}
