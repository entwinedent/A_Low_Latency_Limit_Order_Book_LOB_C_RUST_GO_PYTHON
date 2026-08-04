# Benchmarking Guide

## Overview

This guide covers benchmarking the order book engine to measure and track performance.

## Google Benchmark

### Building Benchmarks

Enable benchmark building in CMake:

```bash
cmake -B build -S . -DBUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Running Benchmarks

```bash
# Run all benchmarks
./build/bench/Benchmark_OrderBook

# Run specific benchmark
./build/bench/Benchmark_OrderBook --benchmark_filter=AddBuyOrder

# Output JSON for analysis
./build/bench/Benchmark_OrderBook --benchmark_format=json > results.json
```

### Benchmark Options

```bash
--benchmark_repetitions=N      # Run N times
--benchmark_min_time=N        # Minimum time per benchmark
--benchmark_filter=REGEX      # Filter benchmarks
--benchmark_list              # List available benchmarks
```

## Custom Benchmarks

### Latency Measurement

Measure single-operation latency:

```cpp
#include <chrono>

auto start = std::chrono::high_resolution_clock::now();
book.add_limit_order(1, 100.0, 10, Side::BUY);
auto end = std::chrono::high_resolution_clock::now();
auto latency_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
```

### Throughput Measurement

Measure operations per second:

```cpp
auto start = std::chrono::high_resolution_clock::now();
for (int i = 0; i < N; ++i) {
    book.add_limit_order(i, 100.0 + i, 10, Side::BUY);
}
auto end = std::chrono::high_resolution_clock::now();
auto duration_sec = std::chrono::duration<double>(end - start).count();
double ops_per_sec = N / duration_sec;
```

### Percentile Latency

Measure P50, P95, P99 latencies:

```cpp
std::vector<uint64_t> latencies;
latencies.reserve(N);

for (int i = 0; i < N; ++i) {
    auto start = std::chrono::high_resolution_clock::now();
    book.add_limit_order(i, 100.0, 10, Side::BUY);
    auto end = std::chrono::high_resolution_clock::now();
    latencies.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

std::sort(latencies.begin(), latencies.end());
uint64_t p50 = latencies[N * 50 / 100];
uint64_t p95 = latencies[N * 95 / 100];
uint64_t p99 = latencies[N * 99 / 100];
```

## Benchmark Scenarios

### 1. Order Addition

```cpp
BENCHMARK(BM_AddBuyOrder)(benchmark::State& state) {
    OrderBook book;
    uint64_t id = 1;
    for (auto _ : state) {
        book.add_limit_order(id++, 100.0, 10, Side::BUY);
    }
}
```

### 2. Order Cancellation

```cpp
BENCHMARK(BM_CancelOrder)(benchmark::State& state) {
    OrderBook book;
    // Pre-populate
    for (uint64_t i = 1; i <= 10000; ++i) {
        book.add_limit_order(i, 100.0 + i, 10, Side::BUY);
    }
    
    uint64_t id = 1;
    for (auto _ : state) {
        book.cancel_order(id++);
    }
}
```

### 3. Order Matching

```cpp
BENCHMARK(BM_Matching)(benchmark::State& state) {
    OrderBook book;
    // Pre-populate bids and asks
    for (uint64_t i = 1; i <= 5000; ++i) {
        book.add_limit_order(i, 100.0 + i * 0.01, 10, Side::BUY);
        book.add_limit_order(i + 5000, 105.0 + i * 0.01, 10, Side::SELL);
    }
    
    for (auto _ : state) {
        book.add_limit_order(10001, 103.0, 10, Side::BUY);
    }
}
```

### 4. Market Data Queries

```cpp
BENCHMARK(BM_QueryBestBid)(benchmark::State& state) {
    OrderBook book;
    for (uint64_t i = 1; i <= 10000; ++i) {
        book.add_limit_order(i, 100.0 + i * 0.01, 10, Side::BUY);
    }
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(book.get_best_bid());
    }
}
```

## Profiling Integration

### perf (Linux)

```bash
# Profile benchmark
perf record ./build/bench/Benchmark_OrderBook

# Analyze
perf report

# Annotate source
perf annotate
```

### VTune (Windows)

```bash
# Hotspots analysis
vtune -collect hotspots -result-dir vtune_results ./build/bench/Benchmark_OrderBook.exe

# View results
vtune -report hotspots -result-dir vtune_results
```

## Regression Testing

### CI Integration

Add to GitHub Actions:

```yaml
- name: Run Benchmarks
  run: |
    cmake --build build --config Release
    ./build/bench/Benchmark_OrderBook --benchmark_format=json > benchmark_results.json
    
- name: Check Performance Regression
  run: |
    python scripts/check_regression.py benchmark_results.json baseline.json
```

### Baseline Tracking

Store baseline results:

```json
{
  "AddBuyOrder": {
    "mean": 25.5,
    "stddev": 1.2,
    "median": 25.0
  }
}
```

### Regression Threshold

Alert if performance degrades > 5%:

```python
if current_mean > baseline_mean * 1.05:
    print("PERFORMANCE REGRESSION DETECTED")
    exit(1)
```

## Environment Setup

### Disable CPU Scaling

```bash
# Linux
sudo cpupower frequency-set -g performance

# Windows
powercfg /setactive scheme_min
```

### Disable Turbo Boost

For consistent measurements:

```bash
# Linux
echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo

# Windows (BIOS setting)
```

### Pin Process to Core

```bash
# Linux
taskset -c 0 ./build/bench/Benchmark_OrderBook

# Windows (PowerShell)
$process = Start-Process -FilePath ".\build\bench\Benchmark_OrderBook.exe" -PassThru
$process.ProcessorAffinity = 1
```

## Interpreting Results

### Mean vs Median

- **Mean**: Sensitive to outliers
- **Median**: Robust to outliers
- **P95/P99**: Tail latency critical for HFT

### Standard Deviation

- Low stddev: Consistent performance
- High stddev: Possible interference or cache effects

### Iterations

- More iterations = more accurate
- Minimum 1M iterations for nanosecond precision
- Use `--benchmark_repetitions=10` for statistical significance

## Common Issues

### 1. High Variance

**Causes**:
- CPU frequency scaling
- Background processes
- Cache pollution

**Solutions**:
- Disable frequency scaling
- Isolate CPU core
- Warm-up runs

### 2. Unexpected Latency Spikes

**Causes**:
- Page faults
- Context switches
- Interrupts

**Solutions**:
- Lock memory pages (mlock)
- Real-time priority
- Disable interrupts (not recommended)

### 3. Compiler Optimizations Not Applied

**Causes**:
- Debug build
- Missing optimization flags
- Inline assembly blocking optimization

**Solutions**:
- Use Release build
- Check compiler flags
- Review generated assembly
