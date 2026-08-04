# C++23 Migration Guide

This guide helps you migrate from C++20 to C++23 in the Low-Latency Order Book Engine.

## Overview

The project has been upgraded from C++20 to C++23 to leverage modern language features and improvements. This guide covers the changes, new features, and migration steps.

## Compiler Requirements

### Minimum Compiler Versions

| Compiler | Minimum Version | C++23 Support |
|----------|----------------|---------------|
| GCC      | 13.0+          | Full          |
| Clang    | 16.0+          | Full          |
| MSVC     | Visual Studio 2022 17.6+ | Partial |
| Apple Clang | 15.0+      | Partial       |

### Platform-Specific Notes

#### Linux
- GCC 13+ recommended for full C++23 support
- Available in Ubuntu 23.04+ and recent distributions
- Install: `sudo apt-get install g++-13`

#### macOS
- Apple Clang 15+ (Xcode 15.0+) provides partial C++23 support
- Some features may not be available
- Consider using Homebrew's Clang for full support

#### Windows
- Visual Studio 2022 17.6+ provides partial C++23 support
- MSVC may not support all C++23 features yet
- Consider using Clang-cl for full support

## New C++23 Features Used

### 1. std::expected for Error Handling

**Before (C++20):**
```cpp
#include <optional>
#include <string>

std::optional<std::string> parse_order(const std::string& input) {
    if (input.empty()) {
        return std::nullopt;
    }
    return input;
}

auto result = parse_order(data);
if (result) {
    // Success
} else {
    // Handle error
}
```

**After (C++23):**
```cpp
#include <expected>

std::expected<std::string, std::string> parse_order(const std::string& input) {
    if (input.empty()) {
        return std::unexpected("Input is empty");
    }
    return input;
}

auto result = parse_order(data);
if (result) {
    // Success
    std::cout << *result << std::endl;
} else {
    // Handle error with message
    std::cout << result.error() << std::endl;
}
```

### 2. std::print for Output

**Before (C++20):**
```cpp
#include <iostream>
#include <format>

std::cout << std::format("Order ID: {}, Price: {:.2f}\n", order_id, price);
```

**After (C++23):**
```cpp
#include <print>

std::print("Order ID: {}, Price: {:.2f}\n", order_id, price);
```

### 3. Deducing this for Member Functions

**Before (C++20):**
```cpp
struct OrderBook {
    auto& get_best_bid() & {
        return best_bid_;
    }
    
    const auto& get_best_bid() const& {
        return best_bid_;
    }
    
    auto&& get_best_bid() && {
        return std::move(best_bid_);
    }
};
```

**After (C++23):**
```cpp
struct OrderBook {
    auto this auto& get_best_bid() {
        return best_bid_;
    }
};
```

### 4. String View Improvements

**Before (C++20):**
```cpp
#include <string_view>

bool has_special_chars(std::string_view sv) {
    return sv.find('@') != std::string_view::npos;
}
```

**After (C++23):**
```cpp
#include <string_view>

bool has_special_chars(std::string_view sv) {
    return sv.contains('@');
}
```

## Migration Steps

### Step 1: Update Compiler

#### Linux
```bash
# Install GCC 13
sudo apt-get update
sudo apt-get install g++-13

# Set as default
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
```

#### macOS
```bash
# Update Xcode to latest version
# Or install Clang via Homebrew
brew install llvm
```

#### Windows
```bash
# Update Visual Studio 2022 to latest version
# Or install Clang via Visual Studio Installer
```

### Step 2: Update CMake Configuration

The CMakeLists.txt has been updated to use C++23:

```cmake
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

### Step 3: Rebuild Project

```bash
# Clean build
rm -rf build

# Rebuild with Conan
conan install . --output-folder=build --build=missing
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Step 4: Update Code (If Needed)

If you have custom code that uses C++20 patterns, update them to C++23 equivalents:

- Replace error handling with `std::expected`
- Replace `std::cout << std::format` with `std::print`
- Simplify member functions with deducing this
- Use new string view methods

### Step 5: Test

```bash
# Run tests
ctest --test-dir build --config Release

# Run benchmarks
./build/bench/Benchmark_OrderBook
```

## Feature Detection

The project includes C++23 feature detection in `cmake/CheckCXX23.cmake`. This checks for:

- `std::expected`
- `std::print`
- Deducing this
- `std::generator`
- `std::mdspan`
- String view improvements

Features are detected at configure time and appropriate compile definitions are set.

## Graceful Degradation

If your compiler doesn't support full C++23, the project will:

1. Detect available features at configure time
2. Set appropriate compile definitions
3. Use C++20 fallbacks where needed
4. Log warnings about missing features

## Backward Compatibility

### For Users

- If your compiler doesn't support C++23, the build will fail
- Update your compiler to minimum requirements
- Use C++20 build option if needed (not recommended)

### For Developers

- Code is written to be C++23 compatible
- Graceful degradation for incomplete C++23 support
- Feature detection ensures compatibility

## Performance Impact

C++23 features generally have minimal performance impact:

- `std::expected`: Similar to current error handling
- `std::print`: Potentially faster than iostream
- Deducing this: No runtime cost
- String view improvements: No runtime cost

## Troubleshooting

### Compiler Too Old

**Error:** `C++23 is not supported`

**Solution:** Update your compiler to minimum requirements

### Feature Not Available

**Error:** `'std::expected' has not been declared`

**Solution:** 
- Update to latest compiler version
- Check feature detection output in CMake configure
- Use C++20 fallback if needed

### Build Fails on macOS

**Error:** C++23 features not available on Apple Clang

**Solution:**
- Update Xcode to latest version
- Consider using Homebrew's Clang
- Use feature detection to disable unsupported features

## References

- [C++23 Reference](https://en.cppreference.com/w/cpp/23)
- [C++23 Features](https://en.cppreference.com/w/cpp/23/history)
- [Compiler Support](https://en.cppreference.com/w/cpp/compiler_support/23)

## Support

If you encounter issues during migration:

1. Check compiler version meets requirements
2. Review CMake configure output for feature detection
3. Consult this guide for specific issues
4. Report problems with compiler version and error messages