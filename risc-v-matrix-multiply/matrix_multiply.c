#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <math.h>

#ifdef _WIN32
#include <malloc.h>
#else
#include <unistd.h>
#endif

#ifdef USE_VECTOR
#include <riscv_vector.h>
#endif

// Configuration
#define DEFAULT_SIZE 512
#define DEFAULT_TILE_SIZE 64
#define NUM_RUNS 3

// Matrix structure
typedef struct {
    double* data;
    int size;
} Matrix;

// Function prototypes
Matrix* create_matrix(int size);
void free_matrix(Matrix* matrix);
void initialize_matrix(Matrix* matrix, int seed);
void print_matrix_sample(Matrix* matrix, int sample_size);
double get_time_diff(struct timespec start, struct timespec end);
void naive_multiply(Matrix* A, Matrix* B, Matrix* C);
void cache_aware_multiply(Matrix* A, Matrix* B, Matrix* C, int tile_size);
void cache_aware_multiply_tiled(Matrix* A, Matrix* B, Matrix* C, int tile_size);
int verify_result(Matrix* C1, Matrix* C2, double tolerance);
void benchmark_multiplication(int size, int tile_size);

#ifdef USE_VECTOR
void vector_multiply(Matrix* A, Matrix* B, Matrix* C);
#endif

// Create and allocate matrix
Matrix* create_matrix(int size) {
    Matrix* matrix = (Matrix*)malloc(sizeof(Matrix));
    if (!matrix) {
        fprintf(stderr, "Error: Failed to allocate matrix structure\n");
        return NULL;
    }
    
    matrix->size = size;
#ifdef _WIN32
    matrix->data = (double*)_aligned_malloc(size * size * sizeof(double), 64);
#else
    matrix->data = (double*)aligned_alloc(64, size * size * sizeof(double));
#endif
    if (!matrix->data) {
        fprintf(stderr, "Error: Failed to allocate matrix data\n");
        free(matrix);
        return NULL;
    }
    
    return matrix;
}

// Free matrix memory
void free_matrix(Matrix* matrix) {
    if (matrix) {
        if (matrix->data) {
#ifdef _WIN32
            _aligned_free(matrix->data);
#else
            free(matrix->data);
#endif
        }
        free(matrix);
    }
}

// Initialize matrix with random values
void initialize_matrix(Matrix* matrix, int seed) {
    srand(seed);
    for (int i = 0; i < matrix->size * matrix->size; i++) {
        matrix->data[i] = (double)(rand() % 100) / 10.0;
    }
}

// Print a sample of the matrix for verification
void print_matrix_sample(Matrix* matrix, int sample_size) {
    printf("Matrix sample (%dx%d):\n", sample_size, sample_size);
    for (int i = 0; i < sample_size && i < matrix->size; i++) {
        for (int j = 0; j < sample_size && j < matrix->size; j++) {
            printf("%8.2f ", matrix->data[i * matrix->size + j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Calculate time difference in seconds
double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

// Naive matrix multiplication (IJK order)
void naive_multiply(Matrix* A, Matrix* B, Matrix* C) {
    int n = A->size;
    
    // Initialize result matrix to zero
    for (int i = 0; i < n * n; i++) {
        C->data[i] = 0.0;
    }
    
    // Naive triple loop
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                C->data[i * n + j] += A->data[i * n + k] * B->data[k * n + j];
            }
        }
    }
}

// Cache-aware matrix multiplication with loop tiling/blocking
void cache_aware_multiply(Matrix* A, Matrix* B, Matrix* C, int tile_size) {
    int n = A->size;
    
    // Initialize result matrix to zero
    for (int i = 0; i < n * n; i++) {
        C->data[i] = 0.0;
    }
    
    // Tiled multiplication
    for (int ii = 0; ii < n; ii += tile_size) {
        for (int jj = 0; jj < n; jj += tile_size) {
            for (int kk = 0; kk < n; kk += tile_size) {
                // Compute the actual tile boundaries
                int i_end = (ii + tile_size < n) ? ii + tile_size : n;
                int j_end = (jj + tile_size < n) ? jj + tile_size : n;
                int k_end = (kk + tile_size < n) ? kk + tile_size : n;
                
                // Multiply tiles
                for (int i = ii; i < i_end; i++) {
                    for (int j = jj; j < j_end; j++) {
                        register double sum = C->data[i * n + j];
                        for (int k = kk; k < k_end; k++) {
                            sum += A->data[i * n + k] * B->data[k * n + j];
                        }
                        C->data[i * n + j] = sum;
                    }
                }
            }
        }
    }
}

// Alternative cache-aware implementation with better data locality
void cache_aware_multiply_tiled(Matrix* A, Matrix* B, Matrix* C, int tile_size) {
    int n = A->size;
    
    // Initialize result matrix to zero
    memset(C->data, 0, n * n * sizeof(double));
    
    // Tiled multiplication with IKJ order for better cache locality
    for (int ii = 0; ii < n; ii += tile_size) {
        for (int kk = 0; kk < n; kk += tile_size) {
            for (int jj = 0; jj < n; jj += tile_size) {
                // Compute the actual tile boundaries
                int i_end = (ii + tile_size < n) ? ii + tile_size : n;
                int j_end = (jj + tile_size < n) ? jj + tile_size : n;
                int k_end = (kk + tile_size < n) ? kk + tile_size : n;
                
                // Multiply tiles with IKJ order
                for (int i = ii; i < i_end; i++) {
                    for (int k = kk; k < k_end; k++) {
                        register double a_ik = A->data[i * n + k];
                        for (int j = jj; j < j_end; j++) {
                            C->data[i * n + j] += a_ik * B->data[k * n + j];
                        }
                    }
                }
            }
        }
    }
}

#ifdef USE_VECTOR
// Vector-optimized matrix multiplication (if RISC-V vector extension is available)
void vector_multiply(Matrix* A, Matrix* B, Matrix* C, int tile_size) {
    int n = A->size;
    
    // Initialize result matrix to zero
    memset(C->data, 0, n * n * sizeof(double));
    
    // Vector length
    size_t vl = vsetvlmax_e64m1();
    
    // Tiled multiplication with vector instructions
    for (int ii = 0; ii < n; ii += tile_size) {
        for (int kk = 0; kk < n; kk += tile_size) {
            for (int jj = 0; jj < n; jj += tile_size) {
                int i_end = (ii + tile_size < n) ? ii + tile_size : n;
                int j_end = (jj + tile_size < n) ? jj + tile_size : n;
                int k_end = (kk + tile_size < n) ? kk + tile_size : n;
                
                for (int i = ii; i < i_end; i++) {
                    for (int k = kk; k < k_end; k++) {
                        double a_ik = A->data[i * n + k];
                        vfloat64m1_t va_ik = vfmv_v_f_f64m1(a_ik, vl);
                        
                        for (int j = jj; j < j_end; j += vl) {
                            size_t actual_vl = vsetvl_e64m1(j_end - j);
                            
                            vfloat64m1_t vb = vle64_v_f64m1(&B->data[k * n + j], actual_vl);
                            vfloat64m1_t vc = vle64_v_f64m1(&C->data[i * n + j], actual_vl);
                            vc = vfmadd_vv_f64m1(va_ik, vb, vc, actual_vl);
                            vse64_v_f64m1(&C->data[i * n + j], vc, actual_vl);
                        }
                    }
                }
            }
        }
    }
}
#endif

// Verify that two matrices are approximately equal
int verify_result(Matrix* C1, Matrix* C2, double tolerance) {
    if (C1->size != C2->size) {
        return 0;
    }
    
    int n = C1->size;
    for (int i = 0; i < n * n; i++) {
        if (fabs(C1->data[i] - C2->data[i]) > tolerance) {
            printf("Mismatch at position %d: %f vs %f (diff: %f)\n", 
                   i, C1->data[i], C2->data[i], fabs(C1->data[i] - C2->data[i]));
            return 0;
        }
    }
    return 1;
}

// Benchmark matrix multiplication algorithms
void benchmark_multiplication(int size, int tile_size) {
    printf("=== Matrix Multiplication Benchmark ===\n");
    printf("Matrix size: %dx%d\n", size, size);
    printf("Tile size: %d\n", tile_size);
    printf("Number of runs: %d\n\n", NUM_RUNS);
    
    // Create matrices
    Matrix* A = create_matrix(size);
    Matrix* B = create_matrix(size);
    Matrix* C_naive = create_matrix(size);
    Matrix* C_cache = create_matrix(size);
    Matrix* C_tiled = create_matrix(size);
    
    if (!A || !B || !C_naive || !C_cache || !C_tiled) {
        fprintf(stderr, "Error: Failed to allocate matrices\n");
        return;
    }
    
    // Initialize input matrices
    printf("Initializing matrices...\n");
    initialize_matrix(A, 42);
    initialize_matrix(B, 84);
    
    // Show a small sample of input matrices
    if (size <= 8) {
        printf("Matrix A:\n");
        print_matrix_sample(A, size);
        printf("Matrix B:\n");
        print_matrix_sample(B, size);
    }
    
    struct timespec start, end;
    double naive_time = 0.0, cache_time = 0.0, tiled_time = 0.0;
    
    printf("Running benchmarks...\n\n");
    
    // Benchmark naive implementation
    printf("1. Naive Matrix Multiplication:\n");
    for (int run = 0; run < NUM_RUNS; run++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        naive_multiply(A, B, C_naive);
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        double run_time = get_time_diff(start, end);
        naive_time += run_time;
        printf("   Run %d: %.4f seconds\n", run + 1, run_time);
    }
    naive_time /= NUM_RUNS;
    printf("   Average: %.4f seconds\n\n", naive_time);
    
    // Benchmark cache-aware implementation
    printf("2. Cache-Aware Matrix Multiplication (blocking):\n");
    for (int run = 0; run < NUM_RUNS; run++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        cache_aware_multiply(A, B, C_cache, tile_size);
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        double run_time = get_time_diff(start, end);
        cache_time += run_time;
        printf("   Run %d: %.4f seconds\n", run + 1, run_time);
    }
    cache_time /= NUM_RUNS;
    printf("   Average: %.4f seconds\n\n", cache_time);
    
    // Benchmark tiled implementation with better locality
    printf("3. Cache-Aware Matrix Multiplication (tiled IKJ):\n");
    for (int run = 0; run < NUM_RUNS; run++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        cache_aware_multiply_tiled(A, B, C_tiled, tile_size);
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        double run_time = get_time_diff(start, end);
        tiled_time += run_time;
        printf("   Run %d: %.4f seconds\n", run + 1, run_time);
    }
    tiled_time /= NUM_RUNS;
    printf("   Average: %.4f seconds\n\n", tiled_time);
    
#ifdef USE_VECTOR
    // Benchmark vector implementation if available
    Matrix* C_vector = create_matrix(size);
    if (C_vector) {
        printf("4. Vector-Optimized Matrix Multiplication:\n");
        double vector_time = 0.0;
        for (int run = 0; run < NUM_RUNS; run++) {
            clock_gettime(CLOCK_MONOTONIC, &start);
            vector_multiply(A, B, C_vector, tile_size);
            clock_gettime(CLOCK_MONOTONIC, &end);
            
            double run_time = get_time_diff(start, end);
            vector_time += run_time;
            printf("   Run %d: %.4f seconds\n", run + 1, run_time);
        }
        vector_time /= NUM_RUNS;
        printf("   Average: %.4f seconds\n\n", vector_time);
        
        // Verify vector result
        printf("Verifying vector result...\n");
        if (verify_result(C_naive, C_vector, 1e-10)) {
            printf("✓ Vector result matches naive implementation\n\n");
        } else {
            printf("✗ Vector result does not match naive implementation\n\n");
        }
        
        free_matrix(C_vector);
    }
#endif
    
    // Verify results
    printf("Verifying results...\n");
    if (verify_result(C_naive, C_cache, 1e-10)) {
        printf("✓ Cache-aware result matches naive implementation\n");
    } else {
        printf("✗ Cache-aware result does not match naive implementation\n");
    }
    
    if (verify_result(C_naive, C_tiled, 1e-10)) {
        printf("✓ Tiled result matches naive implementation\n");
    } else {
        printf("✗ Tiled result does not match naive implementation\n");
    }
    
    // Show a small sample of result matrix
    if (size <= 8) {
        printf("\nResult matrix C (naive):\n");
        print_matrix_sample(C_naive, size);
    }
    
    // Performance summary
    printf("\n=== Performance Summary ===\n");
    double total_ops = 2.0 * size * size * size;  // Multiply-add operations
    
    printf("Algorithm                 | Time (s) | GFLOPS | Speedup\n");
    printf("--------------------------|----------|--------|---------\n");
    printf("Naive (IJK)              | %8.4f | %6.2f | %7.2fx\n", 
           naive_time, total_ops / (naive_time * 1e9), 1.0);
    printf("Cache-aware (blocking)   | %8.4f | %6.2f | %7.2fx\n", 
           cache_time, total_ops / (cache_time * 1e9), naive_time / cache_time);
    printf("Cache-aware (tiled IKJ)  | %8.4f | %6.2f | %7.2fx\n", 
           tiled_time, total_ops / (tiled_time * 1e9), naive_time / tiled_time);
    
#ifdef USE_VECTOR
    printf("Vector-optimized         | %8.4f | %6.2f | %7.2fx\n", 
           vector_time, total_ops / (vector_time * 1e9), naive_time / vector_time);
#endif
    
    printf("\nBest performing algorithm: ");
    double best_time = naive_time;
    const char* best_name = "Naive";
    
    if (cache_time < best_time) {
        best_time = cache_time;
        best_name = "Cache-aware (blocking)";
    }
    if (tiled_time < best_time) {
        best_time = tiled_time;
        best_name = "Cache-aware (tiled IKJ)";
    }
    
#ifdef USE_VECTOR
    if (vector_time < best_time) {
        best_time = vector_time;
        best_name = "Vector-optimized";
    }
#endif
    
    printf("%s (%.4f seconds, %.2fx speedup)\n", best_name, best_time, naive_time / best_time);
    
    // Clean up
    free_matrix(A);
    free_matrix(B);
    free_matrix(C_naive);
    free_matrix(C_cache);
    free_matrix(C_tiled);
}

int main(int argc, char* argv[]) {
    int size = DEFAULT_SIZE;
    int tile_size = DEFAULT_TILE_SIZE;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--size") == 0) {
            if (i + 1 < argc) {
                size = atoi(argv[++i]);
                if (size <= 0) {
                    fprintf(stderr, "Error: Invalid matrix size\n");
                    return 1;
                }
            }
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tile") == 0) {
            if (i + 1 < argc) {
                tile_size = atoi(argv[++i]);
                if (tile_size <= 0) {
                    fprintf(stderr, "Error: Invalid tile size\n");
                    return 1;
                }
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  -s, --size <n>    Matrix size (default: %d)\n", DEFAULT_SIZE);
            printf("  -t, --tile <n>    Tile size for cache-aware algorithms (default: %d)\n", DEFAULT_TILE_SIZE);
            printf("  -h, --help        Show this help message\n");
            printf("\nTo compile with vector support:\n");
            printf("  gcc -O3 -DUSE_VECTOR -march=rv64gcv matrix_multiply.c -o matrix_multiply\n");
            printf("\nTo compile without vector support:\n");
            printf("  gcc -O3 matrix_multiply.c -o matrix_multiply\n");
            return 0;
        }
    }
    
    printf("RISC-V Matrix Multiplication Benchmark\n");
    printf("======================================\n\n");
    
#ifdef USE_VECTOR
    printf("Compiled with RISC-V vector support\n");
#else
    printf("Compiled without vector support\n");
#endif
    
    printf("Compilation flags: ");
#ifdef __GNUC__
    printf("GCC optimization level: ");
    #ifdef __OPTIMIZE__
        printf("enabled");
    #else
        printf("disabled");
    #endif
#endif
    printf("\n\n");
    
    // Run benchmark
    benchmark_multiplication(size, tile_size);
    
    return 0;
}
