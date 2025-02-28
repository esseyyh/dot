#include <cuda_runtime.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_THREADS 1024
#define NUM_REPETITIONS 5

__global__ void matvec_kernel(float *A, float *B, float *C, int M, int K) {
  int row = blockIdx.x * blockDim.x + threadIdx.x;
  if (row < M) {
    float sum = 0.0f;
    for (int k = 0; k < K; ++k) {
      sum += A[row * K + k] * B[k];
    }
    C[row] = sum;
  }
}

void matvec(float *A, float *B, float *C, int M, int K) {
  float *d_A, *d_B, *d_C;

  size_t size_A = M * K * sizeof(float);
  size_t size_B = K * sizeof(float);
  size_t size_C = M * sizeof(float);

  cudaMalloc(&d_A, size_A);
  cudaMalloc(&d_B, size_B);
  cudaMalloc(&d_C, size_C);

  cudaMemcpy(d_A, A, size_A, cudaMemcpyHostToDevice);
  cudaMemcpy(d_B, B, size_B, cudaMemcpyHostToDevice);

  int block_size = 256;
  int grid_size = (M + block_size - 1) / block_size;

  matvec_kernel<<<grid_size, block_size>>>(d_A, d_B, d_C, M, K);

  cudaMemcpy(C, d_C, size_C, cudaMemcpyDeviceToHost);

  cudaFree(d_A);
  cudaFree(d_B);
  cudaFree(d_C);
}

double get_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char *argv[]) {
  int sizes[][2] = {{128, 128},   {512, 512},   {1024, 1024},  {2048, 2048},
                    {4096, 4096}, {8192, 8192}, {16384, 16384}};
  int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
  int num_threads = 4; // Default value
  if (argc > 1) {
    num_threads = atoi(argv[1]);
    if (num_threads <= 0 || num_threads > MAX_THREADS) {
      fprintf(stderr, "Invalid number of threads. Using default (4).\n");
      num_threads = 4;
    }
  }
  srand(time(NULL));

  printf("m,k,time,gflops\n");

  double best_gflops = 0.0;
  int best_m = 0, best_k = 0;

  for (int i = 0; i < num_sizes; i++) {
    int M = sizes[i][0];
    int K = sizes[i][1];
    float *A = (float *)malloc(M * K * sizeof(float));
    float *B = (float *)malloc(K * sizeof(float));
    float *C = (float *)malloc(M * sizeof(float));
    if (!A || !B || !C) {
      fprintf(stderr, "Memory allocation failed\n");
      exit(1);
    }

    for (int j = 0; j < M * K; j++) {
      A[j] = (float)rand() / RAND_MAX;
    }
    for (int j = 0; j < K; j++) {
      B[j] = (float)rand() / RAND_MAX;
    }

    double total_time = 0.0;
    double min_time = DBL_MAX;

    for (int rep = 0; rep < NUM_REPETITIONS; rep++) {
      double start_time = get_time();
      matvec(A, B, C, M, K);
      double end_time = get_time();
      double elapsed_time = end_time - start_time;

      total_time += elapsed_time;
      if (elapsed_time < min_time) {
        min_time = elapsed_time;
      }
    }

    double avg_time = total_time / NUM_REPETITIONS;
    double flops = 2.0 * M * K;
    double avg_gflops = flops / (avg_time * 1e9);
    double max_gflops = flops / (min_time * 1e9);

    printf("%d,%d,%.6f,%.2f\n", M, K, avg_time, avg_gflops);

    if (max_gflops > best_gflops) {
      best_gflops = max_gflops;
      best_m = M;
      best_k = K;
    }

    free(A);
    free(B);
    free(C);
  }

  printf("\nBest configuration:\n");
  printf("M=%d, K=%d\n", best_m, best_k);
  printf("Best performance: %.2f GFLOPS\n", best_gflops);

  return 0;
}
