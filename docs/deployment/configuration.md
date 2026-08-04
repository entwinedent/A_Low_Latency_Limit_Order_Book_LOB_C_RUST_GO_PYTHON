# Configuration Guide

## Overview

The order book engine can be configured through compile-time constants, runtime parameters, and configuration files.

## Compile-Time Configuration

### Memory Pool Capacity

Configure in `core/include/lob/OrderBook.h`:

```cpp
static constexpr size_t POOL_CAPACITY = 1000000; // 1 million orders
```

**Guidelines**:
- Development: 10,000 - 100,000
- Testing: 100,000 - 1,000,000
- Production: 1,000,000 - 10,000,000

### CMake Options

```bash
# Build with tests
cmake -DBUILD_TESTS=ON ..

# Build with benchmarks
cmake -DBUILD_BENCHMARKS=ON ..

# Build with sanitizers
cmake -DENABLE_ASAN=ON -DENABLE_UBSAN=ON ..

# Build with thread sanitizer
cmake -DENABLE_TSAN=ON ..
```

## Runtime Configuration

### Risk Limits

Configure via `RiskLimits` structure:

```cpp
#include "lob/RiskManager.h"

lob::RiskLimits limits;
limits.max_position_per_symbol = 1000000;      // Max net position
limits.max_order_size = 100000;                 // Max single order
limits.max_daily_volume = 10000000;             // Max daily volume
limits.max_price_deviation = 0.10;              // 10% deviation
limits.max_orders_per_second = 10000;           // Rate limit
limits.enable_self_trade_prevention = true;     // Prevent self-trades
limits.enable_circuit_breaker = true;            // Enable circuit breaker
limits.circuit_breaker_threshold = 0.20;        // 20% threshold

lob::RiskManager risk_mgr(limits);
```

### Symbol Configuration

Configure symbol-specific settings:

```cpp
#include "lob/OrderBookManager.h"

lob::SymbolInfo info;
info.symbol = "AAPL";
info.description = "Apple Inc.";
info.tick_size = 0.01;        // Minimum price increment
info.lot_size = 100;          # Minimum order size
info.is_active = true;

lob::OrderBookManager manager;
manager.register_symbol("AAPL", info);
```

### Logging Configuration

Configure spdlog:

```cpp
#include <spdlog/spdlog.h>

// Set log level
spdlog::set_level(spdlog::level::debug);

// Set log pattern
spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%l] %v");

// Log to file
auto file_logger = spdlog::basic_logger_mt("file_logger", "lob_engine.log");
spdlog::set_default_logger(file_logger);
```

## Environment Variables

### Linux/macOS

```bash
# Set library path
export LD_LIBRARY_PATH=/path/to/build/core/Release:$LD_LIBRARY_PATH

# Set log level
export LOB_LOG_LEVEL=debug

# Set memory pool capacity
export LOB_POOL_CAPACITY=1000000
```

### Windows

```powershell
# Set library path
$env:PATH = "C:\path\to\build\core\Release;$env:PATH"

# Set log level
$env:LOB_LOG_LEVEL = "debug"

# Set memory pool capacity
$env:LOB_POOL_CAPACITY = "1000000"
```

## Configuration File

### JSON Configuration

Create `config.json`:

```json
{
  "memory_pool": {
    "capacity": 1000000
  },
  "risk_limits": {
    "max_position_per_symbol": 1000000,
    "max_order_size": 100000,
    "max_daily_volume": 10000000,
    "max_price_deviation": 0.10,
    "max_orders_per_second": 10000,
    "enable_self_trade_prevention": true,
    "enable_circuit_breaker": true,
    "circuit_breaker_threshold": 0.20
  },
  "symbols": [
    {
      "symbol": "AAPL",
      "description": "Apple Inc.",
      "tick_size": 0.01,
      "lot_size": 100,
      "is_active": true
    },
    {
      "symbol": "GOOGL",
      "description": "Alphabet Inc.",
      "tick_size": 0.01,
      "lot_size": 100,
      "is_active": true
    }
  ],
  "logging": {
    "level": "info",
    "file": "lob_engine.log",
    "pattern": "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
  }
}
```

### Loading Configuration

```cpp
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void load_config(const std::string& config_file) {
    std::ifstream f(config_file);
    json config = json::parse(f);
    
    // Load memory pool configuration
    size_t pool_capacity = config["memory_pool"]["capacity"];
    
    // Load risk limits
    lob::RiskLimits limits;
    limits.max_position_per_symbol = config["risk_limits"]["max_position_per_symbol"];
    limits.max_order_size = config["risk_limits"]["max_order_size"];
    limits.max_daily_volume = config["risk_limits"]["max_daily_volume"];
    limits.max_price_deviation = config["risk_limits"]["max_price_deviation"];
    limits.max_orders_per_second = config["risk_limits"]["max_orders_per_second"];
    limits.enable_self_trade_prevention = config["risk_limits"]["enable_self_trade_prevention"];
    limits.enable_circuit_breaker = config["risk_limits"]["enable_circuit_breaker"];
    limits.circuit_breaker_threshold = config["risk_limits"]["circuit_breaker_threshold"];
    
    // Load symbols
    lob::OrderBookManager manager;
    for (const auto& symbol_config : config["symbols"]) {
        lob::SymbolInfo info;
        info.symbol = symbol_config["symbol"];
        info.description = symbol_config["description"];
        info.tick_size = symbol_config["tick_size"];
        info.lot_size = symbol_config["lot_size"];
        info.is_active = symbol_config["is_active"];
        manager.register_symbol(info.symbol, info);
    }
    
    // Configure logging
    std::string log_level = config["logging"]["level"];
    std::string log_file = config["logging"]["file"];
    std::string log_pattern = config["logging"]["pattern"];
    
    spdlog::level::level_enum level;
    if (log_level == "debug") level = spdlog::level::debug;
    else if (log_level == "info") level = spdlog::level::info;
    else if (log_level == "警告") level = spdlog::level::warn;
    else if (log_level == "error") level = spdlog::level::error;
    
    spdlog::set_level(level);
    spdlog::set_pattern(log_pattern);
    auto file_logger = spdlog::basic_logger_mt("file_logger", log_file);
    spdlog::set_default_logger(file_logger);
}
```

## Performance Tuning

### CPU Affinity

Pin the matching engine to specific cores:

```cpp
#include <pthread.h>

void set_cpu_affinity(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}
```

### NUMA Awareness

For multi-socket systems:

```cpp
#include <numa.h>

void set_numa_node(int node_id) {
    numa_set_preferred(node_id);
}
```

### Disable Turbo Boost

```bash
# Linux
echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo

# Windows (BIOS setting)
```

## Metrics Configuration

Enable metrics collection:

```cpp
#include "lob/Metrics.h"

// Access global metrics
auto& metrics = lob::SystemMetrics::instance();

// Get order book metrics for a symbol
auto& order_book_metrics = metrics.get_order_book_metrics("AAPL");

// Print metrics
std::cout << metrics.to_string() << std::endl;
```

## Advanced Order Types

Configure advanced order types:

```cpp
#include "lob/OrderTypes.h"

// Stop-loss order
lob::ExtendedOrder stop_loss(1, 95.0, 100, lob::Side::SELL, lob::OrderType::STOP_LOSS);
stop_loss.stop_price = 95.0;

// Take-profit order
lob::ExtendedOrder take_profit(2, 105.0, 100, lob::Side::SELL, lob::OrderType::TAKE_PROFIT);
take_profit.take_profit_price = 105.0;

// Iceberg order
lob::ExtendedOrder iceberg(3, 100.0, 1000, lob::Side::BUY, lob::OrderType::ICEBERG);
// display_quantity and hidden_quantity are auto-calculated

// Trailing stop
lob::ExtendedOrder trailing_stop(4, 100.0, 100, lob::Side::SELL, lob::OrderType::TRAILING_STOP);
trailing_stop.trail_amount = 5.0;
trailing_stop.activation_price = 105.0;
```

## Monitoring Configuration

### Prometheus Metrics

Export metrics in Prometheus format:

```cpp
std::string to_prometheus_format(const lob::OrderBookMetrics& metrics, const std::string& symbol) {
    std::ostringstream oss;
    oss << "# HELP lob_orders_received Total orders received\n";
    oss << "# TYPE lob_orders_received counter\n";
    oss << "lob_orders_received{symbol=\"" << symbol << "\"} " << metrics.orders_received.get() << "\n";
    
    oss << "# HELP lob_orders_accepted Total orders accepted\n";
    oss << "# TYPE lob_orders_accepted counter\n";
    oss << "lob_orders_accepted{symbol=\"" << symbol << "\"} " << metrics.orders_accepted.get() << "\n";
    
    oss << "# HELP lob_trades_executed Total trades executed\n";
    oss << "# TYPE lob_trades_executed counter\n";
    oss << "lob_trades_executed{symbol=\"" << symbol << "\"} " << metrics.trades_executed.get() << "\n";
    
    oss << "# HELP lob_add_order_latency_p50 P50 add order latency in nanoseconds\n";
    oss << "# TYPE lob_add_order_latency_p50 gauge\n";
    oss << "lob_add_order_latency_p50{symbol=\"" << symbol << "\"} " << metrics.add_order_latency.percentile(50) << "\n";
    
    oss << "# HELP lob_add_order_latency_p95 P95 add order latency in nanoseconds\n";
    oss << "# TYPE lob_add_order_latency_p95 gauge\n";
    oss << "lob_add_order_latency_p95{symbol=\"" << symbol << "\"} " << metrics.add_order_latency.percentile(95) << "\n";
    
    oss << "# HELP lob_add_order_latency_p99 P99 add order latency in nanoseconds\n";
    oss << "# TYPE lob_add_order_latency_p99 gauge\n";
    oss << "lob_add_order_latency_p99{symbol=\"" << symbol << "\"} " << metrics.add_order_latency.percentile(99) << "\n";
    
    return oss.str();
}
```

## Security Configuration

### Input Validation

Enable strict input validation:

```cpp
bool validate_order(const lob::ExtendedOrder& order) {
    // Validate price
    if (order.price <= 0 || order.price > MAX_PRICE) {
        return false;
    }
    
    // Validate quantity
    if (order.quantity <= 0 || order.quantity > MAX_QUANTITY) {
        return false;
    }
    
    // Validate order ID
    if (order.id == 0) {
        return false;
    }
    
    return true;
}
```

### Rate Limiting

Configure rate limiting:

```cpp
class RateLimiter {
    std::atomic<uint64_t> count_;
    uint64_t max_per_second_;
    std::chrono::steady_clock::time_point last_reset_;
    
public:
    RateLimiter(uint64_t max_per_second) 
        : count_(0), max_per_second_(max_per_second), 
          last_reset_(std::chrono::steady_clock::now()) {}
    
    bool allow() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_reset_);
        
        if (elapsed.count() >= 1) {
            count_.store(0);
            last_reset_ = now;
        }
        
        if (count_.load() >= max_per_second_) {
            return false;
        }
        
        count_.fetch_add(1);
        return true;
    }
};
```

## Best Practices

1. **Start with conservative limits**: Begin with strict risk limits and relax as needed
2. **Monitor metrics continuously**: Track latency, throughput, and error rates
3. **Test in staging**: Validate configuration changes before production deployment
4. **Document changes**: Keep a changelog of configuration modifications
5. **Backup configurations**: Version control your configuration files
6. **Use environment-specific configs**: Separate configs for dev, test, and production
