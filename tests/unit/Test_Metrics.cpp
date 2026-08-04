#include <gtest/gtest.h>
#include "lob/Metrics.h"

using namespace lob;

TEST(MetricsCounterTest, DefaultConstruction) {
    MetricsCounter counter;
    
    EXPECT_EQ(counter.get(), 0);
}

TEST(MetricsCounterTest, Increment) {
    MetricsCounter counter;
    
    counter.increment();
    EXPECT_EQ(counter.get(), 1);
    
    counter.increment(5);
    EXPECT_EQ(counter.get(), 6);
}

TEST(MetricsCounterTest, Set) {
    MetricsCounter counter;
    
    counter.set(42);
    EXPECT_EQ(counter.get(), 42);
}

TEST(MetricsCounterTest, Reset) {
    MetricsCounter counter;
    
    counter.increment(100);
    EXPECT_EQ(counter.get(), 100);
    
    counter.reset();
    EXPECT_EQ(counter.get(), 0);
}

TEST(MetricsCounterTest, NegativeIncrement) {
    MetricsCounter counter;
    
    counter.increment(10);
    counter.increment(-5);
    EXPECT_EQ(counter.get(), 5);
}

TEST(LatencyHistogramTest, DefaultConstruction) {
    LatencyHistogram histogram;
    
    EXPECT_EQ(histogram.get_total_count(), 0);
    EXPECT_EQ(histogram.percentile(50), 0);
    EXPECT_EQ(histogram.percentile(95), 0);
    EXPECT_EQ(histogram.percentile(99), 0);
}

TEST(LatencyHistogramTest, RecordLatency) {
    LatencyHistogram histogram;
    
    histogram.record(100);
    histogram.record(200);
    histogram.record(300);
    
    EXPECT_EQ(histogram.get_total_count(), 3);
}

TEST(LatencyHistogramTest, RecordHighLatency) {
    LatencyHistogram histogram;
    
    histogram.record(2000000); // 2ms, above max
    EXPECT_EQ(histogram.get_total_count(), 1);
}

TEST(LatencyHistogramTest, GetBucketCount) {
    LatencyHistogram histogram;
    
    histogram.record(100);
    
    uint64_t count = histogram.get_count(0);
    EXPECT_GT(count, 0);
}

TEST(LatencyHistogramTest, Reset) {
    LatencyHistogram histogram;
    
    histogram.record(100);
    histogram.record(200);
    
    EXPECT_EQ(histogram.get_total_count(), 2);
    
    histogram.reset();
    EXPECT_EQ(histogram.get_total_count(), 0);
}

TEST(LatencyHistogramTest, Percentile50) {
    LatencyHistogram histogram;
    
    for (int i = 1; i <= 100; ++i) {
        histogram.record(i * 1000); // 1,000 to 100,000 ns
    }
    
    uint64_t p50 = histogram.percentile(50);
    EXPECT_GT(p50, 0);
    EXPECT_LT(p50, 1000000);
}

TEST(LatencyHistogramTest, Percentile95) {
    LatencyHistogram histogram;
    
    for (int i = 0; i < 100; ++i) {
        histogram.record(i * 1000);
    }
    
    uint64_t p95 = histogram.percentile(95);
    EXPECT_GT(p95, 0);
    EXPECT_LT(p95, 1000000);
}

TEST(LatencyHistogramTest, Percentile99) {
    LatencyHistogram histogram;
    
    for (int i = 0; i < 100; ++i) {
        histogram.record(i * 1000);
    }
    
    uint64_t p99 = histogram.percentile(99);
    EXPECT_GT(p99, 0);
    EXPECT_LT(p99, 1000000);
}

TEST(LatencyHistogramTest, EmptyPercentile) {
    LatencyHistogram histogram;
    
    EXPECT_EQ(histogram.percentile(50), 0);
    EXPECT_EQ(histogram.percentile(95), 0);
    EXPECT_EQ(histogram.percentile(99), 0);
}

TEST(OrderBookMetricsTest, DefaultConstruction) {
    OrderBookMetrics metrics;
    
    EXPECT_EQ(metrics.orders_received.get(), 0);
    EXPECT_EQ(metrics.orders_accepted.get(), 0);
    EXPECT_EQ(metrics.orders_rejected.get(), 0);
}

TEST(OrderBookMetricsTest, RecordAddOrderAccepted) {
    OrderBookMetrics metrics;
    
    metrics.record_add_order(100, true);
    
    EXPECT_EQ(metrics.orders_received.get(), 1);
    EXPECT_EQ(metrics.orders_accepted.get(), 1);
    EXPECT_EQ(metrics.orders_rejected.get(), 0);
}

TEST(OrderBookMetricsTest, RecordAddOrderRejected) {
    OrderBookMetrics metrics;
    
    metrics.record_add_order(100, false);
    
    EXPECT_EQ(metrics.orders_received.get(), 1);
    EXPECT_EQ(metrics.orders_accepted.get(), 0);
    EXPECT_EQ(metrics.orders_rejected.get(), 1);
}

TEST(OrderBookMetricsTest, RecordCancelOrder) {
    OrderBookMetrics metrics;
    
    metrics.record_cancel_order(50);
    
    EXPECT_EQ(metrics.orders_cancelled.get(), 1);
}

TEST(OrderBookMetricsTest, RecordTrade) {
    OrderBookMetrics metrics;
    
    metrics.record_trade(75, 100);
    
    EXPECT_EQ(metrics.trades_executed.get(), 1);
    EXPECT_EQ(metrics.trade_volume.get(), 100);
}

TEST(OrderBookMetricsTest, MultipleRecords) {
    OrderBookMetrics metrics;
    
    metrics.record_add_order(100, true);
    metrics.record_add_order(100, true);
    metrics.record_add_order(100, false);
    metrics.record_cancel_order(50);
    metrics.record_trade(75, 100);
    metrics.record_trade(75, 200);
    
    EXPECT_EQ(metrics.orders_received.get(), 3);
    EXPECT_EQ(metrics.orders_accepted.get(), 2);
    EXPECT_EQ(metrics.orders_rejected.get(), 1);
    EXPECT_EQ(metrics.orders_cancelled.get(), 1);
    EXPECT_EQ(metrics.trades_executed.get(), 2);
    EXPECT_EQ(metrics.trade_volume.get(), 300);
}

TEST(OrderBookMetricsTest, Reset) {
    OrderBookMetrics metrics;
    
    metrics.record_add_order(100, true);
    metrics.record_trade(75, 100);
    
    EXPECT_EQ(metrics.orders_received.get(), 1);
    EXPECT_EQ(metrics.trades_executed.get(), 1);
    
    metrics.reset();
    
    EXPECT_EQ(metrics.orders_received.get(), 0);
    EXPECT_EQ(metrics.trades_executed.get(), 0);
}

TEST(OrderBookMetricsTest, ToString) {
    OrderBookMetrics metrics;
    
    metrics.record_add_order(100, true);
    metrics.record_trade(75, 100);
    
    std::string output = metrics.to_string();
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("OrderBookMetrics"), std::string::npos);
}

TEST(SystemMetricsTest, Singleton) {
    auto& metrics1 = SystemMetrics::instance();
    auto& metrics2 = SystemMetrics::instance();
    
    EXPECT_EQ(&metrics1, &metrics2);
}

TEST(SystemMetricsTest, GetOrderBookMetrics) {
    auto& system_metrics = SystemMetrics::instance();
    system_metrics.reset(); // Reset singleton state
    
    auto& order_book_metrics = system_metrics.get_order_book_metrics("AAPL");
    
    order_book_metrics.record_add_order(100, true);
    EXPECT_EQ(order_book_metrics.orders_received.get(), 1);
}

TEST(SystemMetricsTest, Reset) {
    auto& system_metrics = SystemMetrics::instance();
    
    auto& order_book_metrics = system_metrics.get_order_book_metrics("AAPL");
    order_book_metrics.record_add_order(100, true);
    
    system_metrics.reset();
    
    EXPECT_EQ(order_book_metrics.orders_received.get(), 0);
}

TEST(ScopedLatencyTimerTest, RecordsLatency) {
    LatencyHistogram histogram;
    
    {
        ScopedLatencyTimer timer(histogram);
        // Simulate some work
        volatile int x = 0;
        for (int i = 0; i < 1000; ++i) {
            x += i;
        }
    } // Timer destructor records latency
    
    EXPECT_GT(histogram.get_total_count(), 0);
}

TEST(ScopedLatencyTimerTest, MultipleTimers) {
    LatencyHistogram histogram;
    
    {
        ScopedLatencyTimer timer1(histogram);
        volatile int x = 0;
        for (int i = 0; i < 100; ++i) { x += i; }
    }
    
    {
        ScopedLatencyTimer timer2(histogram);
        volatile int x = 0;
        for (int i = 0; i < 100; ++i) { x += i; }
    }
    
    EXPECT_EQ(histogram.get_total_count(), 2);
}

TEST(OrderBookMetricsTest, LatencyPercentiles) {
    OrderBookMetrics metrics;
    
    for (int i = 1; i <= 100; ++i) {
        metrics.record_add_order(i * 1000, true);
    }
    
    uint64_t p50 = metrics.add_order_latency.percentile(50);
    uint64_t p95 = metrics.add_order_latency.percentile(95);
    uint64_t p99 = metrics.add_order_latency.percentile(99);
    
    EXPECT_GT(p50, 0);
    EXPECT_GE(p95, p50);
    EXPECT_GE(p99, p95);
}

TEST(OrderBookMetricsTest, LastUpdateTime) {
    OrderBookMetrics metrics;
    
    EXPECT_EQ(metrics.last_update_time_ns.load(), 0);
    
    metrics.record_add_order(100, true);
    
    EXPECT_GT(metrics.last_update_time_ns.load(), 0);
}
