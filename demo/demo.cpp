#include "ares/gaussian_blur.hpp"
#include "ares/image_io.hpp"
#include "ares/aes.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <cstring>

using namespace ares;
using namespace std::chrono;

void print_header(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(60, '=') << "\n\n";
}

void demo_gaussian_blur() {
    print_header("GAUSSIAN BLUR DEMONSTRATION");
    
    std::cout << "Creating a 1024x1024 test image...\n";
    Image input = create_test_image(1024, 1024);
    Image output(1024, 1024);
    
    std::cout << "Saving original image to 'original.ppm'...\n";
    if (save_image_ppm(input, "original.ppm")) {
        std::cout << "✓ Saved original.ppm\n";
    }
    
    // Blur with different methods and time them
    float sigma = 3.0f;
    
    // Baseline
    std::cout << "\nBlurring with BASELINE method...\n";
    auto start = high_resolution_clock::now();
    gaussian_blur_baseline(input, output, sigma);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    std::cout << "  Time: " << duration << " ms\n";
    save_image_ppm(output, "blurred_baseline.ppm");
    std::cout << "✓ Saved blurred_baseline.ppm\n";
    
    // SIMD
    std::cout << "\nBlurring with SIMD method...\n";
    start = high_resolution_clock::now();
    gaussian_blur_simd(input, output, sigma);
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    std::cout << "  Time: " << duration << " ms\n";
    save_image_ppm(output, "blurred_simd.ppm");
    std::cout << "✓ Saved blurred_simd.ppm\n";
    
    // Tiled
    std::cout << "\nBlurring with TILED method...\n";
    start = high_resolution_clock::now();
    gaussian_blur_tiled(input, output, sigma);
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    std::cout << "  Time: " << duration << " ms\n";
    save_image_ppm(output, "blurred_tiled.ppm");
    std::cout << "✓ Saved blurred_tiled.ppm\n";
    
    // Multi-threaded blur requires C++11 threading support
    // Commented out for compatibility with older MinGW
    /*
    std::cout << "\nBlurring with MULTI-THREADED method...\n";
    start = high_resolution_clock::now();
    gaussian_blur_multithreaded(input, output, sigma);
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    std::cout << "  Time: " << duration << " ms\n";
    save_image_ppm(output, "blurred_multithreaded.ppm");
    std::cout << "✓ Saved blurred_multithreaded.ppm\n";
    */
    
    std::cout << "\n✓ All blurred images saved! Open them to see the results.\n";
}

void demo_aes_encryption() {
    print_header("AES-128 ENCRYPTION DEMONSTRATION");
    
    const char* message = "Hello, ARES Project! This is a secret message.";
    size_t msg_len = strlen(message);
    
    // Pad to 16-byte blocks
    size_t num_blocks = (msg_len + 15) / 16;
    size_t padded_len = num_blocks * 16;
    
    uint8_t* plaintext = new uint8_t[padded_len];
    uint8_t* ciphertext_baseline = new uint8_t[padded_len];
    uint8_t* ciphertext_simd = new uint8_t[padded_len];
    
    memcpy(plaintext, message, msg_len);
    memset(plaintext + msg_len, 0, padded_len - msg_len);
    
    uint8_t key[16] = "SecretKey123456";
    
    std::cout << "Original message: \"" << message << "\"\n";
    std::cout << "Message length: " << msg_len << " bytes (" << num_blocks << " blocks)\n\n";
    
    // Baseline encryption
    std::cout << "Encrypting with BASELINE method...\n";
    auto start = high_resolution_clock::now();
    aes_encrypt_baseline(plaintext, ciphertext_baseline, key, num_blocks);
    auto end = high_resolution_clock::now();
    auto baseline_time = duration_cast<microseconds>(end - start).count();
    
    std::cout << "  Time: " << baseline_time << " μs\n";
    std::cout << "  Ciphertext (hex): ";
    for (size_t i = 0; i < std::min(size_t(32), padded_len); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(ciphertext_baseline[i]);
    }
    std::cout << std::dec << "...\n";
    
    // SIMD encryption
    if (has_aes_ni_support()) {
        std::cout << "\nEncrypting with SIMD (AES-NI) method...\n";
        start = high_resolution_clock::now();
        aes_encrypt_simd(plaintext, ciphertext_simd, key, num_blocks);
        end = high_resolution_clock::now();
        auto simd_time = duration_cast<microseconds>(end - start).count();
        
        std::cout << "  Time: " << simd_time << " μs\n";
        std::cout << "  Ciphertext (hex): ";
        for (size_t i = 0; i < std::min(size_t(32), padded_len); ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') 
                      << static_cast<int>(ciphertext_simd[i]);
        }
        std::cout << std::dec << "...\n";
        
        if (simd_time > 0) {
            double speedup = static_cast<double>(baseline_time) / simd_time;
            std::cout << "\n✓ AES-NI Speedup: " << std::fixed << std::setprecision(2) 
                      << speedup << "x faster!\n";
        }
    } else {
        std::cout << "\n⚠ AES-NI not supported on this CPU\n";
    }
    
    delete[] plaintext;
    delete[] ciphertext_baseline;
    delete[] ciphertext_simd;
}

int main(int argc, char* argv[]) {
    std::cout << R"(
    █████╗ ██████╗ ███████╗███████╗
   ██╔══██╗██╔══██╗██╔════╝██╔════╝
   ███████║██████╔╝█████╗  ███████╗
   ██╔══██║██╔══██╗██╔══╝  ╚════██║
   ██║  ██║██║  ██║███████╗███████║
   ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚══════╝
   
   Architecture-aware Real-time
   Encryption & Signal-processing
   
   Microarchitectural Optimization Demo
)" << "\n";
    
    if (argc > 1 && std::string(argv[1]) == "aes") {
        demo_aes_encryption();
    } else if (argc > 1 && std::string(argv[1]) == "blur") {
        demo_gaussian_blur();
    } else {
        std::cout << "Usage:\n";
        std::cout << "  " << argv[0] << " blur    - Demonstrate Gaussian blur with all optimizations\n";
        std::cout << "  " << argv[0] << " aes     - Demonstrate AES encryption speed\n\n";
        
        std::cout << "Running both demos...\n";
        demo_aes_encryption();
        demo_gaussian_blur();
    }
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  Demo complete! Check the .ppm files for blurred images.\n";
    std::cout << std::string(60, '=') << "\n\n";
    
    return 0;
}
