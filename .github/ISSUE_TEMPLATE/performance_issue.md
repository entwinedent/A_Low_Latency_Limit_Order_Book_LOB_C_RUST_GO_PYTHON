---
name: Performance issue
about: Report performance-related issues or regressions
title: '[PERFORMANCE] '
labels: performance
assignees: ''
---

## Performance Issue Description
A clear description of the performance problem you're experiencing.

## Expected Performance
What performance metrics do you expect? (e.g., latency, throughput, memory usage)

## Actual Performance
What are the actual performance metrics you're observing?

## Benchmark Results
```bash
# Include benchmark output
./build/bench/Benchmark_OrderBook --benchmark_format=json
```

## Environment
- **OS**: [e.g., Ubuntu 22.04, Windows 11, macOS 13]
- **Compiler**: [e.g., GCC 13.2, MSVC 19.35, Clang 15]
- **C++ Standard**: [e.g., C++20, C++23]
- **Build Type**: [e.g., Debug, Release]
- **CPU**: [e.g., Intel i7-12700K, AMD Ryzen 9 5950X]
- **Memory**: [e.g., 32GB DDR4]

## Test Scenario
Describe the specific scenario or workload that exhibits the performance issue:
- Order rate (orders/second)
- Order book size
- Number of symbols/instruments
- Duration of test

## Code Sample
```cpp
// Include code that demonstrates the performance issue
```

## Profiling Data
If available, include profiling data (perf, VTune, etc.)

## Additional Context
- **C++23 Features**: Are you using C++23 features that might impact performance?
- **Optimizations**: Are compiler optimizations enabled?
- **Sanitizers**: Are you running with sanitizers (ASAN, UBSAN)?
