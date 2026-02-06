# Source library
add_library(ares STATIC
    aes_baseline.cpp
    aes_simd.cpp
    gaussian_baseline.cpp
    gaussian_simd.cpp
    gaussian_tiled.cpp
    gaussian_multithreaded.cpp
    image_io.cpp
)

target_include_directories(ares PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Enable AVX2 for SIMD files
if(MSVC)
    target_compile_options(ares PRIVATE /arch:AVX2)
else()
    target_compile_options(ares PRIVATE -mavx2 -mfma -maes)
endif()
