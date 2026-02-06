# Windows PowerShell Build Script for ARES
# This script compiles the project without CMake using cl.exe (MSVC) or g++

param(
    [string]$Compiler = "auto"  # "msvc", "gcc", or "auto"
)

Write-Host "=== ARES Build Script ===" -ForegroundColor Cyan
Write-Host ""

# Detect compiler
$useGCC = $false
if ($Compiler -eq "auto") {
    if (Get-Command g++ -ErrorAction SilentlyContinue) {
        Write-Host "Detected: GCC/MinGW" -ForegroundColor Green
        $useGCC = $true
    }
    elseif (Get-Command cl -ErrorAction SilentlyContinue) {
        Write-Host "Detected: MSVC" -ForegroundColor Green
        $useGCC = $false
    }
    else {
        Write-Host "ERROR: No C++ compiler found!" -ForegroundColor Red
        Write-Host "Please install Visual Studio or MinGW-w64" -ForegroundColor Yellow
        exit 1
    }
}
elseif ($Compiler -eq "gcc") {
    $useGCC = $true
}
else {
    $useGCC = $false
}

# Create build directory
New-Item -ItemType Directory -Force -Path "build" | Out-Null
Set-Location build

Write-Host "Building ARES library..." -ForegroundColor Cyan

if ($useGCC) {
    # GCC Build
    Write-Host "Compiling with GCC..." -ForegroundColor Yellow
    
    g++ -c -std=c++17 -O3 -march=native -I../include ../src/aes_baseline.cpp -o aes_baseline.o
    g++ -c -std=c++17 -O3 -march=native -I../include ../src/aes_simd.cpp -o aes_simd.o
    g++ -c -std=c++17 -O3 -march=native -I../include ../src/gaussian_baseline.cpp -o gaussian_baseline.o
    g++ -c -std=c++17 -O3 -march=native -I../include ../src/gaussian_simd.cpp -o gaussian_simd.o
    g++ -c -std=c++17 -O3 -march=native -I../include ../src/gaussian_tiled.cpp -o gaussian_tiled.o
    g++ -c -std=c++17 -O3 -march=native -I../include ../src/image_io.cpp -o image_io.o
    
    Write-Host "Building tests..." -ForegroundColor Yellow
    g++ -std=c++17 -O3 -march=native -I../include ../tests/test_aes.cpp aes_baseline.o aes_simd.o -o test_aes.exe
    g++ -std=c++17 -O3 -march=native -I../include ../tests/test_gaussian.cpp gaussian_baseline.o gaussian_simd.o gaussian_tiled.o -o test_gaussian.exe
    
    Write-Host "Building benchmarks..." -ForegroundColor Yellow
    g++ -std=c++17 -O3 -march=native -I../include ../benchmarks/bench_aes.cpp aes_baseline.o aes_simd.o -o bench_aes.exe
    g++ -std=c++17 -O3 -march=native -I../include ../benchmarks/bench_gaussian.cpp gaussian_baseline.o gaussian_simd.o gaussian_tiled.o -o bench_gaussian.exe
    
    Write-Host "Building demo..." -ForegroundColor Yellow
    g++ -std=c++17 -O3 -march=native -I../include ../demo/demo.cpp aes_baseline.o aes_simd.o gaussian_baseline.o gaussian_simd.o gaussian_tiled.o image_io.o -o ares_demo.exe
    
}
else {
    # MSVC Build
    Write-Host "Compiling with MSVC..." -ForegroundColor Yellow
    
    cl /c /std:c++20 /O2 /arch:AVX2 /I..\include ..\src\aes_simd.cpp
    cl /c /std:c++20 /O2 /arch:AVX2 /I..\include ..\src\gaussian_baseline.cpp
    cl /c /std:c++20 /O2 /arch:AVX2 /I..\include ..\src\gaussian_simd.cpp
    cl /c /std:c++20 /O2 /arch:AVX2 /I..\include ..\src\gaussian_tiled.cpp
    
    Write-Host "Building tests..." -ForegroundColor Yellow
    cl /std:c++20 /O2 /arch:AVX2 /I..\include ..\tests\test_aes.cpp aes_baseline.obj aes_simd.obj /Fe:test_aes.exe
    cl /std:c++20 /O2 /arch:AVX2 /I..\include ..\tests\test_gaussian.cpp gaussian_baseline.obj gaussian_simd.obj gaussian_tiled.obj /Fe:test_gaussian.exe
    
    Write-Host "Building benchmarks..." -ForegroundColor Yellow
    cl /std:c++20 /O2 /arch:AVX2 /I..\include ..\benchmarks\bench_aes.cpp aes_baseline.obj aes_simd.obj /Fe:bench_aes.exe
    cl /std:c++20 /O2 /arch:AVX2 /I..\include ..\benchmarks\bench_gaussian.cpp gaussian_baseline.obj gaussian_simd.obj gaussian_tiled.obj /Fe:bench_gaussian.exe
}

Set-Location ..

Write-Host ""
Write-Host "=== Build Complete! ===" -ForegroundColor Green
Write-Host ""
Write-Host "To run tests:" -ForegroundColor Cyan
Write-Host "  .\build\test_aes.exe"
Write-Host "  .\build\test_gaussian.exe"
Write-Host ""
Write-Host "To run benchmarks:" -ForegroundColor Cyan
Write-Host "  .\build\bench_aes.exe"
Write-Host "  .\build\bench_gaussian.exe"
Write-Host ""
Write-Host "To run demo:" -ForegroundColor Cyan
Write-Host "  .\build\ares_demo.exe         (runs both demos)"
Write-Host "  .\build\ares_demo.exe blur    (Gaussian blur demo)"
Write-Host "  .\build\ares_demo.exe aes     (AES encryption demo)"
