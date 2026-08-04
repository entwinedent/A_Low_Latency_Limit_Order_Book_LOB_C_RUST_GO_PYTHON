# .github

This folder contains GitHub-specific configuration and workflows for the Low Latency Limit Order Book (LOB) project.

## Structure

- **workflows/** - GitHub Actions CI/CD workflows
  - `ci.yml` - Main CI/CD pipeline for building, testing, and quality checks
  - `README.md` - Documentation for workflow configurations

## CI/CD Pipeline Overview

The project uses GitHub Actions for continuous integration and deployment with the following workflows:

### Main CI Pipeline (`ci.yml`)

**Build and Test Matrix:**
- **OS**: Ubuntu, Windows, macOS
- **Build Types**: Debug, Release
- **Compilers**: GCC (Ubuntu), MSVC (Windows), Clang (macOS)

**Testing Coverage:**
- Unit tests with GoogleTest
- AddressSanitizer and UndefinedBehaviorSanitizer (Linux Debug)
- Benchmark execution and result collection
- Language binding tests (Go, Rust, Python)

**Code Quality:**
- clang-format formatting checks
- clang-tidy static analysis
- Strict warning builds with `-Werror`

**Docker Integration:**
- Docker image build verification
- Container-based testing

**Documentation:**
- Sphinx documentation generation (when configured)

## C++23 Feature Support

The project implements advanced C++23 features with conditional compilation:

- **std::expected** - Value-based error handling
- **std::print** and `<format>` - High-performance output
- **Deducing this** - Member traversal elegance (requires GCC 15+ or MSVC 19.52+)
- **std::mdspan** - Multi-dimensional array views (uses Kokkos mdspan fallback when native support is incomplete)

## Docker Environment

The Docker environment (`lob-cpp23` image) provides:
- GCC 16.0.1 for C++23 feature testing
- Pre-cloned Kokkos mdspan library for std::mdspan fallback
- Consistent build environment across platforms

## Language Bindings

The CI pipeline tests polyglot language bindings:
- **Go** - CGO wrapper with race detector
- **Rust** - Zero-cost FFI bindings with cargo test
- **Python** - ctypes wrapper with mypy type checking

## Contributing

When contributing to this project:
1. Ensure all CI checks pass before merging
2. Follow the code style enforced by clang-format
3. Address all clang-tidy warnings
4. Add tests for new features
5. Update documentation as needed
