# ARES (Architecture-aware Real-time Encryption & Signal-processing)

**Objective:** To implement and benchmark microarchitectural optimizations (SIMD, Cache Blocking, and Assembly) for compute-intensive kernels, specifically targeting the AMD Zen architecture.

## 1. The Core Modules

The team will build two distinct "Kernels" to show versatility:

1. **The Signal Kernel (Compute-Bound):** Fast Fourier Transform (FFT) or a Large-Scale 2D Gaussian Blur. (Targets: Floating point units, FMA instructions).
2. **The Security Kernel (Data-Bound):** AES-256 Encryption (using `AES-NI` instructions) or ChaCha20 Stream Cipher. (Targets: Integer units, bitwise ops, and memory throughput).

## 2. Technical Roadmap & Phases

### Phase 1: The Baseline (Reference Implementation)

* **Goal:** Write "Clean C++" code using standard loops.
* **Constraint:** No compiler-specific flags or pragmas.
* **Output:** A functional library that produces correct results (the "Gold Standard").

### Phase 2: Vectorization & SIMD (The "Heart" of the project)

* **Implementation:** Rewrite the kernels using AVX2 / AVX-512 Intrinsics (`immintrin.h`).
* **Focus:**
  * Convert Scalar operations (1 value at a time) to Vector operations (8 floats/32 chars at a time).
  * **Memory Alignment:** Use `_mm_malloc` or `alignas(64)` to ensure data is aligned to cache-line boundaries.
  * **Handling the "Tail":** Implement "cleanup loops" for data arrays that aren't perfectly divisible by vector width.

### Phase 3: Microarchitectural Tuning

* **Loop Unrolling:** Manually unroll loops to hide instruction latency and increase ILP (Instruction Level Parallelism).
* **Cache Tiling:** For the Image/Signal kernel, implement Blocking. Instead of processing a 4K image row-by-row, process it in $32 \times 32$ tiles to keep the data in the L1 Cache.
* **Prefetching:** Inject `_mm_prefetch` hints to move the next block of data into the cache before the CPU actually asks for it.

## 3. Toolchain & Profiling (The "AMD Way")

This is the part that makes the project "Industry Grade." The team must use professional tools:

* **Compiler:** Compare outputs of `GCC`, `Clang`, and `AOCC` (AMD Optimizing C/C++ Compiler).
* **Profiling:** Use AMD uProf to generate a "Instruction Mix" report.
* **Roofline Modeling:** Create a plot showing GFLOPS vs. Operational Intensity.

## 4. Implementation Details for the Team

| Component | Industry Standard Approach |
|-----------|---------------------------|
| **Language** | C++20 (for `std::span` and memory management) |
| **Build System** | CMake (with auto-detection for AVX/FMA support) |
| **Assembly** | Use `.s` files or Inline Assembly for the inner-most loop of the AES/FFT kernel. |
| **Verification** | Unit tests against known-good outputs (e.g., OpenCV for blur, OpenSSL for AES). |
| **Benchmarking** | Use Google Benchmark to get cycle-accurate timing and avoid "cold cache" bias. |

## 5. Final Deliverables (The "Resume Killers")

* **Performance Report:** A PDF containing graphs of "Cycles Per Byte" comparing Baseline C++ vs. SIMD vs. SIMD+Tiling.
* **Memory Analysis:** Evidence of reduced L1-D Cache Misses and TLB Misses after optimizations.
* **AOCC Comparison:** A table showing how the AMD-specific compiler (AOCC) optimized the code differently than standard GCC.
