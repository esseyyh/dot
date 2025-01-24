
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// accepts the pointers to the tensors and their respective sizes
void dot(float *A, float *B, float *C, int ii, int jj) {
  for (int i = 0; i < ii; i++) {
    float sum = 0.0;
    for (int j = 0; j < jj; j++) {
      sum += A[i * jj + j] * B[j];
    }
    C[i] = sum;
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
