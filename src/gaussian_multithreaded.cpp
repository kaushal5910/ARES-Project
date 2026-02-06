#include "ares/gaussian_blur.hpp"
#include <immintrin.h>
#include <cmath>
#include <algorithm>
#include <thread>
#include <vector>

// Helper for clamping values (C++17 compatible)
template<typename T>
static inline T clamp(T value, T min_val, T max_val) {
    return (value < min_val) ? min_val : ((value > max_val) ? max_val : value);
}

namespace ares {

// Generate aligned Gaussian kernel
static float* generate_kernel_mt(int radius, float sigma) {
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

// Worker function for horizontal pass
static void horizontal_pass_worker(
    const Image& input,
    Image& temp,
    const float* kernel,
    int radius,
    size_t start_row,
    size_t end_row
) {
    for (size_t y = start_row; y < end_row; ++y) {
        for (size_t x = 0; x < input.width; ++x) {
            for (int c = 0; c < 4; ++c) {
                float sum = 0.0f;
                
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
                
                __m128 sum_high = _mm256_extractf128_ps(sum_vec, 1);
                __m128 sum_low = _mm256_castps256_ps128(sum_vec);
                __m128 sum_128 = _mm_add_ps(sum_low, sum_high);
                sum_128 = _mm_hadd_ps(sum_128, sum_128);
                sum_128 = _mm_hadd_ps(sum_128, sum_128);
                sum = _mm_cvtss_f32(sum_128);
                
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

// Worker function for vertical pass
static void vertical_pass_worker(
    const Image& temp,
    Image& output,
    const float* kernel,
    int radius,
    size_t start_row,
    size_t end_row
) {
    for (size_t y = start_row; y < end_row; ++y) {
        for (size_t x = 0; x < temp.width; ++x) {
            for (int c = 0; c < 4; ++c) {
                float sum = 0.0f;
                
                int k = -radius;
                __m256 sum_vec = _mm256_setzero_ps();
                
                for (; k <= radius - 7; k += 8) {
                    float sample_data[8];
                    for (int i = 0; i < 8; ++i) {
                        int sample_y = clamp(static_cast<int>(y) + k + i,
                                           0,
                                           static_cast<int>(temp.height) - 1);
                        sample_data[i] = temp.data[(sample_y * temp.width + x) * 4 + c];
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
                                       static_cast<int>(temp.height) - 1);
                    sum += temp.data[(sample_y * temp.width + x) * 4 + c] * kernel[k + radius];
                }
                
                output.data[(y * temp.width + x) * 4 + c] = sum;
            }
        }
    }
}

void gaussian_blur_multithreaded(const Image& input, Image& output, float sigma) {
    if (input.width != output.width || input.height != output.height) {
        return;
    }
    
    const int radius = static_cast<int>(std::ceil(3.0f * sigma));
    float* kernel = generate_kernel_mt(radius, sigma);
    
    Image temp(input.width, input.height);
    
    // Get number of threads (use hardware concurrency)
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4; // fallback
    
    // Horizontal pass with multi-threading
    {
        std::vector<std::thread> threads;
        size_t rows_per_thread = input.height / num_threads;
        
        for (unsigned int t = 0; t < num_threads; ++t) {
            size_t start_row = t * rows_per_thread;
            size_t end_row = (t == num_threads - 1) ? input.height : (t + 1) * rows_per_thread;
            
            threads.emplace_back(horizontal_pass_worker,
                               std::ref(input),
                               std::ref(temp),
                               kernel,
                               radius,
                               start_row,
                               end_row);
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    }
    
    // Vertical pass with multi-threading
    {
        std::vector<std::thread> threads;
        size_t rows_per_thread = input.height / num_threads;
        
        for (unsigned int t = 0; t < num_threads; ++t) {
            size_t start_row = t * rows_per_thread;
            size_t end_row = (t == num_threads - 1) ? input.height : (t + 1) * rows_per_thread;
            
            threads.emplace_back(vertical_pass_worker,
                               std::ref(temp),
                               std::ref(output),
                               kernel,
                               radius,
                               start_row,
                               end_row);
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    }
    
    _mm_free(kernel);
}

} // namespace ares
