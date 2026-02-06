# ARES - Architecture-aware Real-time Encryption & Signal-processing

A high-performance computing project demonstrating microarchitectural optimizations (SIMD, cache blocking, hardware intrinsics) for compute-intensive kernels targeting modern x86-64 CPUs.

## 🎯 Project Overview

ARES implements two compute kernels with progressive optimization levels:

1. **AES-128 Encryption** (Security Kernel)
   - Baseline: Pure C++ implementation
   - SIMD: Hardware-accelerated using AES-NI instructions

2. **2D Gaussian Blur** (Signal Processing Kernel)
   - Baseline: Standard nested loops
   - SIMD: AVX2 vectorization
   - Tiled: Cache-blocked with prefetching

## 🚀 Key Features

- ✅ **Clean baseline implementations** for correctness verification
- ✅ **SIMD optimization** using AVX2 and AES-NI intrinsics
- ✅ **Cache-aware tiling** with 32×32 blocks for L1 cache efficiency
- ✅ **Memory alignment** for optimal SIMD performance
- ✅ **Comprehensive testing** with numerical verification
- ✅ **Benchmarking suite** for performance measurement

## 📋 Requirements

- **Compiler**: GCC 9+, Clang 10+, or MSVC 2019+
- **CMake**: 3.15 or higher *(optional - see manual build below)*
- **CPU**: x86-64 with AVX2 support (for SIMD), AES-NI (for AES hardware acceleration)
- **OS**: Windows, Linux, or macOS

> **💡 No CMake?** See [`BUILD_MANUAL.md`](BUILD_MANUAL.md) for manual compilation or use `build.ps1` on Windows.

## 🛠️ Building the Project

### Windows (PowerShell)

```powershell
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
cmake --build . --config Release

# Run tests
ctest -C Release
```

### Linux/macOS

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
make -j$(nproc)

# Run tests
ctest
```

## 🧪 Running Tests

After building, run the test executables:

```bash
# Windows
.\build\tests\Release\test_aes.exe
.\build\tests\Release\test_gaussian.exe

# Linux/macOS
./build/tests/test_aes
./build/tests/test_gaussian
```

Example output:
```
=== ARES AES Tests ===

✓ AES baseline encryption produces different output
✓ AES SIMD encryption produces different output
✓ AES baseline and SIMD produce identical results
✓ AES encrypts multiple blocks successfully

✓ All AES tests passed!
```

## 📊 Running Benchmarks

```bash
# Windows
.\build\benchmarks\Release\bench_aes.exe
.\build\benchmarks\Release\bench_gaussian.exe

# Linux/macOS
./build/benchmarks/bench_aes
./build/benchmarks/bench_gaussian
```

Example benchmark output:
```
=== ARES AES Encryption Benchmarks ===

Data Size: 1 MB
  Baseline:   1234.56 μs  |  810.23 MB/s
  SIMD:        156.78 μs  |  6400.15 MB/s  |  7.87x speedup
```

## 🏗️ Project Structure

```
ARES_Project/
├── include/ares/         # Public headers
│   ├── aes.hpp
│   └── gaussian_blur.hpp
├── src/                  # Implementation files
│   ├── aes_baseline.cpp
│   ├── aes_simd.cpp
│   ├── gaussian_baseline.cpp
│   ├── gaussian_simd.cpp
│   └── gaussian_tiled.cpp
├── tests/                # Unit tests
│   ├── test_aes.cpp
│   └── test_gaussian.cpp
├── benchmarks/           # Performance benchmarks
│   ├── bench_aes.cpp
│   └── bench_gaussian.cpp
├── CMakeLists.txt        # Root build configuration
└── README.md
```

## 🎓 Technical Approach

### AES Encryption
- **Baseline**: Implements AES-128 using lookup tables (S-box) and bitwise operations
- **SIMD**: Uses `_mm_aesenc_si128()` and `_mm_aesenclast_si128()` for hardware-accelerated encryption
- **Speedup**: ~8-12x on modern CPUs with AES-NI support

### Gaussian Blur
- **Baseline**: Separable convolution (horizontal + vertical passes) with standard loops
- **SIMD**: AVX2 vectorization processing 8 floats simultaneously with FMA instructions
- **Tiled**: 32×32 cache blocking + SIMD + `_mm_prefetch` hints
- **Speedup**: 3-5x (SIMD), 4-7x (Tiled) depending on image size

## 📈 Performance Expectations

On a modern CPU (e.g., AMD Ryzen or Intel Core with AVX2):

| Kernel | Optimization | Expected Speedup |
|--------|-------------|------------------|
| AES-128 | AES-NI | 8-12x |
| Gaussian (512×512) | SIMD | 3-4x |
| Gaussian (512×512) | Tiled | 4-6x |
| Gaussian (4K) | Tiled | 5-7x |

## 🔬 Optimization Techniques Demonstrated

1. **SIMD Vectorization**: Using intrinsics to process multiple data elements per instruction
2. **Memory Alignment**: Ensuring data is aligned to cache line boundaries (32/64 bytes)
3. **Cache Blocking**: Tiling data to fit in L1 cache (32KB typical)
4. **Prefetching**: Using `_mm_prefetch` to bring data into cache before use
5. **Loop Unrolling**: Implicit through SIMD operations
6. **Separable Convolution**: Reducing O(n²) to O(2n) for 2D Gaussian blur

## 📚 Learning Outcomes

This project demonstrates understanding of:
- Modern CPU microarchitecture (cache hierarchy, SIMD units)
- Performance optimization techniques
- Intrinsics programming (AVX2, AES-NI)
- Benchmarking methodology
- Software engineering best practices (testing, build systems)

## ⚠️ Notes

- SIMD versions require AVX2 support (check with `cat /proc/cpuinfo | grep avx2` on Linux)
- AES-NI implementation requires hardware support (check with `grep aes /proc/cpuinfo`)
- Results may vary based on CPU model, clock speed, and system load
- This is an educational project demonstrating optimization concepts

## 📝 License

This is an educational project for academic purposes.

## 👤 Author

Created as a demonstration project for understanding microarchitectural optimizations and high-performance computing concepts.

---

**For more details, see [`PERFORMANCE.md`](file:///c:/Users/P%20kaushal%20Murthy/Desktop/Ares_Project/PERFORMANCE.md) for performance analysis.**
