#include "ares/gaussian_blur.hpp"
#include <cstdio>
#include <cmath>
#include <algorithm>

#define ASSERT_TRUE(cond) \
    if (!(cond)) { \
        printf("FAILED: %s (line %d)\n", #cond, __LINE__); \
        return false; \
    }

#define TEST(name) \
    bool test_##name(); \
    bool test_##name()

using namespace ares;

// Helper: check if two floats are approximately equal
bool approx_equal(float a, float b, float tolerance = 0.01f) {
    return std::abs(a - b) < tolerance;
}

TEST(image_creation) {
    Image img(64, 64);
    ASSERT_TRUE(img.data != nullptr);
    ASSERT_TRUE(img.width == 64);
    ASSERT_TRUE(img.height == 64);
    
    printf("✓ Image creation successful\n");
    return true;
}

TEST(gaussian_baseline_blur) {
    const size_t size = 32;
    Image input(size, size);
    Image output(size, size);
    
    // Fill with simple pattern
    for (size_t y = 0; y < size; ++y) {
        for (size_t x = 0; x < size; ++x) {
            size_t idx = (y * size + x) * 4;
            input.data[idx + 0] = (x < size/2) ? 1.0f : 0.0f; // R
            input.data[idx + 1] = (y < size/2) ? 1.0f : 0.0f; // G
            input.data[idx + 2] = 0.5f;                        // B
            input.data[idx + 3] = 1.0f;                        // A
        }
    }
    
    gaussian_blur_baseline(input, output, 2.0f);
    
    // Verify output is different (blurred)
    bool different = false;
    for (size_t i = 0; i < size * size * 4; ++i) {
        if (!approx_equal(input.data[i], output.data[i], 0.1f)) {
            different = true;
            break;
        }
    }
    ASSERT_TRUE(different);
    
    printf("✓ Gaussian baseline blur produces different output\n");
    return true;
}

TEST(gaussian_simd_blur) {
    const size_t size = 32;
    Image input(size, size);
    Image output(size, size);
    
    // Fill with pattern
    for (size_t y = 0; y < size; ++y) {
        for (size_t x = 0; x < size; ++x) {
            size_t idx = (y * size + x) * 4;
            input.data[idx + 0] = static_cast<float>(x) / size;
            input.data[idx + 1] = static_cast<float>(y) / size;
            input.data[idx + 2] = 0.5f;
            input.data[idx + 3] = 1.0f;
        }
    }
    
    gaussian_blur_simd(input, output, 2.0f);
    
    // Verify it blurred
    bool different = false;
    for (size_t i = 0; i < size * size * 4; ++i) {
        if (!approx_equal(input.data[i], output.data[i], 0.1f)) {
            different = true;
            break;
        }
    }
    ASSERT_TRUE(different);
    
    printf("✓ Gaussian SIMD blur produces different output\n");
    return true;
}

TEST(gaussian_baseline_vs_simd) {
    const size_t size = 32;
    Image input(size, size);
    Image output_baseline(size, size);
    Image output_simd(size, size);
    
    // Fill with test pattern
    for (size_t y = 0; y < size; ++y) {
        for (size_t x = 0; x < size; ++x) {
            size_t idx = (y * size + x) * 4;
            input.data[idx + 0] = static_cast<float>(x) / size;
            input.data[idx + 1] = static_cast<float>(y) / size;
            input.data[idx + 2] = 0.5f;
            input.data[idx + 3] = 1.0f;
        }
    }
    
    gaussian_blur_baseline(input, output_baseline, 2.0f);
    gaussian_blur_simd(input, output_simd, 2.0f);
    
    // Compare results (should be very close)
    size_t mismatches = 0;
    float max_diff = 0.0f;
    for (size_t i = 0; i < size * size * 4; ++i) {
        float diff = std::abs(output_baseline.data[i] - output_simd.data[i]);
        max_diff = std::max(max_diff, diff);
        if (!approx_equal(output_baseline.data[i], output_simd.data[i], 0.05f)) {
            mismatches++;
        }
    }
    
    // Allow small differences due to floating-point precision
    ASSERT_TRUE(mismatches < (size * size * 4) / 100); // < 1% mismatch
    printf("  Max difference: %.6f, Mismatches: %zu/%zu\n", 
           max_diff, mismatches, size * size * 4);
    
    printf("✓ Gaussian baseline and SIMD produce similar results\n");
    return true;
}

TEST(gaussian_tiled_blur) {
    const size_t size = 64; // Larger for tiling
    Image input(size, size);
    Image output_simd(size, size);
    Image output_tiled(size, size);
    
    // Fill with pattern
    for (size_t y = 0; y < size; ++y) {
        for (size_t x = 0; x < size; ++x) {
            size_t idx = (y * size + x) * 4;
            input.data[idx + 0] = std::sin(x * 0.1f) * 0.5f + 0.5f;
            input.data[idx + 1] = std::cos(y * 0.1f) * 0.5f + 0.5f;
            input.data[idx + 2] = 0.5f;
            input.data[idx + 3] = 1.0f;
        }
    }
    
    gaussian_blur_simd(input, output_simd, 2.0f);
    gaussian_blur_tiled(input, output_tiled, 2.0f);
    
    // Compare tiled vs SIMD
    size_t mismatches = 0;
    for (size_t i = 0; i < size * size * 4; ++i) {
        if (!approx_equal(output_simd.data[i], output_tiled.data[i], 0.05f)) {
            mismatches++;
        }
    }
    
    ASSERT_TRUE(mismatches < (size * size * 4) / 100);
    
    printf("✓ Gaussian tiled blur matches SIMD version\n");
    return true;
}

int main() {
    printf("=== ARES Gaussian Blur Tests ===\n\n");
    
    bool all_passed = true;
    all_passed &= test_image_creation();
    all_passed &= test_gaussian_baseline_blur();
    all_passed &= test_gaussian_simd_blur();
    all_passed &= test_gaussian_baseline_vs_simd();
    all_passed &= test_gaussian_tiled_blur();
    
    printf("\n");
    if (all_passed) {
        printf("✓ All Gaussian blur tests passed!\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
