# Benchmarking configuration

# Benchmark executable
add_executable(benchmark_order_book
    Benchmark_OrderBook.cpp
)

target_link_libraries(benchmark_order_book
    PRIVATE
        lob_core_impl
        benchmark::benchmark
        benchmark::benchmark_main
)

# Profiling executable
add_executable(profile_latency
    Profile_Latency.cpp
)

target_link_libraries(profile_latency
    PRIVATE
        lob_core_impl
)
