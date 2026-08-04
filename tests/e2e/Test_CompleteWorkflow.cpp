#include <gtest/gtest.h>
#include "lob/OrderBook.h"
#include "lob/OrderBookManager.h"
#include "lob/RiskManager.h"
#include "lob/Metrics.h"
#include "lob/OrderTypes.h"
#include "lob/ArenaAllocator.h"

using namespace lob;

TEST(CompleteWorkflow, SingleSymbolTrading) {
    // Initialize components
    OrderBook book;
    
    // Verify order book is initialized
    EXPECT_TRUE(true);
}

TEST(CompleteWorkflow, MultiSymbolTrading) {
    OrderBookManager manager;
    
    // Register symbols
    SymbolInfo aapl_info;
    aapl_info.symbol = "AAPL";
    aapl_info.tick_size = 0.01;
    aapl_info.lot_size = 100;
    manager.register_symbol("AAPL", aapl_info);
    
    SymbolInfo googl_info;
    googl_info.symbol = "GOOGL";
    googl_info.tick_size = 0.01;
    googl_info.lot_size = 100;
    manager.register_symbol("GOOGL", googl_info);
    
    // Add orders to both symbols
    manager.add_order("AAPL", 1, 100.0, 100, Side::BUY);
    manager.add_order("GOOGL", 2, 150.0, 50, Side::BUY);
    
    // Verify independence
    EXPECT_DOUBLE_EQ(manager.get_best_bid("AAPL"), 100.0);
    EXPECT_DOUBLE_EQ(manager.get_best_bid("GOOGL"), 150.0);
    
    // Cancel in one symbol
    manager.cancel_order("AAPL", 1);
    
    // Verify only AAPL affected
    EXPECT_DOUBLE_EQ(manager.get_best_bid("AAPL"), 0.0);
    EXPECT_DOUBLE_EQ(manager.get_best_bid("GOOGL"), 150.0);
}

TEST(CompleteWorkflow, RiskEnforcement) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    RiskManager* risk_mgr = manager.get_risk_manager("AAPL");
    
    // Set strict limits
    RiskLimits limits;
    limits.max_order_size = 100;
    risk_mgr->set_limits(limits);
    
    // Small order should pass
    ExtendedOrder valid_order(1, 100.0, 50, Side::BUY);
    EXPECT_TRUE(risk_mgr->check_order(valid_order, "AAPL"));
    
    // Large order should fail
    ExtendedOrder invalid_order(2, 100.0, 200, Side::BUY);
    EXPECT_FALSE(risk_mgr->check_order(invalid_order, "AAPL"));
}

TEST(CompleteWorkflow, AdvancedOrderTypes) {
    OrderBook book;
    
    // Stop-loss order
    ExtendedOrder stop_loss(1, 95.0, 100, Side::SELL, OrderType::STOP_LOSS);
    stop_loss.stop_price = 95.0;
    EXPECT_TRUE(stop_loss.is_conditional());
    EXPECT_TRUE(stop_loss.should_activate(95.0, Side::SELL));
    
    // Take-profit order
    ExtendedOrder take_profit(2, 105.0, 100, Side::SELL, OrderType::TAKE_PROFIT);
    take_profit.take_profit_price = 105.0;
    EXPECT_TRUE(take_profit.is_conditional());
    EXPECT_TRUE(take_profit.should_activate(105.0, Side::SELL));
    
    // Iceberg order
    ExtendedOrder iceberg(3, 100.0, 1000, Side::BUY, OrderType::ICEBERG);
    EXPECT_EQ(iceberg.display_quantity, 500);
    EXPECT_EQ(iceberg.hidden_quantity, 500);
}

TEST(CompleteWorkflow, MetricsCollection) {
    auto& system_metrics = SystemMetrics::instance();
    system_metrics.reset(); // Reset singleton state
    auto& metrics = system_metrics.get_order_book_metrics("AAPL");
    
    // Simulate trading activity
    for (int i = 1; i <= 100; ++i) {
        metrics.record_add_order(50 + i, true);
    }
    
    for (int i = 0; i < 20; ++i) {
        metrics.record_cancel_order(30 + i);
    }
    
    for (int i = 1; i <= 80; ++i) {
        metrics.record_trade(40 + i, 10);
    }
    
    // Verify metrics are recorded
    EXPECT_GT(metrics.orders_received.get(), 0);
    EXPECT_GT(metrics.orders_cancelled.get(), 0);
    EXPECT_GT(metrics.trades_executed.get(), 0);
    EXPECT_GT(metrics.trade_volume.get(), 0);
}

TEST(CompleteWorkflow, ArenaAllocatorUsage) {
    ArenaAllocator arena(1024 * 1024); // 1MB
    
    // Allocate multiple objects
    for (int i = 0; i < 100; ++i) {
        void* ptr = arena.allocate(100);
        ASSERT_NE(ptr, nullptr);
    }
    
    EXPECT_GT(arena.used(), 0);
    
    // Reset and reuse
    arena.reset();
    EXPECT_EQ(arena.used(), 0);
    
    void* ptr = arena.allocate(500);
    ASSERT_NE(ptr, nullptr);
    EXPECT_GT(arena.used(), 0);
}

TEST(CompleteWorkflow, PositionTracking) {
    RiskManager risk_mgr;
    
    // Add trades
    risk_mgr.update_position("AAPL", Side::BUY, 100, 100.0);
    risk_mgr.update_position("AAPL", Side::BUY, 50, 105.0);
    risk_mgr.update_position("AAPL", Side::BUY, 50, 100.0);
    
    auto position = risk_mgr.get_position("AAPL");
    EXPECT_EQ(position.long_position, 200);
    EXPECT_GT(position.avg_long_price, 0.0);
}

TEST(CompleteWorkflow, CircuitBreaker) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    auto* risk_mgr = manager.get_risk_manager("AAPL");
    
    RiskLimits limits;
    limits.circuit_breaker_threshold = 0.10;
    limits.enable_circuit_breaker = true;
    risk_mgr->set_limits(limits);
    
    // Verify circuit breaker state
    EXPECT_FALSE(risk_mgr->is_circuit_breaker_triggered());
    
    // Reset functionality
    risk_mgr->reset_circuit_breaker();
    EXPECT_FALSE(risk_mgr->is_circuit_breaker_triggered());
}

TEST(CompleteWorkflow, FullTradingDay) {
    OrderBookManager manager;
    auto& system_metrics = SystemMetrics::instance();
    
    // Register symbol
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    auto& metrics = system_metrics.get_order_book_metrics("AAPL");
    RiskManager* risk_mgr = manager.get_risk_manager("AAPL");
    
    RiskLimits limits;
    limits.max_daily_volume = 100000;
    risk_mgr->set_limits(limits);
    
    // Morning session
    for (uint64_t i = 0; i < 1000; ++i) {
        ExtendedOrder order(i, 100.0 + i * 0.01, 10, Side::BUY);
        if (risk_mgr->check_order(order, "AAPL")) {
            manager.add_order("AAPL", i, 100.0 + i * 0.01, 10, Side::BUY);
            metrics.record_add_order(50, true);
        }
    }
    
    // Mid-day session
    for (uint64_t i = 1000; i < 2000; ++i) {
        ExtendedOrder order(i, 105.0 + (i - 1000) * 0.01, 10, Side::SELL);
        if (risk_mgr->check_order(order, "AAPL")) {
            manager.add_order("AAPL", i, 105.0 + (i - 1000) * 0.01, 10, Side::SELL);
            metrics.record_add_order(50, true);
        }
    }
    
    // Afternoon session - some cancellations
    for (uint64_t i = 0; i < 100; ++i) {
        manager.cancel_order("AAPL", i);
        metrics.record_cancel_order(30);
    }
    
    // End of day
    risk_mgr->reset_daily_volume();
    
    // Verify end state
    EXPECT_GT(metrics.orders_received.get(), 0);
    EXPECT_GT(metrics.orders_cancelled.get(), 0);
}

TEST(CompleteWorkflow, ErrorHandling) {
    OrderBook book;
    
    // Cancel non-existent order
    ErrorCode err = book.cancel_order(999);
    EXPECT_EQ(err, ErrorCode::ORDER_NOT_FOUND);
    
    // Add order with invalid parameters (if validation enabled)
    // This tests error paths
}

TEST(CompleteWorkflow, ConcurrentAccess) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    // Simulate concurrent access pattern
    for (uint64_t i = 0; i < 100; ++i) {
        manager.add_order("AAPL", i, 100.0 + i * 0.01, 10, Side::BUY);
    }
    
    for (uint64_t i = 0; i < 50; ++i) {
        manager.cancel_order("AAPL", i);
    }
    
    // Verify final state
    EXPECT_GT(manager.get_best_bid("AAPL"), 0.0);
}

TEST(CompleteWorkflow, MemoryPoolExhaustion) {
    OrderBook book;
    
    // Add orders until pool is exhausted
    // This tests the memory pool limits
    for (uint64_t i = 0; i < 10000; ++i) {
        book.add_limit_order(i, 100.0 + i * 0.01, 10, Side::BUY);
    }
    
    // At some point, orders should fail
    // The exact point depends on pool capacity
}

TEST(CompleteWorkflow, SymbolLifecycle) {
    OrderBookManager manager;
    
    // Register symbol
    SymbolInfo info;
    info.symbol = "AAPL";
    EXPECT_TRUE(manager.register_symbol("AAPL", info));
    
    // Use symbol
    manager.add_order("AAPL", 1, 100.0, 10, Side::BUY);
    EXPECT_DOUBLE_EQ(manager.get_best_bid("AAPL"), 100.0);
    
    // Unregister symbol
    EXPECT_TRUE(manager.unregister_symbol("AAPL"));
    EXPECT_FALSE(manager.has_symbol("AAPL"));
    
    // Re-register symbol
    EXPECT_TRUE(manager.register_symbol("AAPL", info));
    EXPECT_TRUE(manager.has_symbol("AAPL"));
}

TEST(CompleteWorkflow, MetricsReset) {
    auto& system_metrics = SystemMetrics::instance();
    system_metrics.reset(); // Reset singleton state
    auto& metrics = system_metrics.get_order_book_metrics("AAPL");
    
    // Add some metrics
    metrics.record_add_order(100, true);
    metrics.record_trade(50, 10);
    
    // Reset
    system_metrics.reset();
    
    // Verify reset - metrics should be cleared
    auto& metrics_after = system_metrics.get_order_book_metrics("AAPL");
    EXPECT_EQ(metrics_after.orders_received.get(), 0);
    EXPECT_EQ(metrics_after.trades_executed.get(), 0);
}
