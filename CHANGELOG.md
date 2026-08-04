# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial project structure and foundation files
- Version management system
- Security policy and vulnerability reporting
- Comprehensive documentation structure

## [1.0.0] - 2026-08-03

### Added
- High-performance C++20 core with zero-allocation memory pools
- Price-time priority matching engine
- O(1) order cancellation via hash map lookup
- Cache-optimized data structures with alignas(64)
- Advanced order types (stop-loss, take-profit, iceberg, trailing stop, stop-limit, FOK, AON)
- Risk management system (position limits, order size limits, circuit breakers, self-trade prevention)
- Multi-symbol order book management
- Built-in metrics collection and monitoring
- Arena allocator for zero-cost temporary allocations
- Language bindings for Go, Rust, and Python
- Comprehensive testing infrastructure (unit tests, integration tests, fuzz testing)
- Google Benchmark suite with sub-microsecond latency targets
- Docker support for containerized builds
- CI/CD workflows with GitHub Actions
- Pre-commit hooks for code quality
- Coverage instrumentation for C++, Go, Rust, and Python

### Performance
- Target latency: Add Order ~20 ns, Cancel Order ~10 ns, Match Order ~25 ns
- Zero-allocation design for hot path operations
- Cache line alignment for optimal memory access

### Documentation
- Comprehensive README with usage examples
- Setup guide for Windows, Linux, and macOS
- API documentation for C++, Go, Rust, and Python
- Architecture documentation
- Performance benchmarking guide

### Security
- Memory pool based design prevents buffer overflows
- Input validation for all order data
- Sanitizer support (ASAN, UBSAN, TSAN)
- Security policy and vulnerability reporting

## [0.1.0] - Development Phase

### Added
- Initial proof of concept
- Basic order book implementation
- Matching engine prototype
- Language binding experiments

[Unreleased]: https://github.com/example/lob-engine/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/example/lob-engine/releases/tag/v1.0.0
[0.1.0]: https://github.com/example/lob-engine/releases/tag/v0.1.0