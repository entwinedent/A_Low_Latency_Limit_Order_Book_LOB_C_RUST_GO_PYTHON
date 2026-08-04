# Monitoring Guide

## Overview

The order book engine provides comprehensive monitoring capabilities through metrics, logging, and health checks.

## Metrics

### Built-in Metrics

The engine tracks the following metrics automatically:

- **Order Metrics**: Orders received, accepted, rejected, cancelled, filled
- **Trade Metrics**: Trades executed, trade volume
- **Latency Metrics**: Add order, cancel order, match latency (P50, P95, P99)
- **Memory Metrics**: Pool capacity, allocated count, free count
- **System Metrics**: CPU usage, memory usage

### Accessing Metrics

```cpp
#include "lob/Metrics.h"

// Get global metrics instance
auto& metrics = lob::SystemMetrics::instance();

// Get metrics for a specific symbol
auto& order_book_metrics = metrics.get_order_book_metrics("AAPL");

// Print metrics
std::cout << metrics.to_string() << std::endl;
```

### Metrics Output Example

```
OrderBookMetrics:
  Orders Received: 1000000
  Orders Accepted: 999500
  Orders Rejected: 500
  Orders Cancelled: 100000
  Orders Filled: 895000
  Orders Partial Fill: 4500
  Trades Executed: 895000
  Trade Volume: 89500000
  P50 Add Latency: 25 ns
  P95 Add Latency: 45 ns
  P99 Add Latency: 120 ns
  P50 Match Latency: 30 ns
  P95 Match Latency: 55 ns
  P99 Match Latency: 150 ns
```

### Prometheus Export

Export metrics in Prometheus format for scraping:

```cpp
std::string to_prometheus_format(const lob::OrderBookMetrics& metrics, const std::string& symbol) {
    std::ostringstream oss;
    oss << "# HELP lob_orders_received Total orders received\n";
    oss << "# TYPE lob_orders_received counter\n";
    oss << "lob_orders_received{symbol=\"" << symbol << "\"} " << metrics.orders_received.get() << "\n";
    
    oss << "# HELP lob_add_order_latency_p50 P50 add order latency in nanoseconds\n";
    oss << "# TYPE lob_add_order_latency_p50 gauge\n";
    oss << "lob_add_order_latency_p50{symbol=\"" << symbol << "\"} " << metrics.add_order_latency.percentile(50) << "\n";
    
    return oss.str();
}
```

### StatsD Export

Export metrics to StatsD:

```cpp
#include <boost/asio.hpp>

void export_to_statsd(const std::string& host, int port, const lob::OrderBookMetrics& metrics) {
    boost::asio::io_context io;
    boost::asio::ip::udp::socket socket(io);
    boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::make_address(host), port);
    
    std::string metric = "lob.orders_received:" + std::to_string(metrics.orders_received.get()) + "|c";
    socket.send_to(boost::asio::buffer(metric), endpoint);
}
```

## Logging

### Log Levels

Configure log levels based on environment:

```cpp
// Development
spdlog::set_level(spdlog::level::debug);

// Production
spdlog::set_level(spdlog::level::info);
```

### Log Patterns

Customize log format:

```cpp
// Detailed pattern for debugging
spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%l] [%s:%#] %v");

// Simple pattern for production
spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
```

### Log Rotation

Configure log rotation:

```cpp
#include <spdlog/sinks/rotating_file_sink.h>

auto rotating_logger = spdlog::rotating_logger_mt("rotating_logger", "lob_engine.log", 1024 * 1024 * 5, 3);
spdlog::set_default_logger(rotating_logger);
```

### Structured Logging

Use structured logging for better parsing:

```cpp
spdlog::info("Order added: order_id={}, price={}, quantity={}, side={}", 
             order.id, order.price, order.quantity, side_to_string(order.side));
```

## Health Checks

### Basic Health Check

```cpp
bool health_check() {
    auto& metrics = lob::SystemMetrics::instance();
    
    // Check if order book is responsive
    auto book = metrics.get_order_book_metrics("AAPL");
    if (book.orders_received.get() == 0 && book.orders_accepted.get() == 0) {
        return false; // No activity
    }
    
    // Check memory pool
    if (book.pool_allocated() >= book.pool_capacity()) {
        return false; // Pool exhausted
    }
    
    return true;
}
```

### Advanced Health Check

```cpp
struct HealthStatus {
    bool healthy;
    std::string message;
    std::map<std::string, std::string> details;
};

HealthStatus detailed_health_check() {
    HealthStatus status;
    status.healthy = true;
    
    auto& metrics = lob::SystemMetrics::instance();
    auto& order_book_metrics = metrics.get_order_book_metrics("AAPL");
    
    // Check latency
    uint64_t p99_latency = order_book_metrics.add_order_latency.percentile(99);
    if (p99_latency > 1000) {  // 1 microsecond
        status.healthy = false;
        status.message = "High latency detected";
        status.details["p99_latency_ns"] = std::to_string(p99_latency);
    }
    
    // Check rejection rate
    uint64_t received = order_book_metrics.orders_received.get();
    uint64_t rejected = order_book_metrics.orders_rejected.get();
    double rejection_rate = static_cast<double>(rejected) / received;
    if (rejection_rate > 0.01) {  // 1%
        status.healthy = false;
        status.message = "High rejection rate";
        status.details["rejection_rate"] = std::to_string(rejection_rate);
    }
    
    // Check memory pool
    double pool_usage = static_cast<double>(order_book_metrics.pool_allocated()) / 
                        order_book_metrics.pool_capacity();
    if (pool_usage > 0.9) {  // 90%
        status.healthy = false;
        status.message = "Memory pool nearly exhausted";
        status.details["pool_usage"] = std::to_string(pool_usage);
    }
    
    return status;
}
```

### HTTP Health Endpoint

Expose health check via HTTP:

```cpp
#include <cpprest/http_listener.h>

void start_health_server() {
    web::http::listener::http_listener listener(web::http::uri("http://localhost:8080/health"));
    
    listener.support(web::http::methods::GET, [](web::http::http_request request) {
        HealthStatus status = detailed_health_check();
        
        web::json::value response;
        response["healthy"] = status.healthy;
        response["message"] = web::json::value::string(status.message);
        
        for (const auto& [key, value] : status.details) {
            response[key] = web::json::value::string(value);
        }
        
        request.reply(status.healthy ? web::http::status_codes::OK : 
                                      web::http::status_codes::ServiceUnavailable, response);
    });
    
    listener.open().wait();
}
```

## Alerting

### Alert Conditions

Configure alerts for:

- **High Latency**: P99 latency > 1 microsecond
- **High Rejection Rate**: Rejection rate > 1%
- **Memory Exhaustion**: Pool usage > 90%
- **Circuit Breaker**: Circuit breaker triggered
- **No Activity**: No orders received for 1 minute

### Alert Implementation

```cpp
class AlertManager {
    std::function<void(const std::string&)> alert_callback_;
    
public:
    AlertManager(std::function<void(const std::string&)> callback)
        : alert_callback_(callback) {}
    
    void check_alerts(const lob::OrderBookMetrics& metrics) {
        // Check latency
        uint64_t p99_latency = metrics.add_order_latency.percentile(99);
        if (p99_latency > 1000) {
            alert_callback_("ALERT: High P99 latency: " + std::to_string(p99_latency) + " ns");
        }
        
        // Check rejection rate
        uint64_t received = metrics.orders_received.get();
        uint64_t rejected = metrics.orders_rejected.get();
        if (received > 0) {
            double rejection_rate = static_cast<double>(rejected) / received;
            if (rejection_rate > 0.01) {
                alert_callback_("ALERT: High rejection rate: " + std::to_string(rejection_rate * 100) + "%");
            }
        }
        
        // Check memory pool
        double pool_usage = static_cast<double>(metrics.pool_allocated()) / 
                            metrics.pool_capacity();
        if (pool_usage > 0.9) {
            alert_callback_("ALERT: Memory pool usage: " + std::to_string(pool_usage * 100) + "%");
        }
    }
};
```

### Email Alerts

```cpp
void send_email_alert(const std::string& message) {
    // Use SMTP library to send email
    // Example: libsmtp, Poco Net, etc.
}
```

### Slack Alerts

```cpp
void send_slack_alert(const std::string& message) {
    // Send webhook to Slack
    // Example: use curl or HTTP library
}
```

## Performance Monitoring

### Real-time Latency Tracking

```cpp
class LatencyMonitor {
    std::deque<uint64_t> latencies_;
    size_t max_samples_;
    
public:
    LatencyMonitor(size_t max_samples = 1000) : max_samples_(max_samples) {}
    
    void record(uint64_t latency_ns) {
        latencies_.push_back(latency_ns);
        if (latencies_.size() > max_samples_) {
            latencies_.pop_front();
        }
    }
    
    double get_average() const {
        if (latencies_.empty()) return 0.0;
        uint64_t sum = 0;
        for (auto latency : latencies_) {
            sum += latency;
        }
        return static_cast<double>(sum) / latencies_.size();
    }
    
    uint64_t get_max() const {
        if (latencies_.empty()) return 0;
        return *std::max_element(latencies_.begin(), latencies_.end());
    }
    
    uint64_t get_min() const {
        if (latencies_.empty()) return 0;
        return *std::min_element(latencies_.begin(), latencies_.end());
    }
};
```

### Throughput Monitoring

```cpp
class ThroughputMonitor {
    std::atomic<uint64_t> count_;
    std::chrono::steady_clock::time_point start_time_;
    
public:
    ThroughputMonitor() : count_(0), start_time_(std::chrono::steady_clock::now()) {}
    
    void increment() {
        count_.fetch_add(1, std::memory_order_relaxed);
    }
    
    double get_ops_per_second() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
        if (elapsed.count() == 0) return 0.0;
        return static_cast<double>(count_.load()) / elapsed.count();
    }
    
    void reset() {
        count_.store(0);
        start_time_ = std::chrono::steady_clock::now();
    }
};
```

## Distributed Tracing

### OpenTelemetry Integration

```cpp
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/exporters/otlp/otlp_grpc_exporter.h>

void init_tracing() {
    auto exporter = opentelemetry::exporter::otlp::OtlpGrpcExporterFactory::Create();
    
    auto provider = opentelemetry::sdk::trace::TracerProviderFactory::Create(
        opentelemetry::sdk::trace::SimpleSpanProcessorFactory::Create(std::move(exporter))
    );
    
    opentelemetry::trace::Provider::SetTracerProvider(std::move(provider));
}

void trace_add_order(OrderID id, Price price, Quantity quantity, Side side) {
    auto tracer = opentelemetry::trace::Provider::GetTracerProvider()->GetTracer("lob_engine");
    
    auto span = tracer->StartSpan("add_order");
    span->SetAttribute("order.id", id);
    span->SetAttribute("order.price", price);
    span->SetAttribute("order.quantity", quantity);
    span->SetAttribute("order.side", side == Side::BUY ? "BUY" : "SELL");
    
    // Add order logic here
    
    span->End();
}
```

## Dashboard

### Grafana Dashboard

Create a Grafana dashboard to visualize metrics:

1. **Order Flow Panel**: Orders received vs accepted over time
2. **Latency Panel**: P50, P95, P99 latency over time
3. **Throughput Panel**: Orders per second
4. **Memory Panel**: Pool usage over time
5. **Trade Panel**: Trades executed and volume

### Example Prometheus Queries

```promql
# Orders per second
rate(lob_orders_received[1m])

# P99 latency
lob_add_order_latency_p99

# Rejection rate
rate(lob_orders_rejected[1m]) / rate(lob_orders_received[1m])

# Memory pool usage
lob_pool_allocated / lob_pool_capacity
```

## Best Practices

1. **Monitor continuously**: Don't wait for problems to occur
2. **Set appropriate thresholds**: Tune alert thresholds based on baseline performance
3. **Correlate metrics**: Look at multiple metrics together to diagnose issues
4. **Maintain baselines**: Track normal performance patterns
5. **Test alerts**: Verify alerting works before production deployment
6. **Document incidents**: Keep records of incidents and resolutions
7. **Review regularly**: Periodically review monitoring setup for improvements
