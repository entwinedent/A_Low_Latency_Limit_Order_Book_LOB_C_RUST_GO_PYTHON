# Performance Optimization Guide

## Latency Targets

The order book engine targets sub-microsecond latency for all operations:

| Operation      | Target Latency | Current Status |
|----------------|----------------|----------------|
| Add Order      | < 50 ns        | TBD            |
| Cancel Order   | < 30 ns        | TBD            |
| Match Order    | < 100 ns       | TBD            |
| Query Best Bid | < 20 ns        | TBD            |
| Query Best Ask | < 20 ns        | TBD            |

## Optimization Techniques

### 1. Zero-Allocation Design

**Principle**: Eliminate all heap allocations in hot paths

**Implementation**:
- Pre-allocated memory pools
- Stack-based temporary storage
- Placement new for object construction

**Impact**: Eliminates allocator overhead and fragmentation

### 2. Cache-Line Alignment

**Principle**: Align data to 64-byte cache lines

**Implementation**:
```cpp
struct alignas(64) Order {
    // Order fields
};
```

**Impact**: Prevents false sharing, improves cache utilization

### 3. Branch Prediction Hints

**Principle**: Help the CPU predict branches correctly

**Implementation**:
```cpp
if (free_list_.empty()) [[unlikely]] {
    return nullptr;
}
if (order->is_filled()) [[likely]] {
    // Handle filled order
}
```

**Impact**: Reduces branch misprediction penalties

### 4. Inline Functions

**Principle**: Eliminate function call overhead in hot paths

**Implementation**:
```cpp
inline bool is_filled() const {
    return quantity == 0;
}
```

**Impact**: Zero function call overhead

### 5. Template Specialization

**Principle**: Generate optimized code for specific types

**Implementation**:
```cpp
template <typename T, size_t Capacity>
class MemoryPool {
    // Specialized for known capacities
};
```

**Impact**: Compiler can optimize for known constants

### 6. Compiler Optimizations

**Flags for maximum performance**:

```bash
# GCC/Clang
-O3 -march=native -flto -ffast-math

# MSVC
/O2 /arch:AVX2 /GL /LTCG
```

**Impact**: Enables all optimizations, targets specific CPU

## Profiling

### Tools

**Linux**:
- `perf`: CPU profiling
- `valgrind --tool=cachegrind`: Cache analysis
- `perf record`: Sampling profiler

**Windows**:
- VTune: Intel's profiler
- Windows Performance Analyzer (WPA)
- Visual Studio Profiler

**macOS**:
- Instruments: Apple's profiler
- `sample`: Command-line sampling

### Hotspots

Identify and optimize:
1. **Order allocation**: Should be O(1) from pool
2. **Price level lookup**: Should be O(log N) or better
3. **Order matching**: Should be linear in matches
4. **Hash map operations**: Should be O(1) average

## Micro-Optimizations

### 1. Prefer References

```cpp
// Bad: Copy
void process(Order order);

// Good: Reference
void process(const Order& order);
```

### 2. Move Semantics

```cpp
// Bad: Copy
std::vector<Order> orders = get_orders();

// Good: Move
std::vector<Order> orders = std::move(get_orders());
```

### 3. Reserve Capacity

```cpp
// Bad: Reallocations
std::vector<Order*> orders;
for (int i = 0; i < 1000; ++i) {
    orders.push_back(get_order());
}

// Good: Single allocation
std::vector<Order*> orders;
orders.reserve(1000);
for (int i = 0; i < 1000; ++i) {
    orders.push_back(get_order());
}
```

### 4. Avoid Virtual Functions

```cpp
// Bad: Virtual call overhead
class Order {
    virtual void match() = 0;
};

// Good: Compile-time dispatch
template <typename OrderType>
void match_order(OrderType& order);
```

## SIMD Optimization

### AVX2 Vectorization

Vectorize price comparisons:

```cpp
#include <immintrin.h>

// Compare 4 prices simultaneously
__m256d prices = _mm256_load_pd(price_array);
__m256d target = _mm256_set1_pd(target_price);
__m256d cmp = _mm256_cmp_pd(prices, target, _MM_CMPINT_LT);
```

### Use Cases

- Price level aggregation
- Bulk order validation
- Market data processing

## NUMA Optimization

### Local Memory Access

```cpp
// Linux: Allocate on local NUMA node
void* ptr = numa_alloc_local(size);

// Windows: Use NUMA-aware allocation
VirtualAllocExNuma(...);
```

### Thread Affinity

Pin threads to local cores:

```cpp
// Linux
cpu_set_t cpuset;
CPU_ZERO(&cpuset);
CPU_SET(core_id, &cpuset);
pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);

// Windows
SetThreadAffinityMask(GetCurrentThread(), 1ULL << core_id);
```

## Benchmarking

### Google Benchmark

Run micro-benchmarks:

```bash
./build/bench/Benchmark_OrderBook
```

### Custom Benchmarks

Measure specific scenarios:

```cpp
auto start = std::chrono::high_resolution_clock::now();
// Operation to measure
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
```

### Regression Testing

Track performance over time:

- CI runs benchmarks on every commit
- Alert on performance regressions > 5%
- Maintain performance history

## Common Pitfalls

### 1. Premature Optimization

**Problem**: Optimizing before measuring

**Solution**: Profile first, optimize hotspots

### 2. Micro-Optimizations at Wrong Level

**Problem**: Optimizing code that's not on the hot path

**Solution**: Focus on operations called millions of times per second

### 3. Ignoring Memory Bandwidth

**Problem**: CPU-bound optimizations ignore memory limits

**Solution**: Consider cache hierarchy and memory bandwidth

### 4. Over-Optimizing

**Problem**: Code becomes unreadable for minimal gains

**Solution**: Balance performance with maintainability

## Performance Checklist

Before deploying to production:

- [ ] All hot paths have zero allocations
- [ ] Data structures are cache-line aligned
- [ ] Branch prediction hints are used appropriately
- [ ] Compiler optimizations are enabled
- [ ] Profiling has identified true hotspots
- [ ] SIMD is used where beneficial
- [ ] NUMA awareness is enabled on multi-socket systems
- [ ] Benchmarks show sub-microsecond latency
- [ ] Performance regression tests are in place
- [ ] Memory footprint is within limits
