@echo off
REM Test script for RISC-V Matrix Multiplication Benchmark (Windows)
REM This script demonstrates different compilation options and runs basic tests

echo RISC-V Matrix Multiplication Test Suite (Windows)
echo ================================================
echo.

REM Check if gcc is available
gcc --version >nul 2>&1
if errorlevel 1 (
    echo Error: 'gcc' command not found. Please install GCC compiler.
    echo You can install it via MinGW-w64, MSYS2, or other Windows GCC distributions.
    pause
    exit /b 1
)

echo Building standard version...
if exist matrix_multiply.exe del matrix_multiply.exe
gcc -O3 -std=c99 -Wall matrix_multiply.c -o matrix_multiply.exe -lm
if errorlevel 1 (
    echo Error: Build failed
    pause
    exit /b 1
)
echo Standard build successful
echo.

echo Running quick test with small matrix (64x64)...
echo ================================================
matrix_multiply.exe --size 64 --tile 16

echo.
echo Testing vector build (may fail if vector support not available)...
if exist matrix_multiply_vector.exe del matrix_multiply_vector.exe
gcc -O3 -DUSE_VECTOR -march=rv64gcv -std=c99 -Wall matrix_multiply.c -o matrix_multiply_vector.exe -lm >nul 2>&1
if errorlevel 1 (
    echo Vector build failed - this is normal if RISC-V vector support is not available
) else (
    echo Vector build successful
    echo.
    echo Running vector test...
    echo =====================
    matrix_multiply_vector.exe --size 64 --tile 16
)

echo.
echo Testing different matrix sizes...
echo ================================

for %%s in (32 64 128) do (
    echo Testing %%sx%%s matrix:
    matrix_multiply.exe --size %%s --tile 16 | findstr "Best performing"
    echo.
)

echo Testing different tile sizes on 128x128 matrix...
echo ================================================

for %%t in (8 16 32 64) do (
    echo Testing tile size %%t:
    matrix_multiply.exe --size 128 --tile %%t | findstr "Best performing"
    echo.
)

echo Test suite completed!
echo.
echo Manual compilation commands:
echo   Standard build: gcc -O3 -std=c99 -Wall matrix_multiply.c -o matrix_multiply.exe -lm
echo   Vector build:   gcc -O3 -DUSE_VECTOR -march=rv64gcv -std=c99 -Wall matrix_multiply.c -o matrix_multiply_vector.exe -lm
echo.
echo Usage examples:
echo   matrix_multiply.exe --size 256 --tile 32
echo   matrix_multiply.exe --size 512 --tile 64
echo   matrix_multiply.exe --help
echo.
pause
