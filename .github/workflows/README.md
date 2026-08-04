# GitHub Actions Workflows

This folder contains CI/CD workflows for the Low Latency Limit Order Book (LOB) project.

## Available Workflows

### ci.yml - Main CI/CD Pipeline

The main workflow (`ci.yml`) provides comprehensive testing and validation across multiple platforms and configurations.

#### Jobs Overview

**1. build-and-test**
- **Purpose**: Build and test the core C++ library across platforms
- **Matrix**:
  - OS: Ubuntu, Windows, macOS
  - Build Type: Debug, Release
  - Compilers: GCC (Ubuntu), MSVC (Windows), Clang (macOS)
- **Steps**:
  - Checkout code
  - Set up CMake (3.20.x)
  - Configure and build with CMake
  - Run unit tests with CTest
  - Run sanitizers (ASAN, UBSAN) on Linux Debug builds
  - Execute benchmarks (Release only)
  - Upload benchmark results as artifacts

**2. bindings-test**
- **Purpose**: Test polyglot language bindings
- **Matrix**:
  - OS: Ubuntu, Windows
  - Binding: Go, Rust, Python
- **Steps**:
  - Build C++ library
  - Set up language-specific toolchains
  - Run binding-specific tests
  - Additional checks (race detector for Go, mypy for Python)

**3. code-quality**
- **Purpose**: Enforce code quality standards
- **OS**: Ubuntu Latest
- **Steps**:
  - Configure with clang-tidy support
  - Run clang-format formatting checks
  - Run clang-tidy static analysis
  - Build with strict warnings (`-Werror`)

**4. docker-build**
- **Purpose**: Validate Docker build and container-based testing
- **OS**: Ubuntu Latest
- **Steps**:
  - Set up Docker Buildx
  - Build Docker image (`lob-engine:latest`)
  - Run tests inside Docker container

**5. documentation-build**
- **Purpose**: Generate project documentation
- **OS**: Ubuntu Latest
- **Steps**:
  - Set up Python 3.12
  - Install Sphinx and documentation dependencies
  - Build documentation (when configured)
  - Upload documentation artifacts

## C++23 Feature Testing

The workflows support testing of advanced C++23 features:

- **Conditional Compilation**: Features are enabled based on compiler support
- **Docker Environment**: Uses GCC 16.0.1 for comprehensive C++23 testing
- **Fallback Libraries**: Kokkos mdspan for std::mdspan when native support is incomplete

### Supported C++23 Features

- `std::expected` - Value-based error handling
- `std::print` and `<format>` - High-performance output
- Deducing this - Member traversal elegance
- `std::mdspan` - Multi-dimensional array views (with Kokkos fallback)

## Workflow Triggers

Workflows are triggered on:
- Push to `main` or `develop` branches
- Pull requests targeting `main` or `develop` branches

## Artifacts

The workflows generate the following artifacts:
- **Benchmark Results**: JSON format benchmark data per OS and build type
- **Documentation**: Generated Sphinx documentation (when configured)

## Dependencies

- **CMake**: 3.20.x or higher
- **Compilers**: 
  - GCC (Ubuntu) - supports C++23 features
  - MSVC (Windows) - supports C++23 features
  - Clang (macOS) - supports C++23 features
- **Language Toolchains**:
  - Go 1.22
  - Rust stable
  - Python 3.12

## Customization

To customize the workflows:

1. **Add new test configurations**: Modify the matrix in `build-and-test` job
2. **Enable additional sanitizers**: Add to the sanitizer step in `build-and-test`
3. **Add new language bindings**: Extend the `bindings-test` job matrix
4. **Configure documentation**: Update the `documentation-build` job with Sphinx commands
5. **Adjust CMake version**: Update the `cmake-version` parameter in setup-cmake actions

## Troubleshooting

**Common Issues:**

1. **CMake Version Errors**: Ensure CMake 3.20.x is available
2. **Compiler Support**: Some C++23 features require specific compiler versions
3. **Docker Build Failures**: Check Dockerfile and ensure dependencies are available
4. **Binding Test Failures**: Verify language toolchains are properly configured

## Future Enhancements

Potential improvements to consider:
- Add code coverage reporting
- Implement performance regression detection
- Add security scanning (SAST, dependency scanning)
- Add deployment steps for releases
- Add notification integration (Slack, email)
