#pragma once

#include "Common.h"
#include <atomic>
#include <cstdint>
#include <chrono>
#include <array>
#include <string>
#include <sstream>

namespace lob {

// Thread-safe metrics counter
class MetricsCounter {
public:
    MetricsCounter() : value_(0) {}
    
    void increment(int64_t delta = 1) {
        value_.fetch_add(delta, std::memory_order_relaxed);
    }
    
    void set(int64_t value) {
        value_.store(value, std::memory_order_relaxed);
    }
    
    int64_t get() const {
        return value_.load(std::memory_order_relaxed);
    }
    
    void reset() {
        value_.store(0, std::memory_order_relaxed);
    }
    
private:
    std::atomic<int64_t> value_;
};

// Latency histogram for percentile tracking
class LatencyHistogram {
public:
    static constexpr size_t NUM_BUCKETS = 20;
    static constexpr uint64_t MAX_LATENCY_NS = 1000000; // 1ms
    
    LatencyHistogram() {
        for (auto& count : buckets_) {
            count.store(0, std::memory_order_relaxed);
        }
    }
    
    void record(uint64_t latency_ns) {
        if (latency_ns >= MAX_LATENCY_NS) {
            buckets_[NUM_BUCKETS - 1].fetch_add(1, std::memory_order_relaxed);
        } else {
            size_t bucket = (latency_ns * NUM_BUCKETS) / MAX_LATENCY_NS;
            buckets_[bucket].fetch_add(1, std::memory_order_relaxed);
        }
    }
    
    uint64_t get_count(size_t bucket) const {
        return buckets_[bucket].load(std::memory_order_relaxed);
    }
    
    uint64_t get_total_count() const {
        uint64_t total = 0;
        for (const auto& count : buckets_) {
            total += count.load(std::memory_order_relaxed);
        }
        return total;
    }
    
    void reset() {
        for (auto& count : buckets_) {
            count.store(0, std::memory_order_relaxed);
        }
    }
    
    // Estimate percentile
    uint64_t percentile(double p) const {
        uint64_t total = get_total_count();
        if (total == 0) return 0;
        
        uint64_t target = static_cast<uint64_t>(total * p / 100.0);
        uint64_t cumulative = 0;
        
        for (size_t i = 0; i < NUM_BUCKETS; ++i) {
            cumulative += buckets_[i].load(std::memory_order_relaxed);
            if (cumulative >= target) {
                return (i * MAX_LATENCY_NS) / NUM_BUCKETS;
            }
        }
        
        return MAX_LATENCY_NS;
    }
    
private:
    std::array<std::atomic<uint64_t>, NUM_BUCKETS> buckets_;
};

// Comprehensive metrics for order book
struct OrderBookMetrics {
    MetricsCounter orders_received;
    MetricsCounter orders_accepted;
    MetricsCounter orders_rejected;
    MetricsCounter orders_cancelled;
    MetricsCounter orders_filled;
    MetricsCounter orders_partial_fill;
    
    MetricsCounter trades_executed;
    MetricsCounter trade_volume;
    
    LatencyHistogram add_order_latency;
    LatencyHistogram cancel_order_latency;
    LatencyHistogram match_latency;
    
    std::atomic<uint64_t> last_update_time_ns;
    
    OrderBookMetrics() : last_update_time_ns(0) {}
    
    void record_add_order(uint64_t latency_ns, bool accepted) {
        orders_received.increment();
        if (accepted) {
            orders_accepted.increment();
        } else {
            orders_rejected.increment();
        }
        add_order_latency.record(latency_ns);
        update_timestamp();
    }
    
    void record_cancel_order(uint64_t latency_ns) {
        orders_cancelled.increment();
        cancel_order_latency.record(latency_ns);
        update_timestamp();
    }
    
    void record_trade(uint64_t latency_ns, Quantity quantity) {
        trades_executed.increment();
        trade_volume.increment(quantity);
        match_latency.record(latency_ns);
        update_timestamp();
    }
    
    void reset() {
        orders_received.reset();
        orders_accepted.reset();
        orders_rejected.reset();
        orders_cancelled.reset();
        orders_filled.reset();
        orders_partial_fill.reset();
        trades_executed.reset();
        trade_volume.reset();
        add_order_latency.reset();
        cancel_order_latency.reset();
        match_latency.reset();
    }
    
    std::string to_string() const {
        std::ostringstream oss;
        oss << "OrderBookMetrics:\n";
        oss << "  Orders Received: " << orders_received.get() << "\n";
        oss << "  Orders Accepted: " << orders_accepted.get() << "\n";
        oss << "  Orders Rejected: " << orders_rejected.get() << "\n";
        oss << "  Orders Cancelled: " << orders_cancelled.get() << "\n";
        oss << "  Trades Executed: " << trades_executed.get() << "\n";
        oss << "  Trade Volume: " << trade_volume.get() << "\n";
        oss << "  P50 Add Latency: " << add_order_latency.percentile(50) << " ns\n";
        oss << "  P95 Add Latency: " << add_order_latency.percentile(95) << " ns\n";
        oss << "  P99 Add Latency: " << add_order_latency.percentile(99) << " ns\n";
        oss << "  P50 Match Latency: " << match_latency.percentile(50) << " ns\n";
        oss << "  P95 Match Latency: " << match_latency.percentile(95) << " ns\n";
        oss << "  P99 Match Latency: " << match_latency.percentile(99) << " ns\n";
        return oss.str();
    }
    
private:
    void update_timestamp() {
        auto now = std::chrono::high_resolution_clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
        last_update_time_ns.store(ns, std::memory_order_relaxed);
    }
};

// System-wide metrics
class SystemMetrics {
public:
    static SystemMetrics& instance() {
        static SystemMetrics metrics;
        return metrics;
    }
    
    OrderBookMetrics& get_order_book_metrics(const std::string& symbol) {
        // For simplicity, return global metrics
        // In production, would have per-symbol metrics
        return order_book_metrics_;
    }
    
    void reset() {
        order_book_metrics_.reset();
    }
    
    std::string to_string() const {
        return order_book_metrics_.to_string();
    }
    
private:
    SystemMetrics() = default;
    OrderBookMetrics order_book_metrics_;
};

// RAII timer for automatic latency recording
class ScopedLatencyTimer {
public:
    ScopedLatencyTimer(LatencyHistogram& histogram)
        : histogram_(histogram)
        , start_(std::chrono::high_resolution_clock::now())
    {}
    
    ~ScopedLatencyTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto latency_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count();
        histogram_.record(latency_ns);
    }
    
private:
    LatencyHistogram& histogram_;
    std::chrono::high_resolution_clock::time_point start_;
};

} // namespace lob
