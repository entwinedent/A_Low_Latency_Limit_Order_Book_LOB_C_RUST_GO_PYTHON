# Performance Profiling Scripts

This directory contains scripts for profiling and analyzing the performance of the Low-Latency Order Book Engine.

## Available Scripts

### CPU Profiling

#### Linux: perf_profile.sh
```bash
#!/bin/bash
# CPU profiling using Linux perf
perf record -F 99 -g -- ./build/bench/Benchmark_OrderBook
perf report
```

#### macOS: instruments_profile.sh
```bash
#!/bin/bash
# CPU profiling using macOS Instruments
instruments -t "Order Book CPU" -D profile.trace ./build/bench/Benchmark_OrderBook
instruments -l profile.trace
```

#### Windows: vtune_profile.bat
```bash
@echo off
REM CPU profiling using Intel VTune
vtune -collect hotspots -result-dir vtune_results ./build/bench/Benchmark_OrderBook.exe
vtune -report hotspots -result-dir vtune_results
```

### Memory Profiling

#### Linux: valgrind_profile.sh
```bash
#!/bin/bash
# Memory profiling using Valgrind
valgrind --tool=massif --massif-out-file=massif.out ./build/bench/Benchmark_OrderBook
ms_print massif.out
```

#### Linux: leak_check.sh
```bash
#!/bin/bash
# Memory leak detection
valgrind --leak-check=full --show-leak-kinds=all ./build/core/OrderBookTests
```

#### macOS: leaks_profile.sh
```bash
#!/bin/bash
# Memory leak detection on macOS
leaks --atExit -- ./build/bench/Benchmark_OrderBook
```

### Cache Profiling

#### Linux: cache_profile.sh
```bash
#!/bin/bash
# Cache performance analysis
perf stat -e cache-references,cache-misses,L1-dcache-loads,L1-dcache-load-misses \
  ./build/bench/Benchmark_OrderBook
```

### Flame Graphs

#### generate_flamegraph.sh
```bash
#!/bin/bash
# Generate flame graph from perf data
perf record -F 99 -g -- ./build/bench/Benchmark_OrderBook
perf script > out.perf
stackcollapse-perf.pl out.perf > out.folded
flamegraph.pl out.folded > flamegraph.svg
```

## Usage Examples

### Basic CPU Profiling
```bash
# Linux
./scripts/profiling/perf_profile.sh

# macOS
./scripts/profiling/instruments_profile.sh

# Windows
scripts\profiling\vtune_profile.bat
```

### Memory Profiling
```bash
# Linux
./scripts/profiling/valgrind_profile.sh

# macOS
./scripts/profiling/leaks_profile.sh
```

### Cache Analysis
```bash
# Linux
./scripts/profiling/cache_profile.sh
```

### Generate Flame Graph
```bash
# Linux
./scripts/profiling/generate_flamegraph.sh
# Open flamegraph.svg in browser
```

## Interpreting Results

### CPU Profiling
- **Hotspots**: Functions consuming most CPU time
- **Call Graph**: Function call hierarchy
- **Instructions**: Total instructions executed

### Memory Profiling
- **Heap Usage**: Total memory allocated
- **Allocation Rate**: Memory allocation frequency
- **Leaks**: Memory not properly freed

### Cache Profiling
- **Cache Hit Rate**: Percentage of cache hits
- **Cache Misses**: Number of cache misses
- **L1/L2/L3**: Performance at each cache level

## Optimization Workflow

1. **Baseline**: Run benchmarks without profiling
2. **Profile**: Use appropriate profiling script
3. **Analyze**: Identify bottlenecks
4. **Optimize**: Make targeted improvements
5. **Verify**: Re-run benchmarks to confirm improvement
6. **Repeat**: Continue optimization cycle

## Integration with CI

Add profiling to CI pipeline for performance regression detection:

```yaml
# .github/workflows/profiling.yml
name: Performance Profiling
on: [push, pull_request]
jobs:
  profile:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build
        run: |
          cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
          cmake --build build --config Release
      - name: Profile
        run: ./scripts/profiling/perf_profile.sh
      - name: Upload Results
        uses: actions/upload-artifact@v2
        with:
          name: profiling-results
          path: profiling/
```

## Custom Profiling

### Custom Metrics
```bash
# Add custom perf events
perf stat -e cycles,instructions,cache-references,cache-misses \
  ./build/bench/Benchmark_OrderBook
```

### Duration-based Profiling
```bash
# Profile for specific duration
perf record -F 99 -g -- sleep 10 &
PID=$!
./build/bench/Benchmark_OrderBook
kill -INT $PID
```

### Filter by Function
```bash
# Profile specific function
perf record -F 99 -g --filter='OrderBook::add_limit_order' \
  ./build/bench/Benchmark_OrderBook
```

## Requirements

### Linux
- perf-tools (linux-tools-generic)
- Valgrind
- Flame graph tools (optional)

### macOS
- Xcode Command Line Tools
- Instruments (included with Xcode)

### Windows
- Intel VTune Profiler (optional)
- Visual Studio Profiler (included with VS)

## Troubleshooting

### Permission Denied (Linux)
```bash
# Adjust perf permissions
echo 0 | sudo tee /proc/sys/kernel/perf_event_paranoid
```

### VTune Not Found (Windows)
- Install Intel VTune Profiler
- Add to system PATH
- Ensure Intel drivers are installed

### Instruments Not Available (macOS)
- Install Xcode from App Store
- Install Command Line Tools: `xcode-select --install`

## References

- [Linux perf Wiki](https://perf.wiki.kernel.org/)
- [Valgrind Manual](https://valgrind.org/docs/manual/)
- [Intel VTune Documentation](https://www.intel.com/content/www/us/en/develop/documentation/vtune-help/)
- [macOS Instruments](https://help.apple.com/instruments/mac/)