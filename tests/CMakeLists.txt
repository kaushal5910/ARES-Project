# Test executables
add_executable(test_aes test_aes.cpp)
target_link_libraries(test_aes ares)

add_executable(test_gaussian test_gaussian.cpp)
target_link_libraries(test_gaussian ares)

# Add tests to CTest
enable_testing()
add_test(NAME AES_Tests COMMAND test_aes)
add_test(NAME Gaussian_Tests COMMAND test_gaussian)
