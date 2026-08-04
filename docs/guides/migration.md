# Migration Guide

This guide helps you migrate between different versions of the Low-Latency Order Book Engine.

## Version 1.0.0

### What's New
- Initial stable release
- Comprehensive API for C++, Go, Rust, and Python
- Advanced order types (stop-loss, take-profit, iceberg, trailing stop, stop-limit, FOK, AON)
- Risk management system
- Multi-symbol support
- Performance monitoring and metrics
- Comprehensive testing infrastructure

### Migration from Development Versions

If you were using development versions (0.x.x), here are the key changes:

#### API Changes
- **C++**: Namespace changes from `lob_dev` to `lob`
- **Go**: Module path updated to `github.com/example/lob-engine`
- **Rust**: Crate name changed from `lob_engine_dev` to `lob_engine`
- **Python**: Package name changed from `lob_engine_dev` to `lob_engine`

#### Configuration Changes
- Environment variables now use uppercase naming (e.g., `ENABLE_TESTS` instead of `enable_tests`)
- CMake options standardized with `ENABLE_` prefix
- Risk management configuration moved to dedicated config file

#### Breaking Changes
- Memory pool size configuration now uses environment variable `MEMORY_POOL_SIZE`
- Order ID type changed from `uint32_t` to `uint64_t` for larger range
- Trade callback signature updated to include additional metadata

### Migration Steps

#### 1. Update Dependencies
```bash
# C++ (CMake)
# Update CMakeLists.txt to use new version
# No action needed if using the main repository

# Go
cd bindings/go
go mod edit -require=github.com/example/lob-engine@v1.0.0
go mod tidy

# Rust
cd bindings/rust
cargo update

# Python
cd bindings/python
pip install --upgrade lob-engine
```

#### 2. Update Code
```cpp
// C++ - Update namespace
// Old: lob_dev::OrderBook book;
// New: lob::OrderBook book;

// Update order ID type
// Old: uint32_t order_id = 1;
// New: uint64_t order_id = 1;
```

```go
// Go - Update module path
// Old: import "github.com/example/lob-engine-dev"
// New: import "github.com/example/lob-engine"
```

```rust
// Rust - Update crate name
// Old: use lob_engine_dev::OrderBook;
// New: use lob_engine::OrderBook;
```

```python
# Python - Update package name
# Old: import lob_engine_dev
# New: import lob
```

#### 3. Update Configuration
```bash
# Update environment variable names
# Old: enable_tests=true
# New: ENABLE_TESTS=ON

# Update risk management configuration
# Old: in-code configuration
# New: environment variables or config file
```

#### 4. Rebuild and Test
```bash
# Clean build
rm -rf build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run tests
ctest --test-dir build --config Release

# Run language binding tests
cd bindings/go && go test ./...
cd bindings/rust && cargo test
cd bindings/python && python -m pytest tests/
```

### Data Migration

#### Order Book State
If you have persistent order book state from development versions:

1. **Export current state** using development version APIs
2. **Convert data format** to match new schema
3. **Import into new version** using migration scripts

#### Configuration Files
- Old configuration files may need manual updates
- Check environment variable names and formats
- Update risk management configuration

### Testing After Migration

1. **Smoke Tests**: Verify basic order book operations
2. **Integration Tests**: Run full integration test suite
3. **Performance Tests**: Verify performance meets expectations
4. **Regression Tests**: Check for behavior changes

### Rollback Plan

If migration fails:

1. **Keep backup** of development version installation
2. **Revert dependencies** to previous versions
3. **Restore configuration** from backup
4. **Report issues** with detailed error information

## Platform Migration

### Linux to Windows

#### Prerequisites
- Install Visual Studio with C++ workload
- Install CMake for Windows
- Install Go, Rust, Python for Windows

#### Build Changes
```bash
# Use MSBuild instead of make
cmake --build build --config Release

# Windows-specific paths
.\build\bench\Benchmark_OrderBook.exe
```

#### Configuration Changes
- Use Windows environment variables
- Update file paths (backslashes vs forward slashes)
- Adjust service configuration for Windows

### Windows to Linux

#### Prerequisites
- Install GCC or Clang
- Install CMake for Linux
- Install Go, Rust, Python for Linux

#### Build Changes
```bash
# Use make instead of MSBuild
cmake --build build --config Release

# Linux-specific paths
./build/bench/Benchmark_OrderBook
```

#### Configuration Changes
- Use Linux environment variables
- Update file paths (forward slashes)
- Adjust service configuration for Linux (systemd)

### Container Migration

#### Prerequisites
- Install Docker
- Prepare Dockerfile

#### Migration Steps
```bash
# Build container
docker build -t lob-engine .

# Test container
docker run --rm lob-engine

# Deploy container
docker run -d --name lob-engine lob-engine
```

#### Volume Mapping
```bash
# Map configuration and data directories
docker run -d \
  -v /path/to/config:/workspace/config \
  -v /path/to/data:/workspace/data \
  lob-engine
```

## Compatibility Matrix

| Version | C++  | Go  | Rust | Python | Status      |
|---------|------|-----|------|--------|-------------|
| 1.0.0   | 20   | 1.21| 1.70 | 3.8+   | Stable      |
| 0.x.x   | 20   | 1.21| 1.70 | 3.8+   | Deprecated  |

## Common Migration Issues

### Build Errors
- **Issue**: CMake configuration fails
- **Solution**: Ensure CMake 3.20+ is installed
- **Check**: `cmake --version`

### Runtime Errors
- **Issue**: Symbol not found errors
- **Solution**: Rebuild all components after dependency update
- **Check**: Verify all bindings are rebuilt

### Performance Regression
- **Issue**: Slower performance after migration
- **Solution**: Check compiler optimization flags
- **Check**: Verify build type is Release

### Data Incompatibility
- **Issue**: Old data format not recognized
- **Solution**: Use migration scripts to convert data
- **Check**: Verify data format matches new schema

## Getting Help

If you encounter issues during migration:

1. Check this guide for known issues
2. Review GitHub issues for similar problems
3. Check test suite for example usage
4. Report new issues with detailed information

Include in bug reports:
- Version migrating from and to
- Platform and environment details
- Error messages and stack traces
- Steps to reproduce the issue