#pragma once

#include "gaussian_blur.hpp"
#include <string>

namespace ares {

/**
 * @brief Save image to PPM format (simple, no dependencies)
 * 
 * PPM is a simple image format that can be opened by most image viewers.
 * 
 * @param image Image to save
 * @param filename Output filename (should end with .ppm)
 * @return true if successful, false otherwise
 */
bool save_image_ppm(const Image& image, const std::string& filename);

/**
 * @brief Create a simple test image (gradient pattern)
 * 
 * @param width Image width
 * @param height Image height
 * @return Generated image
 */
Image create_test_image(size_t width, size_t height);

} // namespace ares
