# Benchmark executables
add_executable(bench_aes bench_aes.cpp)
target_link_libraries(bench_aes ares)

add_executable(bench_gaussian bench_gaussian.cpp)
target_link_libraries(bench_gaussian ares)
