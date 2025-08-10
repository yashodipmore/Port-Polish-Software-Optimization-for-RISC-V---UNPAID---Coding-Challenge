# Benchmark Results

This file contains benchmark results for the RISC-V Matrix Multiplication implementation.

## Test System Specifications

- **CPU:** [Enter CPU model]
- **Architecture:** [Enter architecture, e.g., RISC-V RV64GCV, x86_64, ARM64]
- **Clock Speed:** [Enter base/boost clock speeds]
- **Cache Sizes:**
  - L1 Data: [Enter L1 cache size]
  - L2: [Enter L2 cache size]  
  - L3: [Enter L3 cache size]
- **Memory:** [Enter RAM size and type]
- **Compiler:** [Enter compiler version]
- **OS:** [Enter operating system]

## Compilation Flags Used

```bash
# Standard build
gcc -O3 -std=c99 -Wall matrix_multiply.c -o matrix_multiply -lm

# Vector build (if supported)
gcc -O3 -DUSE_VECTOR -march=rv64gcv -std=c99 -Wall matrix_multiply.c -o matrix_multiply_vector -lm
```

## Benchmark Results

### Matrix Size: 128x128

| Algorithm | Time (s) | GFLOPS | Speedup |
|-----------|----------|--------|---------|
| Naive (IJK) | [Enter] | [Enter] | 1.00x |
| Cache-aware (blocking) | [Enter] | [Enter] | [Enter]x |
| Cache-aware (tiled IKJ) | [Enter] | [Enter] | [Enter]x |
| Vector-optimized | [Enter] | [Enter] | [Enter]x |

### Matrix Size: 256x256

| Algorithm | Time (s) | GFLOPS | Speedup |
|-----------|----------|--------|---------|
| Naive (IJK) | [Enter] | [Enter] | 1.00x |
| Cache-aware (blocking) | [Enter] | [Enter] | [Enter]x |
| Cache-aware (tiled IKJ) | [Enter] | [Enter] | [Enter]x |
| Vector-optimized | [Enter] | [Enter] | [Enter]x |

### Matrix Size: 512x512

| Algorithm | Time (s) | GFLOPS | Speedup |
|-----------|----------|--------|---------|
| Naive (IJK) | [Enter] | [Enter] | 1.00x |
| Cache-aware (blocking) | [Enter] | [Enter] | [Enter]x |
| Cache-aware (tiled IKJ) | [Enter] | [Enter] | [Enter]x |
| Vector-optimized | [Enter] | [Enter] | [Enter]x |

### Matrix Size: 1024x1024

| Algorithm | Time (s) | GFLOPS | Speedup |
|-----------|----------|--------|---------|
| Naive (IJK) | [Enter] | [Enter] | 1.00x |
| Cache-aware (blocking) | [Enter] | [Enter] | [Enter]x |
| Cache-aware (tiled IKJ) | [Enter] | [Enter] | [Enter]x |
| Vector-optimized | [Enter] | [Enter] | [Enter]x |

## Tile Size Analysis

### Matrix Size: 512x512

| Tile Size | Time (s) | GFLOPS | Notes |
|-----------|----------|--------|-------|
| 16 | [Enter] | [Enter] | [Enter notes] |
| 32 | [Enter] | [Enter] | [Enter notes] |
| 64 | [Enter] | [Enter] | [Enter notes] |
| 128 | [Enter] | [Enter] | [Enter notes] |
| 256 | [Enter] | [Enter] | [Enter notes] |

## Key Observations

1. **Cache Performance:**
   - [Enter observations about cache-aware algorithms]
   - [Best tile size for this system]

2. **Vector Performance:**
   - [Enter observations about vector performance, if available]
   - [Vector length used]

3. **Scaling Behavior:**
   - [How performance scales with matrix size]
   - [Memory bandwidth limitations observed]

## Recommendations

- **Optimal tile size:** [Enter optimal tile size for this system]
- **Best algorithm:** [Enter best performing algorithm]
- **Performance notes:** [Any system-specific optimizations or observations]

## Raw Benchmark Output

```
[Paste raw benchmark output here for reference]
```

## Instructions for Running Benchmarks

To reproduce these results:

```bash
# Build the program
make clean
make

# Run benchmarks
./matrix_multiply --size 128 --tile 32
./matrix_multiply --size 256 --tile 64
./matrix_multiply --size 512 --tile 64
./matrix_multiply --size 1024 --tile 128

# Test different tile sizes
make benchmark_tiles

# If vector support is available
make vector
./matrix_multiply_vector --size 512 --tile 64
```
