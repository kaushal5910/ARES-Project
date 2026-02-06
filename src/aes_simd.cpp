#include "ares/aes.hpp"
#include <immintrin.h>
#include <wmmintrin.h>
#include <cstring>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif

namespace ares {

bool has_aes_ni_support() {
#ifdef _MSC_VER
    int cpu_info[4];
    __cpuid(cpu_info, 1);
    return (cpu_info[2] & (1 << 25)) != 0; // Check ECX bit 25
#else
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        return (ecx & bit_AES) != 0;
    }
    return false;
#endif
}

// Key expansion using AES-NI
static void expand_key_aesni(const uint8_t* key, __m128i* round_keys) {
    __m128i key_schedule[11];
    key_schedule[0] = _mm_loadu_si128(reinterpret_cast<const __m128i*>(key));
    
    // Helper macro for key expansion
    #define AES_128_key_exp(k, rcon) \
        _mm_xor_si128(_mm_shuffle_epi32(_mm_aeskeygenassist_si128(k, rcon), 0xff), k)
    
    // Simplified key expansion for demonstration
    // In production, you'd use proper AES-128 key schedule
    __m128i temp = key_schedule[0];
    round_keys[0] = temp;
    
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(temp, 0x01), 0xff));
    round_keys[1] = temp;
    
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(temp, 0x02), 0xff));
    round_keys[2] = temp;
    
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(temp, 0x04), 0xff));
    round_keys[3] = temp;
    
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(temp, 0x08), 0xff));
    round_keys[4] = temp;
    
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(temp, 0x10), 0xff));
    round_keys[5] = temp;
    
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(temp, 0x20), 0xff));
    round_keys[6] = temp;
    
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(temp, 0x40), 0xff));
    round_keys[7] = temp;
    
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(temp, 0x80), 0xff));
    round_keys[8] = temp;
    
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(temp, 0x1b), 0xff));
    round_keys[9] = temp;
    
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4));
    temp = _mm_xor_si128(temp, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(temp, 0x36), 0xff));
    round_keys[10] = temp;
    
    #undef AES_128_key_exp
}

void aes_encrypt_simd(
    const uint8_t* plaintext,
    uint8_t* ciphertext,
    const uint8_t* key,
    size_t num_blocks
) {
    // Expand the key into round keys using AES-NI
    __m128i round_keys[11];
    expand_key_aesni(key, round_keys);
    
    // Process each block using hardware AES instructions
    for (size_t block = 0; block < num_blocks; ++block) {
        // Load plaintext block
        __m128i state = _mm_loadu_si128(
            reinterpret_cast<const __m128i*>(plaintext + block * 16)
        );
        
        // Initial round: XOR with first round key
        state = _mm_xor_si128(state, round_keys[0]);
        
        // 9 main rounds using AESENC instruction
        // Each AESENC does: ShiftRows + SubBytes + MixColumns + AddRoundKey
        state = _mm_aesenc_si128(state, round_keys[1]);
        state = _mm_aesenc_si128(state, round_keys[2]);
        state = _mm_aesenc_si128(state, round_keys[3]);
        state = _mm_aesenc_si128(state, round_keys[4]);
        state = _mm_aesenc_si128(state, round_keys[5]);
        state = _mm_aesenc_si128(state, round_keys[6]);
        state = _mm_aesenc_si128(state, round_keys[7]);
        state = _mm_aesenc_si128(state, round_keys[8]);
        state = _mm_aesenc_si128(state, round_keys[9]);
        
        // Final round using AESENCLAST (no MixColumns)
        state = _mm_aesenclast_si128(state, round_keys[10]);
        
        // Store ciphertext block
        _mm_storeu_si128(
            reinterpret_cast<__m128i*>(ciphertext + block * 16),
            state
        );
    }
}

} // namespace ares
