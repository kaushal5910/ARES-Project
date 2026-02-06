#include "ares/gaussian_blur.hpp"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <immintrin.h>

// Helper for clamping values (C++17 compatible)
template<typename T>
static inline T clamp(T value, T min_val, T max_val) {
    return (value < min_val) ? min_val : ((value > max_val) ? max_val : value);
}

namespace ares {

// Image implementation
Image::Image(size_t w, size_t h) : width(w), height(h) {
    // Allocate aligned memory for SIMD operations (32-byte alignment for AVX2)
    data = static_cast<float*>(_mm_malloc(width * height * 4 * sizeof(float), 32));
    if (data) {
        std::memset(data, 0, width * height * 4 * sizeof(float));
    }
}

Image::~Image() {
    if (data) {
        _mm_free(data);
        data = nullptr;
    }
}

Image::Image(Image&& other) noexcept
    : width(other.width), height(other.height), data(other.data) {
    other.data = nullptr;
    other.width = 0;
    other.height = 0;
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        if (data) {
            _mm_free(data);
        }
        width = other.width;
        height = other.height;
        data = other.data;
        other.data = nullptr;
        other.width = 0;
        other.height = 0;
    }
    return *this;
}

// Generate 1D Gaussian kernel
static void generate_gaussian_kernel(float* kernel, int radius, float sigma) {
    float sum = 0.0f;
    int size = 2 * radius + 1;
    
    for (int i = 0; i < size; ++i) {
        float x = static_cast<float>(i - radius);
        kernel[i] = std::exp(-(x * x) / (2.0f * sigma * sigma));
        sum += kernel[i];
    }
    
    // Normalize
    for (int i = 0; i < size; ++i) {
        kernel[i] /= sum;
    }
}

void gaussian_blur_baseline(const Image& input, Image& output, float sigma) {
    if (input.width != output.width || input.height != output.height) {
        return; // Size mismatch
    }
    
    const int radius = static_cast<int>(std::ceil(3.0f * sigma));
    const int kernel_size = 2 * radius + 1;
    
    // Generate Gaussian kernel
    float* kernel = new float[kernel_size];
    generate_gaussian_kernel(kernel, radius, sigma);
    
    // Temporary buffer for horizontal pass
    Image temp(input.width, input.height);
    
    // Horizontal pass
    for (size_t y = 0; y < input.height; ++y) {
        for (size_t x = 0; x < input.width; ++x) {
            for (int c = 0; c < 4; ++c) { // RGBA channels
                float sum = 0.0f;
                
                for (int k = -radius; k <= radius; ++k) {
                    int sample_x = clamp(static_cast<int>(x) + k, 
                                         0, 
                                         static_cast<int>(input.width) - 1);
                    size_t idx = (y * input.width + sample_x) * 4 + c;
                    sum += input.data[idx] * kernel[k + radius];
                }
                
                temp.data[(y * input.width + x) * 4 + c] = sum;
            }
        }
    }
    
    // Vertical pass
    for (size_t y = 0; y < input.height; ++y) {
        for (size_t x = 0; x < input.width; ++x) {
            for (int c = 0; c < 4; ++c) { // RGBA channels
                float sum = 0.0f;
                
                for (int k = -radius; k <= radius; ++k) {
                    int sample_y = clamp(static_cast<int>(y) + k,
                                         0,
                                         static_cast<int>(input.height) - 1);
                    size_t idx = (sample_y * input.width + x) * 4 + c;
                    sum += temp.data[idx] * kernel[k + radius];
                }
                
                output.data[(y * input.width + x) * 4 + c] = sum;
            }
        }
    }
    
    delete[] kernel;
}

} // namespace ares
