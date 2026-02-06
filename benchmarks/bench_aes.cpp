#include "ares/aes.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

using namespace ares;
using namespace std::chrono;

// Simple benchmark class
class Benchmark {
public:
    template<typename Func>
    static double measure(Func func, int iterations = 100) {
        auto start = high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            func();
        }
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start).count();
        return duration / static_cast<double>(iterations);
    }
};

void benchmark_aes(size_t data_size_kb) {
    size_t num_blocks = (data_size_kb * 1024) / 16;
    std::vector<uint8_t> plaintext(num_blocks * 16);
    std::vector<uint8_t> ciphertext(num_blocks * 16);
    uint8_t key[16] = "BenchmarkKey123";
    
    // Fill with random data
    for (size_t i = 0; i < plaintext.size(); ++i) {
        plaintext[i] = static_cast<uint8_t>(rand() % 256);
    }
    
    // Benchmark baseline
    double baseline_time = Benchmark::measure([&]() {
        aes_encrypt_baseline(plaintext.data(), ciphertext.data(), key, num_blocks);
    }, 50);
    
    double baseline_throughput = (data_size_kb * 1024.0) / (baseline_time / 1000000.0) / (1024 * 1024);
    
    printf("  Baseline:  %8.2f μs  |  %6.2f MB/s\n", 
           baseline_time, baseline_throughput);
    
    // Benchmark SIMD (if available)
    if (has_aes_ni_support()) {
        double simd_time = Benchmark::measure([&]() {
            aes_encrypt_simd(plaintext.data(), ciphertext.data(), key, num_blocks);
        }, 50);
        
        double simd_throughput = (data_size_kb * 1024.0) / (simd_time / 1000000.0) / (1024 * 1024);
        double speedup = baseline_time / simd_time;
        
        printf("  SIMD:      %8.2f μs  |  %6.2f MB/s  |  %.2fx speedup\n",
               simd_time, simd_throughput, speedup);
    } else {
        printf("  SIMD:      [AES-NI not supported]\n");
    }
}

int main() {
    printf("=== ARES AES Encryption Benchmarks ===\n\n");
    printf("Testing AES-128 encryption performance\n");
    printf("Format: Time per operation | Throughput | Speedup\n\n");
    
    printf("Data Size: 4 KB\n");
    benchmark_aes(4);
    
    printf("\nData Size: 64 KB\n");
    benchmark_aes(64);
    
    printf("\nData Size: 1 MB\n");
    benchmark_aes(1024);
    
    printf("\nData Size: 10 MB\n");
    benchmark_aes(10240);
    
    printf("\n=== Benchmark Complete ===\n");
    printf("\nNotes:\n");
    printf("- SIMD version uses AES-NI hardware instructions\n");
    printf("- Speedup shows performance improvement over baseline\n");
    printf("- Results may vary based on CPU model and clock speed\n");
    
    return 0;
}
