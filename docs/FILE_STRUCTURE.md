# File Structure and Directory Layout

This document provides a comprehensive overview of the project's file structure and the purpose of each file and directory.

## Root Directory

### Configuration Files

- **`.clang-format`** - Clang-format configuration for consistent C++ code formatting
- **`.clang-tidy`** - Clang-tidy configuration for static analysis and code quality checks
- **`.coveragerc`** - Python coverage configuration for test coverage reporting
- **`.dockerignore`** - Docker build ignore patterns to exclude unnecessary files from Docker images
- **`.editorconfig`** - Editor configuration for consistent coding styles across different editors
- **`.env`** - Environment variables for local development (not committed to git)
- **`.env.example`** - Example environment variables template
- **`.gitattributes`** - Git attributes for file handling and line ending configurations
- **`.github/`** - GitHub-specific configurations
  - **`workflows/`** - CI/CD pipeline definitions
    - **`ci.yml`** - Main CI/CD pipeline for automated testing and building
    - **`README.md`** - Documentation for GitHub workflows
- **`.gitignore`** - Git ignore patterns to exclude build artifacts and temporary files
- **`.pre-commit-config.yaml`** - Pre-commit hooks configuration for code quality checks
- **`CMakeLists.txt`** - Main CMake build configuration file for the C++ project
- **`conanfile.txt`** - Conan dependency specification file for C++ package management
- **`conanfile.py`** - Custom Conan recipe for building and packaging the project
- **`docker-compose.yml`** - Docker Compose configuration for multi-container setups
- **`Dockerfile`** - Docker image configuration for containerized builds
- **`Makefile`** - Makefile for common build and development tasks
- **`vcpkg.json`** - vcpkg dependency specification file (alternative to Conan)
- **`requirements.txt`** - Python dependencies for development and testing

### Documentation Files

- **`ARCHITECTURE.md`** - System architecture documentation, data structures, and algorithms
- **`CHANGELOG.md`** - Version history and changelog for releases
- **`CODE_OF_CONDUCT.md`** - Community code of conduct guidelines
- **`CONTRIBUTING.md`** - Guidelines for contributing to the project
- **`NO_LICENSE`** - Placeholder for license information
- **`README.md`** - Main project README with overview, features, and getting started guide
- **`SECURITY.md`** - Security policy and vulnerability reporting guidelines
- **`SETUP.md`** - Detailed setup and installation instructions
- **`VERSION`** - Current version number of the project

### Source Code Directories

- **`apps/`** - Command-line applications and tools
  - **`CMakeLists.txt`** - CMake configuration for building applications
  - **`README.md`** - Documentation for CLI applications
  - **`main.cpp`** - Main CLI application entry point

- **`artifacts/`** - Build artifacts and test outputs
  - **`go-coverage.out`** - Go test coverage output file
  - **`go-coverage.txt`** - Go test coverage report in text format
  - **`rust-coverage.lcov`** - Rust test coverage in LCOV format
  - **`rust-coverage.txt`** - Rust test coverage report in text format

- **`bench/`** - Benchmarking suite for performance testing
  - **`Benchmark_OrderBook.cpp`** - Google Benchmark suite for order book operations
  - **`CMakeLists.txt`** - CMake configuration for building benchmarks
  - **`Profile_Latency.cpp`** - Latency profiling utilities
  - **`README.md`** - Documentation for running benchmarks

- **`bindings/`** - Language bindings for Go, Rust, and Python
  - **`README.md`** - Overview of language bindings

- **`cmake/`** - CMake modules and helper scripts
  - **`CheckCXX23.cmake`** - C++23 feature detection module
  - **`PreventInSourceBuilds.cmake`** - Prevents in-source builds
  - **`Sanitizers.cmake`** - Compiler sanitizer configuration (ASAN, UBSAN, TSAN)
  - **`StandardProjectSettings.cmake`** - Standard CMake project settings
  - **`README.md`** - Documentation for CMake modules

- **`core/`** - Core C++ implementation
  - **`include/lob/`** - Header files for core components
    - **`ArenaAllocator.h`** - Arena allocator for zero-cost temporary allocations
    - **`IntrusiveList.h`** - Intrusive doubly-linked list implementation
    - **`MemoryPool.h`** - Pre-allocated memory pool for order management
    - **`OrderBook.h`** - Main order book class and matching engine
    - **`OrderBookManager.h`** - Multi-symbol order book management
    - **`OrderHashMap.h`** - Hash map for O(1) order lookup
    - **`OrderTypes.h`** - Order type definitions and structures
    - **`PriceLevelMap.h`** - Sorted maps for bid/ask price levels
    - **`RiskManager.h`** - Risk management and position limits
    - **`Metrics.h`** - Performance metrics and monitoring
    - **`spdlog/`** - Bundled spdlog logging library (header-only)
    - **`fmt/`** - Bundled fmt formatting library (header-only)
  - **`src/`** - Implementation files
    - **`CAPI.cpp`** - C API wrapper for FFI bindings
    - **`OrderBook.cpp`** - Order book implementation
    - **`README.md`** - Documentation for core implementation

- **`docs/`** - Comprehensive documentation
  - **`README.md`** - Documentation index and overview
  - **`api/`** - API documentation
    - **`.gitkeep`** - Placeholder for git tracking
    - **`README.md`** - API reference overview
  - **`architecture/`** - Architecture documentation
    - **`concurrency.md`** - Concurrency and threading model
    - **`data-structures.md`** - Core data structures documentation
    - **`memory-management.md`** - Memory management strategies
    - **`README.md`** - Architecture documentation index
  - **`deployment/`** - Deployment guides
    - **`configuration.md`** - Configuration options and settings
    - **`installation.md`** - Installation procedures
    - **`monitoring.md`** - Monitoring and observability
    - **`README.md`** - Deployment documentation index
  - **`examples/`** - Code examples and tutorials
    - **`.gitkeep`** - Placeholder for git tracking
    - **`README.md`** - Examples documentation
  - **`guides/`** - User guides and tutorials
    - **`cpp23_migration.md`** - C++23 migration guide
    - **`deployment.md`** - Deployment strategies
    - **`migration.md`** - Version migration guide
    - **`README.md`** - User guides index
    - **`troubleshooting.md`** - Common issues and solutions
  - **`performance/`** - Performance documentation
    - **`benchmarking.md`** - Benchmarking procedures
    - **`optimization.md`** - Performance optimization techniques
    - **`profiling.md`** - Profiling tools and techniques
    - **`README.md`** - Performance documentation index

- **`scripts/`** - Utility scripts and automation
  - **`update_dependencies.py`** - Dependency update script for all components
  - **`artifacts/`** - Artifact management scripts
  - **`debug/`** - Debugging helper scripts
  - **`launch/`** - Application launch scripts
  - **`profiling/`** - Profiling helper scripts
    - **`README.md`** - Profiling scripts documentation
  - **`testing/`** - Testing automation scripts
    - **`cpp_coverage_debug.txt`** - C++ coverage debug output
    - **`run_cpp_coverage.ps1`** - PowerShell script for C++ coverage
    - **`run_go_coverage.ps1`** - PowerShell script for Go coverage
    - **`run_rust_coverage.ps1`** - PowerShell script for Rust coverage

- **`tests/`** - Test suites
  - **`CMakeLists.txt`** - CMake configuration for building tests
  - **`README.md`** - Testing documentation
  - **`e2e/`** - End-to-end tests
    - **`README.md`** - E2E test documentation
    - **`Test_CompleteWorkflow.cpp`** - Complete workflow integration test
  - **`fuzz/`** - Fuzz testing
    - **`README.md`** - Fuzz testing documentation
    - **`Fuzz_OrderBook.cpp`** - Fuzz test for order book operations
  - **`integration/`** - Integration tests
    - **`README.md`** - Integration test documentation
    - **`Test_MatchingLogic.cpp`** - Matching logic integration test
    - **`Test_MultiSymbol.cpp`** - Multi-symbol integration test
  - **`load/`** - Load testing
    - **`README.md`** - Load testing documentation
  - **`unit/`** - Unit tests
    - **`README.md`** - Unit test documentation
    - **`Test_ArenaAllocator.cpp`** - Arena allocator unit tests
    - **`Test_CAPI.cpp`** - C API unit tests
    - **`Test_IntrusiveList.cpp`** - Intrusive list unit tests
    - **`Test_MemoryPool.cpp`** - Memory pool unit tests
    - **`Test_Metrics.cpp`** - Metrics unit tests
    - **`Test_OrderBook.cpp`** - Order book unit tests
    - **`Test_OrderBookManager.cpp`** - Order book manager unit tests
    - **`Test_OrderTypes.cpp`** - Order types unit tests
    - **`Test_RiskManager.cpp`** - Risk manager unit tests

## Language Bindings

### Go Bindings (`bindings/go/`)

- **`Makefile`** - Makefile with race detector support and build targets
- **`README.md`** - Go bindings documentation with race detector instructions
- **`.golangci.yml`** - golangci-lint configuration for Go code quality
- **`go.mod`** - Go module definition (Go 1.22+)
- **`go.sum`** - Go dependency checksums
- **`engine.go`** - Go wrapper for the C++ order book engine
- **`engine_test.go`** - Go test suite with race detector support
- **`cl-wrapper.bat`** - Windows batch wrapper for MSVC compiler
- **`cl-wrapper.cmd`** - Windows command wrapper for MSVC compiler
- **`clang-cl-wrapper.cmd`** - Windows wrapper for clang-cl compiler
- **`clang-cl-wrapper.py`** - Python wrapper for clang-cl compiler
- **`coverage`** - Coverage output directory
- **`coverage-go.txt`** - Go coverage report
- **`coverage.out`** - Go coverage data file
- **`$out`** - Temporary build output file

### Python Bindings (`bindings/python/`)

- **`pyproject.toml`** - Python project configuration (Python 3.12+)
- **`README.md`** - Python bindings documentation
- **`lob/`** - Python package
  - **`__init__.py`** - Main Python module with ctypes FFI bindings
  - **`__init__.pyi`** - Type stub file for IDE support and type checking
  - **`py.typed`** - Marker file indicating the package has type hints
- **`mypy.ini`** - mypy configuration for strict type checking
- **`tests/`** - Python test suite
  - **`test_lob.py`** - Python unit tests for order book

### Rust Bindings (`bindings/rust/`)

- **`Cargo.toml`** - Rust project configuration
- **`Cargo.lock`** - Rust dependency lock file
- **`README.md`** - Rust bindings documentation
- **`src/`** - Rust source code
  - **`lib.rs`** - Rust library entry point
  - **`bindings.rs`** - cxx bridge bindings
  - **`order_book.rs`** - Rust wrapper for order book

## External Dependencies

### Bundled Libraries

The project includes bundled versions of certain libraries to ensure reproducible builds:

- **`core/include/lob/spdlog/`** - Bundled spdlog v1.12.0 (header-only logging library)
- **`core/include/lob/fmt/`** - Bundled fmt v10.1.1 (header-only formatting library)

These are included to avoid version conflicts and ensure consistent behavior across platforms.

## Build Artifacts

The following directories are created during the build process and are not committed to version control:

- **`build/`** - Main CMake build directory
- **`build-coverage/`** - Coverage instrumentation build directory
- **`build-*/`** - Other build directories for different configurations

## Temporary Files

The following files are temporary and should not be committed:

- **`*.log`** - Log files from various tools
- **`*.txt`** - Temporary output files (coverage, benchmark results)
- **`*.out`** - Binary output files
- **`coverage/`** - Coverage report directories
- **`htmlcov/`** - HTML coverage reports
- **`.pytest_cache/`** - Python pytest cache
- **`.hypothesis/`** - Hypothesis testing cache
- **`.venv/`** - Python virtual environment

## Summary

The project follows a modular structure with:
- **Core C++ implementation** in `core/` with header-only libraries
- **Language bindings** in `bindings/` for Go, Rust, and Python
- **Comprehensive testing** in `tests/` with unit, integration, E2E, and fuzz tests
- **Documentation** in `docs/` with architecture, API, and user guides
- **Build automation** via CMake, Conan/vcpkg, and language-specific build systems
- **CI/CD** via GitHub Actions workflows
- **Quality tools** including clang-format, clang-tidy, golangci-lint, and mypy

This structure ensures maintainability, reproducibility, and ease of development across multiple platforms and languages.
