#pragma once

#include <cstdint>
#include <cstddef>
#include <array>

namespace ares {

/**
 * @brief AES-128 encryption using baseline C++ implementation
 * 
 * This is the reference implementation using standard loops and lookup tables.
 * Serves as the baseline for comparison with SIMD-optimized version.
 * 
 * @param plaintext Input data (must be 16-byte aligned blocks)
 * @param ciphertext Output encrypted data
 * @param key 128-bit encryption key
 * @param num_blocks Number of 16-byte blocks to encrypt
 */
void aes_encrypt_baseline(
    const uint8_t* plaintext,
    uint8_t* ciphertext,
    const uint8_t* key,
    size_t num_blocks
);

/**
 * @brief AES-128 encryption using AES-NI hardware intrinsics
 * 
 * This implementation leverages Intel/AMD AES-NI instructions for
 * hardware-accelerated encryption. Requires CPU support for AES-NI.
 * 
 * @param plaintext Input data (must be 16-byte aligned)
 * @param ciphertext Output encrypted data
 * @param key 128-bit encryption key
 * @param num_blocks Number of 16-byte blocks to encrypt
 */
void aes_encrypt_simd(
    const uint8_t* plaintext,
    uint8_t* ciphertext,
    const uint8_t* key,
    size_t num_blocks
);

/**
 * @brief Check if CPU supports AES-NI instructions
 * @return true if AES-NI is available, false otherwise
 */
bool has_aes_ni_support();

} // namespace ares
