#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <immintrin.h>
#define TILE_SIZE 64


void dot(float *A, float *B, float *C, int ii, int jj) {
    for (int i = 0; i < ii; i += TILE_SIZE) {
        for (int j = 0; j < jj; j += TILE_SIZE) {
            for (int i0 = i; i0 < i + TILE_SIZE && i0 < ii; i0+=2) {
                __m512 acc = _mm512_setzero_ps();
                __m512 acc2 = _mm512_setzero_ps();
                for (int j0 = j; j0 < j + TILE_SIZE && j0 < jj; j0 += 16) {
                    __m512 a0 = _mm512_loadu_ps(&A[i0 * jj + j0]);
                    __m512 a1 = _mm512_loadu_ps(&A[(i0+1) * jj + j0]);
                    __m512 b0 = _mm512_loadu_ps(&B[j0]);
                    acc = _mm512_fmadd_ps(a0, b0, acc);
                    acc2 = _mm512_fmadd_ps(a1, b0, acc2);
                }
                float sum = _mm512_reduce_add_ps(acc);
                float sum2 = _mm512_reduce_add_ps(acc2);
                C[i0] += sum;
                C[i0+1] += sum2;
            }
        }
    }
}


int main() {
  int sizes[][2] = {{128, 128}, {256, 256}, {512, 512}, {1024, 1024}, {2048, 2048}, {4096, 4096}};
  int iteration = 10;

  srand(time(NULL));
  printf("m,n,time,flops\n");
  for (int i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
    int M = sizes[i][0];
    int N = sizes[i][1];

    float *A = (float *)_mm_malloc(M * N * sizeof(float),64);
    float *B = (float *)_mm_malloc(N * sizeof(float),64);
    float *C = (float *)_mm_malloc(M * sizeof(float),64);

    // randomize the values of the matrices
    for (int j = 0; j < M * N; j++) {
      A[j] = (float)rand() / RAND_MAX;
    }
    for (int j = 0; j < N; j++) {
      B[j] = (float)rand() / RAND_MAX;
    }
if (A == NULL || B == NULL || C == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
}


    for (int iter = 0; iter < iteration; iter++) {
      clock_t start_time = clock();
      dot(A, B, C, M, N);
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

























