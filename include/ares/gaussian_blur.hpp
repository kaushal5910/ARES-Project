#pragma once

#include <cstddef>
#include <memory>

namespace ares {

/**
 * @brief Simple image structure for RGBA data
 */
struct Image {
    size_t width;
    size_t height;
    float* data;  // RGBA interleaved (4 floats per pixel)
    
    Image(size_t w, size_t h);
    ~Image();
    
    // Disable copy, enable move
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;
    
    size_t size_bytes() const { return width * height * 4 * sizeof(float); }
};

/**
 * @brief Baseline Gaussian blur using standard nested loops
 * 
 * Implements separable Gaussian blur (horizontal then vertical passes).
 * Uses standard floating-point operations without vectorization.
 * 
 * @param input Source image
 * @param output Destination image (must be same size as input)
 * @param sigma Gaussian kernel standard deviation (controls blur strength)
 */
void gaussian_blur_baseline(
    const Image& input,
    Image& output,
    float sigma = 2.0f
);

/**
 * @brief SIMD-optimized Gaussian blur using AVX2
 * 
 * Vectorized implementation processing 8 floats simultaneously.
 * Requires aligned memory for optimal performance.
 * 
 * @param input Source image (data should be 32-byte aligned)
 * @param output Destination image
 * @param sigma Gaussian kernel standard deviation
 */
void gaussian_blur_simd(
    const Image& input,
    Image& output,
    float sigma = 2.0f
);

/**
 * @brief Cache-optimized Gaussian blur using tiling
 * 
 * Processes image in 32x32 tiles to maximize L1 cache utilization.
 * Combines SIMD vectorization with cache-aware blocking.
 * 
 * @param input Source image
 * @param output Destination image
 * @param sigma Gaussian kernel standard deviation
 */
void gaussian_blur_tiled(
    const Image& input,
    Image& output,
    float sigma = 2.0f
);

/**
 * @brief Multi-threaded Gaussian blur using SIMD and threading
 * 
 * Combines SIMD vectorization with multi-threading for maximum performance.
 * Distributes work across CPU cores using std::thread.
 * 
 * @param input Source image
 * @param output Destination image
 * @param sigma Gaussian kernel standard deviation
 */
void gaussian_blur_multithreaded(
    const Image& input,
    Image& output,
    float sigma = 2.0f
);

} // namespace ares
