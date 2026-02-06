#include "ares/gaussian_blur.hpp"
#include <immintrin.h>
#include <cmath>
#include <algorithm>

// Helper for clamping values (C++17 compatible)
template<typename T>
static inline T clamp(T value, T min_val, T max_val) {
    return (value < min_val) ? min_val : ((value > max_val) ? max_val : value);
}

namespace ares {

// Tile size for cache blocking (32x32 fits well in L1 cache)
constexpr size_t TILE_SIZE = 32;

// Generate aligned Gaussian kernel
static float* generate_aligned_kernel(int radius, float sigma) {
    int size = 2 * radius + 1;
    int aligned_size = ((size + 7) / 8) * 8;
    
    float* kernel = static_cast<float*>(_mm_malloc(aligned_size * sizeof(float), 32));
    
    for (int i = 0; i < aligned_size; ++i) {
        kernel[i] = 0.0f;
    }
    
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        float x = static_cast<float>(i - radius);
        kernel[i] = std::exp(-(x * x) / (2.0f * sigma * sigma));
        sum += kernel[i];
    }
    
    for (int i = 0; i < size; ++i) {
        kernel[i] /= sum;
    }
    
    return kernel;
}

void gaussian_blur_tiled(const Image& input, Image& output, float sigma) {
    if (input.width != output.width || input.height != output.height) {
        return;
    }
    
    const int radius = static_cast<int>(std::ceil(3.0f * sigma));
    float* kernel = generate_aligned_kernel(radius, sigma);
    
    Image temp(input.width, input.height);
    
    // Horizontal pass with tiling
    // Process image in TILE_SIZE x TILE_SIZE blocks
    for (size_t tile_y = 0; tile_y < input.height; tile_y += TILE_SIZE) {
        size_t tile_end_y = std::min(tile_y + TILE_SIZE, input.height);
        
        for (size_t tile_x = 0; tile_x < input.width; tile_x += TILE_SIZE) {
            size_t tile_end_x = std::min(tile_x + TILE_SIZE, input.width);
            
            // Prefetch next tile (hint to CPU)
            if (tile_x + TILE_SIZE < input.width) {
                size_t prefetch_idx = (tile_y * input.width + tile_x + TILE_SIZE) * 4;
                _mm_prefetch(reinterpret_cast<const char*>(&input.data[prefetch_idx]), 
                            _MM_HINT_T0);
            }
            
            // Process current tile
            for (size_t y = tile_y; y < tile_end_y; ++y) {
                for (size_t x = tile_x; x < tile_end_x; ++x) {
                    for (int c = 0; c < 4; ++c) {
                        float sum = 0.0f;
                        
                        // Vectorized convolution within tile
                        int k = -radius;
                        __m256 sum_vec = _mm256_setzero_ps();
                        
                        for (; k <= radius - 7; k += 8) {
                            float sample_data[8];
                            for (int i = 0; i < 8; ++i) {
                                int sample_x = clamp(static_cast<int>(x) + k + i,
                                                     0,
                                                     static_cast<int>(input.width) - 1);
                                sample_data[i] = input.data[(y * input.width + sample_x) * 4 + c];
                            }
                            __m256 samples = _mm256_loadu_ps(sample_data);
                            __m256 kernel_vec = _mm256_loadu_ps(&kernel[k + radius]);
                            sum_vec = _mm256_fmadd_ps(samples, kernel_vec, sum_vec);
                        }
                        
                        // Reduce vector sum
                        __m128 sum_high = _mm256_extractf128_ps(sum_vec, 1);
                        __m128 sum_low = _mm256_castps256_ps128(sum_vec);
                        __m128 sum_128 = _mm_add_ps(sum_low, sum_high);
                        sum_128 = _mm_hadd_ps(sum_128, sum_128);
                        sum_128 = _mm_hadd_ps(sum_128, sum_128);
                        sum = _mm_cvtss_f32(sum_128);
                        
                        // Scalar cleanup
                        for (; k <= radius; ++k) {
                            int sample_x = clamp(static_cast<int>(x) + k,
                                                 0,
                                                 static_cast<int>(input.width) - 1);
                            sum += input.data[(y * input.width + sample_x) * 4 + c] * kernel[k + radius];
                        }
                        
                        temp.data[(y * input.width + x) * 4 + c] = sum;
                    }
                }
            }
        }
    }
    
    // Vertical pass with tiling
    for (size_t tile_y = 0; tile_y < input.height; tile_y += TILE_SIZE) {
        size_t tile_end_y = std::min(tile_y + TILE_SIZE, input.height);
        
        for (size_t tile_x = 0; tile_x < input.width; tile_x += TILE_SIZE) {
            size_t tile_end_x = std::min(tile_x + TILE_SIZE, input.width);
            
            // Prefetch next tile
            if (tile_y + TILE_SIZE < input.height) {
                size_t prefetch_idx = ((tile_y + TILE_SIZE) * input.width + tile_x) * 4;
                _mm_prefetch(reinterpret_cast<const char*>(&temp.data[prefetch_idx]),
                            _MM_HINT_T0);
            }
            
            for (size_t y = tile_y; y < tile_end_y; ++y) {
                for (size_t x = tile_x; x < tile_end_x; ++x) {
                    for (int c = 0; c < 4; ++c) {
                        float sum = 0.0f;
                        
                        int k = -radius;
                        __m256 sum_vec = _mm256_setzero_ps();
                        
                        for (; k <= radius - 7; k += 8) {
                            float sample_data[8];
                            for (int i = 0; i < 8; ++i) {
                                int sample_y = clamp(static_cast<int>(y) + k + i,
                                                     0,
                                                     static_cast<int>(input.height) - 1);
                                sample_data[i] = temp.data[(sample_y * input.width + x) * 4 + c];
                            }
                            __m256 samples = _mm256_loadu_ps(sample_data);
                            __m256 kernel_vec = _mm256_loadu_ps(&kernel[k + radius]);
                            sum_vec = _mm256_fmadd_ps(samples, kernel_vec, sum_vec);
                        }
                        
                        __m128 sum_high = _mm256_extractf128_ps(sum_vec, 1);
                        __m128 sum_low = _mm256_castps256_ps128(sum_vec);
                        __m128 sum_128 = _mm_add_ps(sum_low, sum_high);
                        sum_128 = _mm_hadd_ps(sum_128, sum_128);
                        sum_128 = _mm_hadd_ps(sum_128, sum_128);
                        sum = _mm_cvtss_f32(sum_128);
                        
                        for (; k <= radius; ++k) {
                            int sample_y = clamp(static_cast<int>(y) + k,
                                                 0,
                                                 static_cast<int>(input.height) - 1);
                            sum += temp.data[(sample_y * input.width + x) * 4 + c] * kernel[k + radius];
                        }
                        
                        output.data[(y * input.width + x) * 4 + c] = sum;
                    }
                }
            }
        }
    }
    
    _mm_free(kernel);
}

} // namespace ares
