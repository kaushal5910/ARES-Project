#include "ares/gaussian_blur.hpp"
#include <chrono>
#include <cstdio>
#include <cmath>

using namespace ares;
using namespace std::chrono;

template<typename Func>
double measure(Func func, int iterations = 10) {
    auto start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        func();
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start).count();
    return duration / static_cast<double>(iterations);
}

void benchmark_gaussian(size_t width, size_t height) {
    Image input(width, height);
    Image output(width, height);
    
    // Fill with test pattern
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            size_t idx = (y * width + x) * 4;
            input.data[idx + 0] = std::sin(x * 0.1f) * 0.5f + 0.5f;
            input.data[idx + 1] = std::cos(y * 0.1f) * 0.5f + 0.5f;
            input.data[idx + 2] = 0.5f;
            input.data[idx + 3] = 1.0f;
        }
    }
    
    float sigma = 2.0f;
    
    // Benchmark baseline
    double baseline_time = measure([&]() {
        gaussian_blur_baseline(input, output, sigma);
    }, 5);
    
    printf("  Baseline:  %8.2f ms\n", baseline_time / 1000.0);
    
    // Benchmark SIMD
    double simd_time = measure([&]() {
        gaussian_blur_simd(input, output, sigma);
    }, 5);
    
    double simd_speedup = baseline_time / simd_time;
    printf("  SIMD:      %8.2f ms  |  %.2fx speedup\n",
           simd_time / 1000.0, simd_speedup);
    
    // Benchmark tiled
    double tiled_time = measure([&]() {
        gaussian_blur_tiled(input, output, sigma);
    }, 5);
    
    double tiled_speedup = baseline_time / tiled_time;
    printf("  Tiled:     %8.2f ms  |  %.2fx speedup\n",
           tiled_time / 1000.0, tiled_speedup);
    
    // Additional metrics
    size_t pixels = width * height;
    double mpixels_per_sec = pixels / (tiled_time / 1000000.0) / 1000000.0;
    printf("  Best throughput: %.2f Mpixels/s\n", mpixels_per_sec);
}

int main() {
    printf("=== ARES Gaussian Blur Benchmarks ===\n\n");
    printf("Testing 2D Gaussian blur performance (sigma=2.0)\n");
    printf("Format: Time per operation | Speedup vs baseline\n\n");
    
    printf("Image Size: 512 x 512\n");
    benchmark_gaussian(512, 512);
    
    printf("\nImage Size: 1024 x 1024\n");
    benchmark_gaussian(1024, 1024);
    
    printf("\nImage Size: 2048 x 2048\n");
    benchmark_gaussian(2048, 2048);
    
    printf("\nImage Size: 3840 x 2160 (4K)\n");
    benchmark_gaussian(3840, 2160);
    
    printf("\n=== Benchmark Complete ===\n");
    printf("\nOptimization Techniques:\n");
    printf("- SIMD: AVX2 vectorization (8 floats at a time)\n");
    printf("- Tiled: 32x32 cache blocking + SIMD + prefetching\n");
    printf("- Both use separable Gaussian convolution\n");
    
    return 0;
}
