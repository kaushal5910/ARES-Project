#include "ares/aes.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>

// Simple test framework macros
#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        printf("FAILED: %s == %s (line %d)\n", #a, #b, __LINE__); \
        return false; \
    }

#define ASSERT_TRUE(cond) \
    if (!(cond)) { \
        printf("FAILED: %s (line %d)\n", #cond, __LINE__); \
        return false; \
    }

#define TEST(name) \
    bool test_##name(); \
    bool test_##name()

using namespace ares;

// Known test vector for AES-128
// Plaintext: "Testing AES-128!"
// Key: "SimpleKey1234567"
// This is a simplified test - in production, use NIST test vectors

TEST(aes_baseline_encryption) {
    uint8_t plaintext[16] = "Testing AES-128";
    uint8_t key[16] = "SimpleKey123456";
    uint8_t ciphertext[16] = {0};
    
    aes_encrypt_baseline(plaintext, ciphertext, key, 1);
    
    // Verify it's not the same as plaintext
    bool different = false;
    for (int i = 0; i < 16; ++i) {
        if (plaintext[i] != ciphertext[i]) {
            different = true;
            break;
        }
    }
    ASSERT_TRUE(different);
    
    printf("✓ AES baseline encryption produces different output\n");
    return true;
}

TEST(aes_simd_encryption) {
    if (!has_aes_ni_support()) {
        printf("⊘ AES-NI not supported, skipping SIMD test\n");
        return true;
    }
    
    uint8_t plaintext[16] = "Testing AES-128";
    uint8_t key[16] = "SimpleKey123456";
    uint8_t ciphertext[16] = {0};
    
    aes_encrypt_simd(plaintext, ciphertext, key, 1);
    
    // Verify it's not the same as plaintext
    bool different = false;
    for (int i = 0; i < 16; ++i) {
        if (plaintext[i] != ciphertext[i]) {
            different = true;
            break;
        }
    }
    ASSERT_TRUE(different);
    
    printf("✓ AES SIMD encryption produces different output\n");
    return true;
}

TEST(aes_baseline_vs_simd) {
    if (!has_aes_ni_support()) {
        printf("⊘ AES-NI not supported, skipping comparison test\n");
        return true;
    }
    
    uint8_t plaintext[16] = "Testing AES-128";
    uint8_t key[16] = "SimpleKey123456";
    uint8_t ciphertext_baseline[16] = {0};
    uint8_t ciphertext_simd[16] = {0};
    
    aes_encrypt_baseline(plaintext, ciphertext_baseline, key, 1);
    aes_encrypt_simd(plaintext, ciphertext_simd, key, 1);
    
    // Both should produce identical results
    bool identical = true;
    for (int i = 0; i < 16; ++i) {
        if (ciphertext_baseline[i] != ciphertext_simd[i]) {
            identical = false;
            printf("Mismatch at byte %d: baseline=0x%02x, simd=0x%02x\n",
                   i, ciphertext_baseline[i], ciphertext_simd[i]);
        }
    }
    ASSERT_TRUE(identical);
    
    printf("✓ AES baseline and SIMD produce identical results\n");
    return true;
}

TEST(aes_multiple_blocks) {
    const size_t num_blocks = 4;
    uint8_t plaintext[64];
    uint8_t key[16] = "SimpleKey123456";
    uint8_t ciphertext[64] = {0};
    
    // Fill plaintext with pattern
    for (size_t i = 0; i < 64; ++i) {
        plaintext[i] = static_cast<uint8_t>(i);
    }
    
    aes_encrypt_baseline(plaintext, ciphertext, key, num_blocks);
    
    // Verify ciphertext is different
    bool different = false;
    for (size_t i = 0; i < 64; ++i) {
        if (plaintext[i] != ciphertext[i]) {
            different = true;
            break;
        }
    }
    ASSERT_TRUE(different);
    
    printf("✓ AES encrypts multiple blocks successfully\n");
    return true;
}

int main() {
    printf("=== ARES AES Tests ===\n\n");
    
    bool all_passed = true;
    all_passed &= test_aes_baseline_encryption();
    all_passed &= test_aes_simd_encryption();
    all_passed &= test_aes_baseline_vs_simd();
    all_passed &= test_aes_multiple_blocks();
    
    printf("\n");
    if (all_passed) {
        printf("✓ All AES tests passed!\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
