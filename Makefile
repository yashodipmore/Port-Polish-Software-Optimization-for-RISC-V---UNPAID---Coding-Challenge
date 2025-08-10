# Makefile for RISC-V Matrix Multiplication Benchmark
# Supports compilation with and without vector extensions

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra
LDFLAGS = -lm

# Target executable names
TARGET = matrix_multiply
TARGET_VECTOR = matrix_multiply_vector

# Source files
SOURCES = matrix_multiply.c

# Default target
all: $(TARGET)

# Standard compilation without vector support
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -O3 $(SOURCES) -o $(TARGET) $(LDFLAGS)
	@echo "Built standard version: $(TARGET)"

# Compilation with RISC-V vector support
vector: $(TARGET_VECTOR)

$(TARGET_VECTOR): $(SOURCES)
	$(CC) $(CFLAGS) -O3 -DUSE_VECTOR -march=rv64gcv $(SOURCES) -o $(TARGET_VECTOR) $(LDFLAGS)
	@echo "Built vector version: $(TARGET_VECTOR)"

# Debug versions
debug: $(SOURCES)
	$(CC) $(CFLAGS) -g -O0 -DDEBUG $(SOURCES) -o $(TARGET)_debug $(LDFLAGS)
	@echo "Built debug version: $(TARGET)_debug"

debug_vector: $(SOURCES)
	$(CC) $(CFLAGS) -g -O0 -DDEBUG -DUSE_VECTOR -march=rv64gcv $(SOURCES) -o $(TARGET_VECTOR)_debug $(LDFLAGS)
	@echo "Built debug vector version: $(TARGET_VECTOR)_debug"

# Cross-compilation for RISC-V (requires RISC-V toolchain)
riscv: $(SOURCES)
	riscv64-unknown-linux-gnu-gcc $(CFLAGS) -O3 $(SOURCES) -o $(TARGET)_riscv $(LDFLAGS)
	@echo "Built RISC-V version: $(TARGET)_riscv"

riscv_vector: $(SOURCES)
	riscv64-unknown-linux-gnu-gcc $(CFLAGS) -O3 -DUSE_VECTOR -march=rv64gcv $(SOURCES) -o $(TARGET_VECTOR)_riscv $(LDFLAGS)
	@echo "Built RISC-V vector version: $(TARGET_VECTOR)_riscv"

# Performance testing targets
test: $(TARGET)
	@echo "Running performance test with default parameters..."
	./$(TARGET)

test_small: $(TARGET)
	@echo "Running performance test with small matrix (128x128)..."
	./$(TARGET) --size 128 --tile 32

test_large: $(TARGET)
	@echo "Running performance test with large matrix (1024x1024)..."
	./$(TARGET) --size 1024 --tile 64

test_vector: $(TARGET_VECTOR)
	@echo "Running performance test with vector support..."
	./$(TARGET_VECTOR)

# Benchmark different tile sizes
benchmark_tiles: $(TARGET)
	@echo "Benchmarking different tile sizes..."
	@for tile in 16 32 64 128; do \
		echo "=== Tile size: $$tile ==="; \
		./$(TARGET) --size 512 --tile $$tile; \
		echo; \
	done

# Clean up build artifacts
clean:
	rm -f $(TARGET) $(TARGET_VECTOR) $(TARGET)_debug $(TARGET_VECTOR)_debug
	rm -f $(TARGET)_riscv $(TARGET_VECTOR)_riscv
	@echo "Cleaned up build artifacts"

# Show help
help:
	@echo "Available targets:"
	@echo "  all           - Build standard version"
	@echo "  vector        - Build with RISC-V vector support"
	@echo "  debug         - Build debug version"
	@echo "  debug_vector  - Build debug version with vector support"
	@echo "  riscv         - Cross-compile for RISC-V"
	@echo "  riscv_vector  - Cross-compile for RISC-V with vector support"
	@echo ""
	@echo "  test          - Run performance test with default parameters"
	@echo "  test_small    - Run test with 128x128 matrix"
	@echo "  test_large    - Run test with 1024x1024 matrix"
	@echo "  test_vector   - Run test with vector support"
	@echo "  benchmark_tiles - Benchmark different tile sizes"
	@echo ""
	@echo "  clean         - Remove build artifacts"
	@echo "  help          - Show this help message"

.PHONY: all vector debug debug_vector riscv riscv_vector test test_small test_large test_vector benchmark_tiles clean help
