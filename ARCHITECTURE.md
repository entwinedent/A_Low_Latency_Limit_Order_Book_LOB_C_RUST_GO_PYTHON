# Architecture

## System Overview

The Low-Latency Order Book Engine is a high-performance trading system designed for sub-microsecond latency requirements. The architecture follows a modular design with a native C++ core and language bindings for Go, Rust, and Python.

## Core Components

### 1. Memory Management

#### Memory Pool
- **Purpose**: Eliminate runtime heap allocations for order management
- **Implementation**: Pre-allocated contiguous memory block with placement new
- **Benefits**: Zero-allocation hot path, deterministic memory usage, cache-friendly
- **Location**: `core/include/lob/MemoryPool.h`

#### Arena Allocator
- **Purpose**: Zero-cost temporary allocations during matching operations
- **Implementation**: Bump allocator with automatic reset
- **Benefits**: No fragmentation, constant-time allocation/deallocation
- **Location**: `core/include/lob/ArenaAllocator.h`

### 2. Data Structures

#### Intrusive List
- **Purpose**: O(1) insertion/removal of orders in price levels
- **Implementation**: Doubly-linked list with embedded nodes
- **Benefits**: No pointer indirection, cache-optimized traversal
- **Location**: `core/include/lob/IntrusiveList.h`

#### Price Level Map
- **Purpose**: Maintain sorted bid/ask price levels
- **Implementation**: Red-black tree for ordered access
- **Benefits**: O(log n) insertion/deletion, O(1) best bid/ask access
- **Location**: `core/include/lob/PriceLevelMap.h`

#### Order Hash Map
- **Purpose**: O(1) order lookup for cancellations
- **Implementation**: Open addressing with linear probing
- **Benefits**: Constant-time cancellation, cache-friendly
- **Location**: `core/include/lob/OrderHashMap.h`

### 3. Matching Engine

#### Order Book
- **Purpose**: Central order management and matching
- **Implementation**: Price-time priority with FIFO matching
- **Algorithms**:
  - Buy orders match against lowest sell price (best ask)
  - Sell orders match against highest buy price (best bid)
  - Orders at same price level matched FIFO
- **Location**: `core/include/lob/OrderBook.h`

#### Order Types
- **Basic**: Limit orders, market orders
- **Advanced**: Stop-loss, take-profit, iceberg, trailing stop, stop-limit, FOK, AON
- **Location**: `core/include/lob/OrderTypes.h`

### 4. Risk Management

#### Risk Manager
- **Purpose**: Pre-trade risk checks and position management
- **Features**:
  - Position limits per symbol
  - Order size limits
  - Daily volume limits
  - Circuit breakers
  - Self-trade prevention
- **Location**: `core/include/lob/RiskManager.h`

### 5. Multi-Symbol Support

#### Order Book Manager
- **Purpose**: Unified interface for multiple order books
- **Implementation**: Hash map of order books keyed by symbol
- **Benefits**: Single point of control, consistent risk management
- **Location**: `core/include/lob/OrderBookManager.h`

### 6. Monitoring & Metrics

#### System Metrics
- **Purpose**: Performance monitoring and profiling
- **Features**:
  - Latency histograms
  - Performance counters
  - Prometheus export
  - RAII latency timers
- **Location**: `core/include/lob/Metrics.h`

## Language Bindings

### Go Bindings
- **Implementation**: cgo-based FFI wrapper
- **Location**: `bindings/go/`
- **Build System**: Go modules
- **Concurrency**: Goroutine-safe with mutex protection

### Rust Bindings
- **Implementation**: cxx bridge for safe C++ interop
- **Location**: `bindings/rust/`
- **Build System**: Cargo
- **Safety**: Zero-cost abstractions, memory-safe wrapper

### Python Bindings
- **Implementation**: pybind11 for Python/C++ integration
- **Location**: `bindings/python/`
- **Build System**: setuptools/pyproject.toml
- **Features**: NumPy integration, type hints

## Performance Characteristics

### Latency Targets
| Operation      | Target Latency | Complexity |
|----------------|----------------|------------|
| Add Order      | ~20 ns         | O(1) amortized |
| Cancel Order   | ~10 ns         | O(1) hash lookup |
| Match Order    | ~25 ns         | O(log n) price lookup |
| Query Best Bid | ~5 ns          | O(1) direct access |
| Query Best Ask | ~5 ns          | O(1) direct access |

### Memory Layout
- **Cache Line Alignment**: alignas(64) for critical structures
- **Memory Pool**: Pre-allocated, no fragmentation
- **Arena Allocator**: Bump allocation, zero-cost
- **Data Locality**: Hot data kept in CPU cache

### Thread Safety
- **Current**: Single-threaded design for maximum performance
- **Future**: Lock-free data structures for multi-threading
- **Strategy**: Per-thread order books with message passing

## Build System

### CMake Configuration
- **Minimum Version**: CMake 3.20+
- **C++ Standard**: C++23
- **C++23 Features Used**:
  - `std::expected` for error handling
  - `std::print` for logging output
  - Deducing this for member functions
  - String view improvements (contains method)
  - Feature detection via `cmake/CheckCXX23.cmake`
- **Build Types**: Debug, Release, RelWithDebInfo
- **Sanitizers**: ASAN, UBSAN, TSAN support
- **Coverage**: Compiler instrumentation for coverage collection
- **Package Managers**: Conan (recommended), vcpkg (alternative), FetchContent (fallback)

### Dependencies
- **GoogleTest**: Unit testing framework
- **Google Benchmark**: Performance benchmarking
- **spdlog**: Header-only logging library
- **fmt**: Header-only formatting library

## Testing Strategy

### Unit Tests
- **Coverage**: Core algorithms and data structures
- **Framework**: GoogleTest
- **Location**: `tests/`

### Integration Tests
- **Coverage**: End-to-end workflows
- **Framework**: Language-specific test runners
- **Location**: `tests/integration/`

### Fuzz Testing
- **Coverage**: Input validation and edge cases
- **Framework**: AFL++ or libFuzzer
- **Target**: Order book operations

### Performance Tests
- **Coverage**: Latency regression detection
- **Framework**: Google Benchmark
- **Location**: `bench/`

## Deployment Architecture

### Standalone Deployment
- **Process**: Single binary with embedded order book
- **Use Case**: High-frequency trading desks
- **Advantages**: Maximum performance, minimal latency

### Service Deployment
- **Process**: Network-accessible service
- **Use Case**: Multi-client trading platforms
- **Advantages**: Centralized management, resource sharing

### Container Deployment
- **Process**: Docker container with all dependencies
- **Use Case**: Cloud deployment and testing
- **Advantages**: Consistency, scalability

## Security Architecture

### Memory Safety
- **Design**: Memory pools prevent buffer overflows
- **Validation**: Extensive input validation
- **Testing**: Sanitizer coverage

### Risk Controls
- **Pre-trade**: Position limits, order size limits
- **Real-time**: Circuit breakers, self-trade prevention
- **Post-trade**: Position monitoring, anomaly detection

### FFI Safety
- **Go**: cgo with careful memory management
- **Rust**: cxx bridge with compile-time safety
- **Python**: pybind11 with automatic reference counting

## Future Enhancements

### Short-term
- Lock-free data structures for multi-threading
- Persistent order book state
- WebSocket API for real-time updates

### Medium-term
- Distributed order book support
- Machine learning for market making
- Advanced order types and strategies

### Long-term
- Hardware acceleration (FPGA/GPU)
- Real-time risk analytics
- Cross-exchange arbitrage