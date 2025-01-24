#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#define TILE_SIZE 64
#define MAX_THREADS 16


typedef struct {
    float *A;
    float *B;
    float *C;
    int M, N;
    int start_row;
    int end_row;
} ThreadArgs;


void *dot_threads(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    float *A = args->A;
    float *B = args->B;
    float *C = args->C;
    int M = args->M, N = args->N;
    int start_row = args->start_row;
    int end_row = args->end_row;
    printf("start_row: %d, end_row: %d\n", start_row, end_row);

  for (int i = start_row; i < end_row; i+=TILE_SIZE) {
      for (int j = 0; j < N; j+=TILE_SIZE) {
        for (int i0 = i; i0 < M +TILE_SIZE && i0 < TILE_SIZE; i0++) {
            float sum=0;
            for (int j0 = j; j0 < N + TILE_SIZE  && j0 < TILE_SIZE; j0++) {
                sum+= A[i0 * N+ j0] * B[j0];
            }
      C[i0] += sum;
    }
  }
  }
return NULL;
}


void dot(float *A, float *B,float *C,  int M, int N, int num_threads) {
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadArgs *thread_args = malloc(num_threads * sizeof(ThreadArgs));
    int rows_per_thread = M / num_threads;
    int extra_rows = M % num_threads;
    for (int i = 0; i < num_threads; i++) {
        thread_args[i].A = A;
        thread_args[i].B = B;
        thread_args[i].C = C;
        thread_args[i].M = M;
        thread_args[i].N = N;
        thread_args[i].start_row = i * rows_per_thread + (i < extra_rows ? i : extra_rows);
        thread_args[i].end_row = (i + 1) * rows_per_thread + (i < extra_rows ? i + 1 : extra_rows);
        pthread_create(&threads[i], NULL, dot_threads, &thread_args[i]);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    free(threads);
    free(thread_args);
}



int main() {
  int sizes[][2] = {{128, 128}, {256, 256}, {512, 512}, {1024, 1024}, {2048, 2048}, {4096, 4096}};
  int iteration = 1;

  srand(time(NULL));
  printf("m,n,time,flops\n");
  for (int i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
    int M = sizes[i][0];
    int N = sizes[i][1];

    float *A = (float *)malloc(M * N * sizeof(float));
    float *B = (float *)malloc(N * sizeof(float));
    float *C = (float *)malloc(M * sizeof(float));

    // randomize the values of the matrices
    for (int j = 0; j < M * N; j++) {
      A[j] = (float)rand() / RAND_MAX;
    }
    for (int j = 0; j < N; j++) {
      B[j] = (float)rand() / RAND_MAX;
    }

    for (int iter = 0; iter < iteration; iter++) {
      clock_t start_time = clock();
      dot(A, B, C, M, N,8);
      clock_t end_time = clock();

      // calculate the time and flops taken by the dot function
      double iteration_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
      double flops = 2.0 * M * N;
      // convert to gigaflops
      double flops_per_second = flops / iteration_time / 1e9; // Convert to gigaflops

      printf("%d,%d,%.6f,%.2f\n", M, N, iteration_time, flops_per_second);
    }
    free(A);
    free(B);
    free(C);
  }
  return 0;
}
