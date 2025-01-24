#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define TILE_SIZE 32
// accepts the pointers to the tensors  and thier respective sizes

void dot(float *A, float *B, float *C, int ii, int jj) {
  for (int i = 0; i < ii; i+=TILE_SIZE) {
      for (int j = 0; j < jj; j+=TILE_SIZE) {
        for (int i0 = i; i0 < ii +TILE_SIZE && i0 < TILE_SIZE; i0++) {
            float sum=0;
            for (int j0 = j; j0 < jj + TILE_SIZE  && j0 < TILE_SIZE; j0++) {
                sum+= A[i0 * jj + j0] * B[j0];
            }
      C[i0] += sum;
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
