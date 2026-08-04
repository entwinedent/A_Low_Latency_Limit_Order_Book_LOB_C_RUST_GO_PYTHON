# Troubleshooting Guide

This guide helps you diagnose and resolve common issues with the Low-Latency Order Book Engine.

## Build Issues

### CMake Configuration Fails

#### Issue: CMake not found
```
Error: CMake is required but not found
```

**Solution:**
```bash
# Install CMake
# Windows: Download from https://cmake.org/download/
# Linux: sudo apt-get install cmake
# macOS: brew install cmake

# Verify installation
cmake --version
```

#### Issue: C++ compiler not found
```
Error: No suitable C++ compiler found
```

**Solution:**
```bash
# Windows: Install Visual Studio with C++ workload
# Linux: sudo apt-get install build-essential
# macOS: xcode-select --install

# Verify installation
# Windows: cl
# Linux: gcc --version
# macOS: clang --version
```

#### Issue: CMake version too old
```
Error: CMake version 3.20 or higher required
```

**Solution:**
```bash
# Update CMake to latest version
# Download from https://cmake.org/download/
```

### Build Errors

#### Issue: Compilation errors in C++ code
```
Error: 'memory_pool' was not declared in this scope
```

**Solution:**
- Ensure all source files are included in CMakeLists.txt
- Check include paths are correct
- Verify header files exist in expected locations

#### Issue: Linker errors
```
Error: undefined reference to 'OrderBook::add_limit_order'
```

**Solution:**
- Ensure all source files are compiled
- Check library linking order in CMakeLists.txt
- Verify implementation files are included

#### Issue: MSVC compatibility errors
```
Error: macro redefinition in fmt/spdlog headers
```

**Solution:**
- Ensure MSVC compatibility fixes are applied
- Check bundled headers are properly configured
- Update to latest version if using older headers

## Runtime Errors

### Segmentation Faults

#### Issue: Segfault on order addition
```
Segmentation fault (core dumped)
```

**Solution:**
```bash
# Run with AddressSanitizer
cmake -B build-san -S . -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON
cmake --build build-san --config Debug
./build-san/core/OrderBookTests

# Check for memory issues
valgrind --leak-check=full ./build/core/OrderBookTests
```

#### Issue: Memory access violations
```
Error: Access violation reading location 0x...
```

**Solution:**
- Enable debug symbols: `-DCMAKE_BUILD_TYPE=Debug`
- Run with debugger (gdb, lldb, Visual Studio debugger)
- Check array bounds and pointer usage

### Performance Issues

#### Issue: Slower than expected performance
```
Performance: 1000ns per operation (expected: 20ns)
```

**Solution:**
```bash
# Verify Release build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Check compiler optimizations
# Ensure -O3 or equivalent is enabled

# Profile to find bottlenecks
perf record -g ./build/bench/Benchmark_OrderBook
perf report
```

#### Issue: Memory leaks
```
Memory leak detected: 1024 bytes
```

**Solution:**
```bash
# Run with leak detector
valgrind --leak-check=full --show-leak-kinds=all ./build/core/OrderBookTests

# Check memory pool reset logic
# Verify arena allocator is reset correctly
```

### Binding Issues

#### Go: cgo errors
```
Error: #include "lob/OrderBook.h" file not found
```

**Solution:**
```bash
# Ensure C++ library is built first
cmake --build build --config Release

# Check CGO_CFLAGS include paths
export CGO_CFLAGS="-I/path/to/core/include"
export CGO_LDFLAGS="-L/path/to/build/lib"

# Verify header file locations
find . -name "OrderBook.h"
```

#### Rust: linking errors
```
Error: linking with `cc` failed: exit code: 1
```

**Solution:**
```bash
# Ensure C++ library is built with proper symbols
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Check library locations in build.rs
# Verify C++ library is built as shared library if needed
```

#### Python: import errors
```
ImportError: No module named 'lob'
```

**Solution:**
```bash
# Install package in development mode
cd bindings/python
pip install -e .

# Verify installation
python -c "import lob; print(lob.__version__)"

# Check shared library is in expected location
```

## Configuration Issues

### Environment Variables

#### Issue: Environment variables not loaded
```
Error: Configuration value not found
```

**Solution:**
```bash
# Verify .env file exists
ls -la .env

# Load environment variables
# Linux/macOS: export $(cat .env | xargs)
# Windows: Use PowerShell or system settings

# Verify variable is set
echo $MEMORY_POOL_SIZE  # Linux/macOS
echo %MEMORY_POOL_SIZE% # Windows
```

#### Issue: Invalid configuration values
```
Error: Invalid value for MEMORY_POOL_SIZE
```

**Solution:**
- Check value type (integer vs string)
- Verify value is within acceptable range
- Check for typos in variable names

### Risk Management

#### Issue: Orders rejected by risk manager
```
Error: Order rejected: Position limit exceeded
```

**Solution:**
```cpp
// Check current position
auto position = manager.get_position("AAPL");

// Adjust risk limits if needed
limits.max_position_per_symbol = larger_value;

// Or reduce order size
order.quantity = smaller_value;
```

## Testing Issues

### Test Failures

#### Issue: Unit tests fail
```
[  FAILED  ] OrderBookTest.AddOrder
```

**Solution:**
```bash
# Run with verbose output
ctest --test-dir build --config Release --verbose

# Run specific test
./build/core/OrderBookTests --gtest_filter=OrderBookTest.AddOrder

# Check test expectations are correct
# Verify test data is valid
```

#### Issue: Integration tests fail
```
Error: Connection refused in integration test
```

**Solution:**
- Ensure required services are running
- Check network configuration
- Verify test environment setup

#### Issue: Fuzz tests crash
```
Error: Fuzz test found crash
```

**Solution:**
```bash
# Run with address sanitizer
cmake -B build-fuzz -S . -DENABLE_ASAN=ON
cmake --build build-fuzz

# Run specific fuzz test
./build-fuzz/tests/FuzzOrderBook

# Report with reproduction steps
```

## Performance Profiling Issues

### Profiling Tools

#### Issue: perf not available (Linux)
```
Error: perf: command not found
```

**Solution:**
```bash
# Install perf
sudo apt-get install linux-tools-common linux-tools-generic

# Or install kernel headers
sudo apt-get install linux-headers-$(uname -r)
```

#### Issue: VTune not available (Windows)
```
Error: VTune Profiler not found
```

**Solution:**
- Download Intel VTune Profiler
- Install and configure PATH
- Use alternative tools (Visual Studio Profiler)

## Memory Issues

### Out of Memory

#### Issue: Process consumes too much memory
```
Error: Out of memory: Killed
```

**Solution:**
```bash
# Reduce memory pool size
export MEMORY_POOL_SIZE=500000

# Enable memory monitoring
export ENABLE_METRICS=ON

# Check for memory leaks
valgrind --leak-check=full ./build/core/OrderBookTests
```

### Memory Fragmentation

#### Issue: Performance degrades over time
```
Performance: 20ns -> 100ns after 1M orders
```

**Solution:**
- Verify memory pool is being reused
- Check arena allocator reset logic
- Monitor memory usage patterns

## Concurrency Issues

### Race Conditions

#### Issue: Non-deterministic test failures
```
Error: Test fails intermittently
```

**Solution:**
```bash
# Run with ThreadSanitizer
cmake -B build-tsan -S . -DENABLE_TSAN=ON
cmake --build build-tsan

# Run tests
./build-tsan/core/OrderBookTests

# Check for data races in output
```

## Platform-Specific Issues

### Windows

#### Issue: Path length limitations
```
Error: Path too long for Windows
```

**Solution:**
- Use shorter directory names
- Enable long path support in Windows
- Move project closer to drive root

#### Issue: DLL loading errors
```
Error: The specified module could not be found
```

**Solution:**
- Add DLL directory to PATH
- Use LoadLibrary with full path
- Check dependency walker for missing DLLs

### Linux

#### Issue: Permission denied
```
Error: Permission denied when accessing /dev/perf_event
```

**Solution:**
```bash
# Adjust perf event permissions
echo 0 | sudo tee /proc/sys/kernel/perf_event_paranoid

# Or run with sudo
sudo ./build/bench/Benchmark_OrderBook
```

### macOS

#### Issue: Code signing errors
```
Error: Code signature invalid
```

**Solution:**
```bash
# Disable gatekeeper temporarily
sudo spctl --master-disable

# Or sign the binary
codesign -s - ./build/bench/Benchmark_OrderBook
```

## Getting Help

### Information to Collect

When reporting issues, collect:

1. **System Information**
   ```bash
   uname -a
   cmake --version
   gcc --version  # or cl / clang
   ```

2. **Build Configuration**
   ```bash
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   cat build/CMakeCache.txt | grep -E "CMAKE_BUILD_TYPE|CMAKE_CXX_COMPILER"
   ```

3. **Error Messages**
   - Full error output
   - Stack traces if available
   - Log files

4. **Reproduction Steps**
   - Minimal code example
   - Configuration used
   - Commands executed

### Debugging Tips

1. **Enable Debug Symbols**
   ```bash
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
   ```

2. **Enable Verbose Logging**
   ```bash
   export SPDLOG_LEVEL=trace
   export ENABLE_VERBOSE_LOGGING=ON
   ```

3. **Use Debuggers**
   - GDB (Linux): `gdb ./build/core/OrderBookTests`
   - LLDB (macOS): `lldb ./build/core/OrderBookTests`
   - Visual Studio Debugger (Windows)

4. **Check Logs**
   ```bash
   # Application logs
   tail -f logs/orderbook.log

   # System logs
   journalctl -xe  # Linux
   log show --predicate 'process == "OrderBook"'  # macOS
   ```

### Common Solutions Summary

| Issue | Common Solution |
|-------|-----------------|
| Build fails | Update CMake/compiler, check dependencies |
| Runtime crash | Run with sanitizers, check memory |
| Slow performance | Use Release build, enable optimizations |
| Binding errors | Build C++ library first, check paths |
| Test failures | Run verbose, check test data |
| Memory issues | Reduce pool size, check for leaks |
| Platform issues | Check platform-specific requirements |