# Integration Tests

This directory contains integration tests for the Low-Latency Order Book Engine that verify end-to-end functionality across components.

## Test Categories

### Cross-Language Integration
- **C++ to Go**: Verify C++ core works correctly with Go bindings
- **C++ to Rust**: Verify C++ core works correctly with Rust bindings
- **C++ to Python**: Verify C++ core works correctly with Python bindings
- **Multi-Language**: Verify consistent behavior across all languages

### Component Integration
- **Order Book + Risk Manager**: Verify risk checks during order processing
- **Order Book + Metrics**: Verify metrics collection during operations
- **Order Book Manager + Risk**: Verify multi-symbol risk management
- **CLI + Core**: Verify CLI applications work with core library

### End-to-End Workflows
- **Order Lifecycle**: Complete order from submission to fill/cancellation
- **Market Making**: Continuous market making scenario
- **High-Frequency Trading**: Rapid order submission and cancellation
- **Multi-Symbol Trading**: Trading across multiple symbols simultaneously

## Running Integration Tests

### All Integration Tests
```bash
# C++ integration tests
./build/tests/IntegrationTests

# Go integration tests
cd bindings/go
go test -tags=integration ./...

# Rust integration tests
cd bindings/rust
cargo test --test integration

# Python integration tests
cd bindings/python
pytest tests/integration/
```

### Specific Test Categories
```bash
# Cross-language tests
./build/tests/IntegrationTests --gtest_filter=IntegrationTest.CrossLanguage

# Component integration tests
./build/tests/IntegrationTests --gtest_filter=IntegrationTest.ComponentIntegration

# End-to-end workflow tests
./build/tests/IntegrationTests --gtest_filter=IntegrationTest.EndToEnd
```

## Test Structure

### C++ Integration Tests
```cpp
// tests/integration/test_cross_language.cpp
TEST(IntegrationTest, CrossLanguage) {
    // Test C++ core with Go bindings
    lob::OrderBook book;
    // Add order via C++ API
    auto err = book.add_limit_order(1, 100.0, 10, lob::Side::BUY);
    // Verify via Go bindings
    // ...
}
```

### Go Integration Tests
```go
// bindings/go/integration_test.go
func TestCrossLanguage(t *testing.T) {
    // Test Go bindings with C++ core
    book := lob.NewOrderBook()
    err := book.AddLimitOrder(1, 100.0, 10, lob.SideBuy)
    // Verify with C++ API
    // ...
}
```

### Rust Integration Tests
```rust
// bindings/rust/tests/integration.rs
#[test]
fn test_cross_language() {
    // Test Rust bindings with C++ core
    let mut book = OrderBook::new();
    let result = book.add_limit_order(1, 100.0, 10, Side::Buy);
    // Verify with C++ API
    // ...
}
```

### Python Integration Tests
```python
# bindings/python/tests/integration/test_cross_language.py
def test_cross_language():
    # Test Python bindings with C++ core
    book = lob.OrderBook()
    err = book.add_limit_order(1, 100.0, 10, lob.Side.BUY)
    # Verify with C++ API
    # ...
```

## Test Scenarios

### Order Lifecycle Scenario
1. Create order book
2. Add buy order
3. Add sell order
4. Verify match occurs
5. Cancel remaining order
6. Verify order book state

### Market Making Scenario
1. Create order book
2. Continuously add bid/ask orders
3. Verify continuous matching
4. Monitor performance metrics
5. Verify no memory leaks

### High-Frequency Trading Scenario
1. Create order book
2. Rapidly add and cancel orders
3. Verify throughput meets targets
4. Monitor latency distribution
5. Verify system stability

### Multi-Symbol Scenario
1. Create order book manager
2. Register multiple symbols
3. Add orders to different symbols
4. Verify cross-symbol risk checks
5. Verify metrics per symbol

## Performance Requirements

### Throughput
- **Order Processing**: >10M orders/second
- **Matching**: >5M matches/second
- **Multi-Symbol**: >1M orders/second per symbol

### Latency
- **Order Addition**: <50 ns (p99)
- **Order Cancellation**: <25 ns (p99)
- **Matching**: <75 ns (p99)

### Resource Usage
- **Memory**: <200 MB total
- **CPU**: <50% single core at max throughput
- **No Memory Leaks**: Zero leaks over extended runs

## Test Data

### Order Generation
- Random price and quantity
- Realistic price distributions
- Market-making order patterns
- High-frequency trading patterns

### Market Data
- Historical market data files
- Real-time market data feeds
- Synthetic market data generators

## CI/CD Integration

### GitHub Actions
```yaml
# .github/workflows/integration.yml
name: Integration Tests
on: [push, pull_request]
jobs:
  integration:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build
        run: |
          cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
          cmake --build build --config Release
      - name: Integration Tests
        run: ./build/tests/IntegrationTests
      - name: Language Integration Tests
        run: |
          cd bindings/go && go test -tags=integration ./...
          cd bindings/rust && cargo test --test integration
          cd bindings/python && pytest tests/integration/
```

## Debugging Integration Tests

### Verbose Output
```bash
# C++ tests
./build/tests/IntegrationTests --gtest_filter=* --gtest_print_time=1

# Go tests
go test -tags=integration -v ./...

# Rust tests
cargo test --test integration -- --nocapture

# Python tests
pytest tests/integration/ -v
```

### Debug Build
```bash
# Build with debug symbols
cmake -B build-debug -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --config Debug

# Run with debugger
gdb ./build-debug/tests/IntegrationTests
```

### Logging
```bash
# Enable verbose logging
export SPDLOG_LEVEL=trace
export ENABLE_VERBOSE_LOGGING=ON

# Run tests
./build/tests/IntegrationTests
```

## Writing New Integration Tests

### Template
```cpp
// Add to tests/integration/
TEST(IntegrationTest, NewFeature) {
    // Arrange
    lob::OrderBook book;
    
    // Act
    auto err = book.add_limit_order(1, 100.0, 10, lob::Side::BUY);
    
    // Assert
    ASSERT_EQ(err, lob::ErrorCode::OK);
    EXPECT_EQ(book.get_best_bid(), 100.0);
}
```

### Best Practices
1. **Independent**: Each test should be independent
2. **Isolated**: Tests should not affect each other
3. **Fast**: Integration tests should run in <1 minute
4. **Clear**: Test names should describe what is being tested
5. **Comprehensive**: Cover happy path and error cases

## Troubleshooting

### Test Flakiness
- Add retry logic for transient failures
- Increase timeouts for slow operations
- Ensure proper cleanup between tests

### Environment Issues
- Ensure all dependencies are installed
- Verify environment variables are set
- Check network connectivity for network tests

### Platform-Specific Issues
- Use platform guards for platform-specific tests
- Provide alternative implementations
- Document platform limitations