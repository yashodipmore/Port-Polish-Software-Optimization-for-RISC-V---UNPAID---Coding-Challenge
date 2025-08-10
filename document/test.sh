#!/bin/bash
# Test script for RISC-V Matrix Multiplication Benchmark
# This script demonstrates different compilation options and runs basic tests

echo "RISC-V Matrix Multiplication Test Suite"
echo "======================================="
echo ""

# Check if make is available
if ! command -v make &> /dev/null; then
    echo "Error: 'make' command not found. Please install make utility."
    exit 1
fi

# Check if gcc is available
if ! command -v gcc &> /dev/null; then
    echo "Error: 'gcc' command not found. Please install GCC compiler."
    exit 1
fi

echo "Building standard version..."
make clean > /dev/null 2>&1
if make > /dev/null 2>&1; then
    echo "✓ Standard build successful"
else
    echo "✗ Standard build failed"
    exit 1
fi

echo ""
echo "Running quick test with small matrix (64x64)..."
echo "================================================"
./matrix_multiply --size 64 --tile 16

echo ""
echo "Building vector version (may fail if vector support not available)..."
if make vector > /dev/null 2>&1; then
    echo "✓ Vector build successful"
    echo ""
    echo "Running vector test..."
    echo "====================="
    ./matrix_multiply_vector --size 64 --tile 16
else
    echo "✗ Vector build failed (this is normal if RISC-V vector support is not available)"
fi

echo ""
echo "Testing different matrix sizes..."
echo "================================"

for size in 32 64 128; do
    echo "Testing ${size}x${size} matrix:"
    ./matrix_multiply --size $size --tile 16 | grep "Best performing"
    echo ""
done

echo "Testing different tile sizes on 128x128 matrix..."
echo "================================================"

for tile in 8 16 32 64; do
    echo "Testing tile size $tile:"
    ./matrix_multiply --size 128 --tile $tile | grep "Best performing"
    echo ""
done

echo "Test suite completed!"
echo ""
echo "To run more comprehensive tests:"
echo "  make test           # Default parameters"
echo "  make test_large     # Large matrix test"
echo "  make benchmark_tiles # Benchmark different tile sizes"
echo ""
echo "To build for RISC-V cross-compilation (requires RISC-V toolchain):"
echo "  make riscv          # Standard RISC-V build"
echo "  make riscv_vector   # RISC-V with vector support"
