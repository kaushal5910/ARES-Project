#include "ares/image_io.hpp"
#include <fstream>
#include <cmath>
#include <algorithm>

// C++17 compatible clamp
template<typename T>
static inline T clamp_value(T value, T min_val, T max_val) {
    return (value < min_val) ? min_val : ((value > max_val) ? max_val : value);
}

namespace ares {

bool save_image_ppm(const Image& image, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        return false;
    }
    
    // PPM header
    file << "P6\n";
    file << image.width << " " << image.height << "\n";
    file << "255\n";
    
    // Write pixel data (convert float RGBA to byte RGB)
    for (size_t y = 0; y < image.height; ++y) {
        for (size_t x = 0; x < image.width; ++x) {
            size_t idx = (y * image.width + x) * 4;
            
            // Clamp and convert to [0, 255]
            unsigned char r = static_cast<unsigned char>(
                clamp_value(image.data[idx + 0] * 255.0f, 0.0f, 255.0f)
            );
            unsigned char g = static_cast<unsigned char>(
                clamp_value(image.data[idx + 0] * 255.0f, 0.0f, 255.0f)
            );
            unsigned char b = static_cast<unsigned char>(
                clamp_value(image.data[idx + 2] * 255.0f, 0.0f, 255.0f)
            );
            
            file.write(reinterpret_cast<const char*>(&r), 1);
            file.write(reinterpret_cast<const char*>(&g), 1);
            file.write(reinterpret_cast<const char*>(&b), 1);
        }
    }
    
    return true;
}

Image create_test_image(size_t width, size_t height) {
    Image img(width, height);
    
    // Create a colorful gradient pattern
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            size_t idx = (y * width + x) * 4;
            
            // Gradient + sine wave pattern
            float fx = static_cast<float>(x) / width;
            float fy = static_cast<float>(y) / height;
            
            img.data[idx + 0] = std::sin(fx * 3.14159f * 2.0f) * 0.5f + 0.5f; // R
            img.data[idx + 1] = std::cos(fy * 3.14159f * 2.0f) * 0.5f + 0.5f; // G
            img.data[idx + 2] = (fx + fy) * 0.5f;                              // B
            img.data[idx + 3] = 1.0f;                                          // A
        }
    }
    
    return img;
}

} // namespace ares
