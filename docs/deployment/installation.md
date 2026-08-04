# Installation Guide

## Prerequisites

### System Requirements

- **OS**: Windows 10/11, Linux (Ubuntu 20.04+), macOS 11+
- **CPU**: x86_64 with AVX2 support recommended
- **RAM**: 8GB minimum, 16GB+ recommended for production
- **Storage**: 100MB for installation

### Software Requirements

- **CMake**: 3.20 or higher
- **C++ Compiler**: 
  - Windows: MSVC 2019+ (Visual Studio 2019/2022)
  - Linux: GCC 10+ or Clang 12+
  - macOS: Clang 12+ (Xcode 13+)
- **Python**: 3.8+ (for Python bindings)
- **Go**: 1.21+ (for Go bindings)
- **Rust**: 1.70+ (for Rust bindings)

## Installation

### Windows

#### 1. Install Build Tools

```powershell
# Install CMake
winget install Kitware.CMake

# Install Visual Studio Community 2022
winget install Microsoft.VisualStudio.2022.Community
# During installation, select "Desktop development with C++" workload

# Install Go
winget install GoLang.Go

# Install Rust
winget install Rustlang.Rust.MSVC

# Install Python
winget install Python.Python.3.14
```

#### 2. Clone Repository

```powershell
git clone https://github.com/example/lob-engine.git
cd lob-engine
```

#### 3. Build C++ Core

```powershell
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

#### 4. Build Language Bindings

```powershell
# Python bindings
cd bindings/python
pip install -e .

# Rust bindings
cd ../rust
cargo build --release

# Go bindings (may have issues on Windows)
cd ../go
go mod download
go build -o engine.so ./engine.go
```

### Linux

#### 1. Install Build Tools

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y cmake build-essential g++ clang ninja-build
sudo apt install -y python3 python3-pip python3-venv
sudo apt install -y golang rustc cargo

# Fedora
sudo dnf install cmake gcc-c++ clang ninja-build
sudo dnf install python3 python3-pip golang rust cargo
```

#### 2. Clone Repository

```bash
git clone https://github.com/example/lob-engine.git
cd lob-engine
```

#### 3. Build C++ Core

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -GNinja
ninja -C build
```

#### 4. Build Language Bindings

```bash
# Python bindings
cd bindings/python
pip install -e .

# Rust bindings
cd ../rust
cargo build --release

# Go bindings
cd ../go
go mod download
go build -o engine.so ./engine.go
```

### macOS

#### 1. Install Build Tools

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install tools
brew install cmake ninja python go rust
```

#### 2. Clone Repository

```bash
git clone https://github.com/example/lob-engine.git
cd lob-engine
```

#### 3. Build C++ Core

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -GNinja
ninja -C build
```

#### 4. Build Language Bindings

```bash
# Python bindings
cd bindings/python
pip install -e .

# Rust bindings
cd ../rust
cargo build --release

# Go bindings
cd ../go
go mod download
go build -o engine.so ./engine.go
```

## Verification

### Test C++ Core

```bash
# Run unit tests
ctest --test-dir build --config Release

# Run benchmarks
./build/bench/benchmark_order_book  # Linux/macOS
.\build\bench\benchmark_order_book.exe  # Windows
```

### Test Python Bindings

```bash
cd bindings/python
python -c "import lob; print('Python bindings loaded successfully')"
pytest tests/ -v
```

### Test Rust Bindings

```bash
cd bindings/rust
cargo test
```

### Test Go Bindings

```bash
cd bindings/go
go test ./...
```

## Configuration

### Memory Pool Configuration

Edit `core/include/lob/OrderBook.h`:

```cpp
static constexpr size_t POOL_CAPACITY = 1000000; // Adjust based on needs
```

### Risk Limits Configuration

Edit `core/include/lob/RiskManager.h` or configure at runtime:

```cpp
RiskLimits limits;
limits.max_position_per_symbol = 1000000;
limits.max_order_size = 100000;
limits.max_daily_volume = 10000000;
```

### Multi-Symbol Configuration

```cpp
#include "lob/OrderBookManager.h"

lob::OrderBookManager manager;
manager.register_symbol("AAPL", {"AAPL", "Apple Inc.", 0.01, 100});
manager.register_symbol("GOOGL", {"GOOGL", "Alphabet Inc.", 0.01, 100});
```

## Production Deployment

### System Tuning

#### CPU Affinity

```bash
# Linux - Isolate cores
echo 0-3 > /sys/devices/system/cpu/isolated
# Pin matching engine to isolated cores
taskset -c 0 ./build/apps/lob_cli
```

#### Disable CPU Frequency Scaling

```bash
# Linux
sudo cpupower frequency-set -g performance

# Windows
powercfg /setactive scheme_min
```

#### Disable Swap

```bash
# Linux
sudo swapoff -a
```

### Monitoring

Enable metrics collection:

```cpp
#include "lob/Metrics.h"

auto& metrics = lob::SystemMetrics::instance();
// Metrics are automatically collected
std::cout << metrics.to_string() << std::endl;
```

### Logging

Configure spdlog in your application:

```cpp
#include <spdlog/spdlog.h>

spdlog::set_level(spdlog::level::info);
spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
```

## Troubleshooting

### Build Errors

**CMake not found**:
- Ensure CMake is in PATH
- Restart terminal after installation

**Compiler not found**:
- Windows: Install Visual Studio with C++ workload
- Linux: Install build-essential or equivalent
- macOS: Install Xcode Command Line Tools

**Linker errors**:
- Ensure all dependencies are built
- Check library paths in CMake

### Runtime Errors

**Library not found**:
- Ensure shared library is in LD_LIBRARY_PATH (Linux) or PATH (Windows)
- Use `ldd` (Linux) or `dumpbin /dependents` (Windows) to check dependencies

**Segmentation fault**:
- Run with sanitizers: `cmake -DENABLE_ASAN=ON`
- Check for null pointer dereferences
- Verify memory pool capacity

### Performance Issues

**High latency**:
- Check CPU frequency scaling is disabled
- Verify CPU affinity is set correctly
- Profile with perf (Linux) or VTune (Windows)

**Memory exhaustion**:
- Increase POOL_CAPACITY
- Monitor memory usage with metrics
- Check for memory leaks with Valgrind

## Uninstallation

### Windows

```powershell
# Remove build directory
Remove-Item -Recurse -Force build

# Uninstall Python package
pip uninstall lob-engine

# Remove Rust artifacts
cd bindings/rust
cargo clean
```

### Linux/macOS

```bash
# Remove build directory
rm -rf build

# Uninstall Python package
pip uninstall lob-engine

# Remove Rust artifacts
cd bindings/rust
cargo clean
```

## Upgrading

```bash
# Pull latest changes
git pull origin main

# Rebuild
cmake --build build --config Release

# Reinstall bindings
cd bindings/python && pip install -e .
cd ../rust && cargo build --release
```
