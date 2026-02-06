# Building Without CMake (Manual Compilation)

If you don't have CMake installed, you can compile the project manually using your C++ compiler.

## Option 1: Build with Visual Studio (Windows)

### Prerequisites
- Visual Studio 2019 or later (with C++ desktop development tools)
- Ensure your CPU supports AVX2 and AES-NI

### Steps

1. **Open Visual Studio** → Create New Project → Empty C++ Project
2. **Add all source files** from `src/` folder
3. **Add include directory**: Project Properties → C/C++ → General → Additional Include Directories → Add `$(ProjectDir)..\include`
4. **Enable AVX2**: Project Properties → C/C++ → Code Generation → Enable Enhanced Instruction Set → AVX2
5. **Set C++ Standard**: Project Properties → C/C++ → Language → C++ Language Standard → C++20
6. **Build in Release mode** for optimal performance

### Build Tests Separately

Create separate projects for:
- `test_aes.cpp` → Links to all AES source files
- `test_gaussian.cpp` → Links to all Gaussian blur source files

## Option 2: Build with GCC/Clang (Windows MinGW or Linux)

### Compile the Library

```bash
# Navigate to project directory
cd Ares_Project

# Compile all source files into object files
g++ -c -std=c++20 -O3 -mavx2 -mfma -maes -Iinclude src/aes_baseline.cpp -o aes_baseline.o
g++ -c -std=c++20 -O3 -mavx2 -mfma -maes -Iinclude src/aes_simd.cpp -o aes_simd.o
g++ -c -std=c++20 -O3 -mavx2 -mfma -maes -Iinclude src/gaussian_baseline.cpp -o gaussian_baseline.o
g++ -c -std=c++20 -O3 -mavx2 -mfma -maes -Iinclude src/gaussian_simd.cpp -o gaussian_simd.o
g++ -c -std=c++20 -O3 -mavx2 -mfma -maes -Iinclude src/gaussian_tiled.cpp -o gaussian_tiled.o

# Create static library (optional)
ar rcs libares.a aes_baseline.o aes_simd.o gaussian_baseline.o gaussian_simd.o gaussian_tiled.o
```

### Build and Run Tests

```bash
# Compile AES test
g++ -std=c++20 -O3 -mavx2 -mfma -maes -Iinclude tests/test_aes.cpp aes_baseline.o aes_simd.o -o test_aes

# Run AES test
./test_aes

# Compile Gaussian test
g++ -std=c++20 -O3 -mavx2 -mfma -maes -Iinclude tests/test_gaussian.cpp gaussian_baseline.o gaussian_simd.o gaussian_tiled.o -o test_gaussian

# Run Gaussian test
./test_gaussian
```

### Build and Run Benchmarks

```bash
# AES benchmark
g++ -std=c++20 -O3 -mavx2 -mfma -maes -Iinclude benchmarks/bench_aes.cpp aes_baseline.o aes_simd.o -o bench_aes
./bench_aes

# Gaussian benchmark
g++ -std=c++20 -O3 -mavx2 -mfma -maes -Iinclude benchmarks/bench_gaussian.cpp gaussian_baseline.o gaussian_simd.o gaussian_tiled.o -o bench_gaussian
./bench_gaussian
```

## Option 3: Use the Provided Build Script (Windows PowerShell)

A convenience script `build.ps1` is provided for Windows users.

```powershell
.\build.ps1
```

## Option 4: Install CMake (Recommended)

### Windows
Download CMake from https://cmake.org/download/ and add to PATH, then:
```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Linux (Ubuntu/Debian)
```bash
sudo apt-get install cmake
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

## Troubleshooting

### "AVX2 not supported" error
Your CPU might not support AVX2. Check with:
- **Windows**: Download CPU-Z
- **Linux**: `grep avx2 /proc/cpuinfo`

### "AES-NI not supported" warning
The SIMD AES tests will be skipped if your CPU doesn't have AES-NI support. This is normal on older CPUs.

### Compilation errors
Make sure you're using:
- GCC 9+ or Clang 10+ or MSVC 2019+
- Flags: `-mavx2 -mfma -maes` (GCC/Clang) or `/arch:AVX2` (MSVC)
