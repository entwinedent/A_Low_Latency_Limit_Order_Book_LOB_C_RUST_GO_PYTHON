# Profiling Guide

## Overview

This guide covers profiling tools and techniques for analyzing the order book engine's performance.

## Profiling Tools

### Linux

#### perf

**CPU Profiling**:
```bash
# Record CPU cycles
perf record -e cycles ./build/apps/lob_cli

# Analyze results
perf report

# Annotate source code
perf annotate
```

**Cache Misses**:
```bash
# Record cache misses
perf record -e cache-misses ./build/apps/lob_cli

# View cache statistics
perf stat -e cache-references,cache-misses ./build/apps/lob_cli
```

**Branch Mispredictions**:
```bash
# Record branch predictions
perf record -e branches,branch-misses ./build/apps/lob_cli

# View branch statistics
perf stat -e branches,branch-misses ./build/apps/lob_cli
```

#### valgrind

**Cache Analysis**:
```bash
valgrind --tool=cachegrind ./build/apps/lob_cli

# View results
cg_annotate cachegrind.out.<pid>
```

**Call Graph**:
```bash
valgrind --tool=callgrind ./build/apps/lob_cli

# View results
kcachegrind callgrind.out.<pid>
```

### Windows

#### VTune Profiler

**Hotspots Analysis**:
```bash
vtune -collect hotspots -result-dir vtune_results ./build/apps/lob_cli.exe
vtune -report hotspots -result-dir vtune_results
```

**Microarchitecture Exploration**:
```bash
vtune -collect uarchexplan -result-dir vtune_results ./build/apps/lob_cli.exe
```

**Memory Access Analysis**:
```bash
vtune -collect memory-access -result-dir vtune_results ./build/apps/lob_cli.exe
```

#### Windows Performance Analyzer (WPA)

1. Record trace:
```powershell
wpr -start generalprofile
.\build\apps\lob_cli.exe
wpr -stop generalprofile.etl
```

2. Open in WPA and analyze

### macOS

#### Instruments

**Time Profiler**:
1. Open Instruments
2. Select "Time Profiler"
3. Choose target executable
4. Record and analyze

**Allocations**:
1. Select "Allocations"
2. Record memory allocations
3. Identify hot allocation points

## Profiling Scenarios

### 1. Identify Hot Functions

**Goal**: Find functions consuming most CPU time

**Linux**:
```bash
perf record -g ./build/apps/lob_cli
perf report --stdio
```

**Windows**:
```bash
vtune -collect hotspots -result-dir results ./build/apps/lob_cli.exe
vtune -report hotspots -result-dir results --report-output hotspot.txt
```

### 2. Analyze Cache Behavior

**Goal**: Identify cache misses and optimize data layout

**Linux**:
```bash
perf stat -e cache-references,cache-misses,L1-dcache-loads,L1-dcache-load-misses ./build/apps/lob_cli
```

**Windows**:
```bash
vtune -collect memory-access -result-dir results ./build/apps/lob_cli.exe
```

### 3. Measure Branch Prediction

**Goal**: Identify branch mispredictions

**Linux**:
```bash
perf stat -e branches,branch-misses ./build/apps/lob_cli
```

**Windows**:
```bash
vtune -collect uarchexplan -result-dir results ./build/apps/lob_cli.exe
```

### 4. Memory Allocation Analysis

**Goal**: Find unexpected heap allocations

**Linux**:
```bash
valgrind --tool=massif ./build/apps/lob_cli
ms_print massif.out.<pid>
```

**Windows**:
```bash
# Use Visual Studio's memory profiler
devenv /debugexe .\build\apps\lob_cli.exe
```

## Flame Graphs

### Generating Flame Graphs

**Linux**:
```bash
# Record with perf
perf record -F 99 -g ./build/apps/lob_cli

# Generate flame graph
perf script | FlameGraph/stackcollapse-perf.pl | FlameGraph/flamegraph.pl > flamegraph.svg
```

**Interpretation**:
- Width = CPU time
- Height = call stack depth
- Identify wide functions for optimization

## Custom Instrumentation

### Manual Timing

```cpp
#include <chrono>

class Timer {
    std::chrono::high_resolution_clock::time_point start_;
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}
    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_);
        std::cout << "Duration: " << duration.count() << " ns\n";
    }
};

// Usage
{
    Timer t;
    book.add_limit_order(1, 100.0, 10, Side::BUY);
} // Prints duration on scope exit
```

### Statistics Collection

```cpp
#include <atomic>
#include <iostream>

class Stats {
    std::atomic<uint64_t> count_;
    std::atomic<uint64_t> total_ns_;
public:
    void record(uint64_t ns) {
        count_++;
        total_ns_ += ns;
    }
    void print() {
        std::cout << "Count: " << count_ << ", Mean: " << (total_ns_ / count_) << " ns\n";
    }
};
```

## Optimization Workflow

### 1. Profile

Run profiler to identify hotspots:
```bash
perf record -g ./build/apps/lob_cli
perf report
```

### 2. Analyze

Examine hot functions:
- Look for cache misses
- Check branch mispredictions
- Identify allocation patterns

### 3. Optimize

Apply optimizations:
- Improve cache locality
- Add branch hints
- Eliminate allocations

### 4. Verify

Re-profile to confirm improvement:
```bash
perf record -g ./build/apps/lob_cli
perf report
```

### 5. Iterate

Repeat until targets met

## Common Performance Issues

### 1. Cache Misses

**Symptoms**: High cache-miss rate

**Solutions**:
- Improve data locality
- Align structures to cache lines
- Use contiguous memory

### 2. Branch Mispredictions

**Symptoms**: High branch-miss rate

**Solutions**:
- Add [[likely]]/[[unlikely]] hints
- Use branchless algorithms
- Reorder conditions

### 3. Memory Allocations

**Symptoms**: Unexpected heap allocations

**Solutions**:
- Use memory pools
- Stack allocation for temporaries
- Reserve capacity upfront

### 4. False Sharing

**Symptoms**: Performance degradation with multiple threads

**Solutions**:
- Align to cache lines
- Pad structures
- Use thread-local storage

## Continuous Profiling

### Production Monitoring

For production deployments:

1. **Sampling Profiler**: Low overhead profiling
2. **Metrics**: Track latency percentiles
3. **Alerts**: Notify on performance degradation

### Integration

Add profiling hooks:

```cpp
#ifdef ENABLE_PROFILING
PROFILE_SCOPE("add_limit_order");
#endif
ErrorCode add_limit_order(...) {
    // Implementation
}
```

## Best Practices

1. **Profile Before Optimizing**: Don't guess, measure
2. **Use Release Builds**: Optimizations only apply in Release
3. **Representative Workload**: Profile with realistic data
4. **Multiple Runs**: Account for variance
5. **Isolate Environment**: Minimize background noise
6. **Document Results**: Track performance over time
