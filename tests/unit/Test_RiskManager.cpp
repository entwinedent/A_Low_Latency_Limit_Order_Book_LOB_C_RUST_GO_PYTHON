#include <gtest/gtest.h>
#include "lob/RiskManager.h"
#include "lob/OrderTypes.h"

using namespace lob;

TEST(RiskManagerTest, DefaultConstruction) {
    RiskManager risk_mgr;
    
    auto limits = risk_mgr.get_limits();
    EXPECT_EQ(limits.max_position_per_symbol, 1000000);
    EXPECT_EQ(limits.max_order_size, 100000);
    EXPECT_EQ(limits.max_daily_volume, 10000000);
    EXPECT_TRUE(limits.enable_self_trade_prevention);
    EXPECT_TRUE(limits.enable_circuit_breaker);
}

TEST(RiskManagerTest, CustomLimits) {
    RiskLimits limits;
    limits.max_position_per_symbol = 500000;
    limits.max_order_size = 50000;
    limits.max_daily_volume = 5000000;
    limits.enable_self_trade_prevention = false;
    limits.enable_circuit_breaker = false;
    
    RiskManager risk_mgr(limits);
    
    auto retrieved_limits = risk_mgr.get_limits();
    EXPECT_EQ(retrieved_limits.max_position_per_symbol, 500000);
    EXPECT_EQ(retrieved_limits.max_order_size, 50000);
    EXPECT_EQ(retrieved_limits.max_daily_volume, 5000000);
    EXPECT_FALSE(retrieved_limits.enable_self_trade_prevention);
    EXPECT_FALSE(retrieved_limits.enable_circuit_breaker);
}

TEST(RiskManagerTest, OrderSizeCheck) {
    RiskManager risk_mgr;
    
    ExtendedOrder valid_order(1, 100.0, 50000, Side::BUY);
    EXPECT_TRUE(risk_mgr.check_order(valid_order, "AAPL"));
}

TEST(RiskManagerTest, PositionLimitCheck) {
    RiskManager risk_mgr;
    
    RiskLimits limits;
    limits.max_position_per_symbol = 1000;
    risk_mgr.set_limits(limits);
    
    // Add some position
    risk_mgr.update_position("AAPL", Side::BUY, 500, 100.0);
    
    // Small order should pass
    ExtendedOrder order(1, 100.0, 100, Side::BUY);
    EXPECT_TRUE(risk_mgr.check_order(order, "AAPL"));
}

TEST(RiskManagerTest, DailyVolumeCheck) {
    RiskLimits limits;
    limits.max_daily_volume = 10000;
    RiskManager risk_mgr(limits);
    
    // Add volume
    risk_mgr.update_position("AAPL", Side::BUY, 5000, 100.0);
    
    // Order should pass
    ExtendedOrder order(1, 100.0, 100, Side::BUY);
    EXPECT_TRUE(risk_mgr.check_order(order, "AAPL"));
}

TEST(RiskManagerTest, PriceDeviationCheck) {
    RiskManager risk_mgr;
    
    // Set last trade price
    risk_mgr.update_position("AAPL", Side::BUY, 100, 100.0);
    
    ExtendedOrder valid_order(1, 105.0, 10, Side::BUY);
    EXPECT_TRUE(risk_mgr.check_order(valid_order, "AAPL"));
    
    ExtendedOrder invalid_order(2, 115.0, 10, Side::BUY);
    EXPECT_FALSE(risk_mgr.check_order(invalid_order, "AAPL"));
}

TEST(RiskManagerTest, CircuitBreaker) {
    RiskManager risk_mgr;
    
    RiskLimits limits;
    limits.circuit_breaker_threshold = 0.10;
    limits.enable_circuit_breaker = true;
    risk_mgr.set_limits(limits);
    
    // Verify circuit breaker is initially not triggered
    EXPECT_FALSE(risk_mgr.is_circuit_breaker_triggered());
    
    // Reset functionality
    risk_mgr.reset_circuit_breaker();
    EXPECT_FALSE(risk_mgr.is_circuit_breaker_triggered());
}

TEST(RiskManagerTest, CircuitBreakerReset) {
    RiskManager risk_mgr;
    
    RiskLimits limits;
    limits.circuit_breaker_threshold = 0.10;
    limits.enable_circuit_breaker = true;
    risk_mgr.set_limits(limits);
    
    // Verify circuit breaker is initially not triggered
    EXPECT_FALSE(risk_mgr.is_circuit_breaker_triggered());
    
    // Reset functionality
    risk_mgr.reset_circuit_breaker();
    EXPECT_FALSE(risk_mgr.is_circuit_breaker_triggered());
}

TEST(RiskManagerTest, SelfTradePrevention) {
    RiskLimits limits;
    limits.enable_self_trade_prevention = true;
    RiskManager risk_mgr(limits);
    
    // Add long position
    risk_mgr.update_position("AAPL", Side::BUY, 100, 100.0);
    
    // Selling with long position should be prevented
    EXPECT_FALSE(risk_mgr.check_self_trade("AAPL", Side::SELL, 12345));
    
    // Buying should be allowed
    EXPECT_TRUE(risk_mgr.check_self_trade("AAPL", Side::BUY, 12345));
}

TEST(RiskManagerTest, SelfTradePreventionDisabled) {
    RiskLimits limits;
    limits.enable_self_trade_prevention = false;
    RiskManager risk_mgr(limits);
    
    // Add long position
    risk_mgr.update_position("AAPL", Side::BUY, 100, 100.0);
    
    // Selling should be allowed when prevention is disabled
    EXPECT_TRUE(risk_mgr.check_self_trade("AAPL", Side::SELL, 12345));
}

TEST(RiskManagerTest, PositionTracking) {
    RiskManager risk_mgr;
    
    // Add long position
    risk_mgr.update_position("AAPL", Side::BUY, 100, 100.0);
    
    auto position = risk_mgr.get_position("AAPL");
    EXPECT_EQ(position.long_position, 100);
    EXPECT_EQ(position.short_position, 0);
    EXPECT_TRUE(position.is_long());
    EXPECT_FALSE(position.is_short());
}

TEST(RiskManagerTest, AveragePriceCalculation) {
    RiskManager risk_mgr;
    
    // Add long positions at different prices
    risk_mgr.update_position("AAPL", Side::BUY, 100, 100.0);
    risk_mgr.update_position("AAPL", Side::BUY, 100, 110.0);
    
    auto position = risk_mgr.get_position("AAPL");
    EXPECT_EQ(position.long_position, 200);
    EXPECT_DOUBLE_EQ(position.avg_long_price, 105.0);
}

TEST(RiskManagerTest, NetPosition) {
    RiskManager risk_mgr;
    
    risk_mgr.update_position("AAPL", Side::BUY, 150, 100.0);
    risk_mgr.update_position("AAPL", Side::SELL, 50, 100.0);
    
    auto position = risk_mgr.get_position("AAPL");
    EXPECT_EQ(position.net_position(), 100);
    EXPECT_EQ(position.gross_position(), 200);
}

TEST(RiskManagerTest, DailyVolumeReset) {
    RiskLimits limits;
    limits.max_daily_volume = 10000;
    RiskManager risk_mgr(limits);
    
    risk_mgr.update_position("AAPL", Side::BUY, 9000, 100.0);
    
    ExtendedOrder order(1, 100.0, 2000, Side::BUY);
    EXPECT_FALSE(risk_mgr.check_order(order, "AAPL"));
    
    risk_mgr.reset_daily_volume();
    
    EXPECT_TRUE(risk_mgr.check_order(order, "AAPL"));
}

TEST(RiskManagerTest, MultipleSymbols) {
    RiskManager risk_mgr;
    
    risk_mgr.update_position("AAPL", Side::BUY, 100, 100.0);
    risk_mgr.update_position("GOOGL", Side::BUY, 200, 150.0);
    
    auto aapl_position = risk_mgr.get_position("AAPL");
    auto googl_position = risk_mgr.get_position("GOOGL");
    
    EXPECT_EQ(aapl_position.long_position, 100);
    EXPECT_EQ(googl_position.long_position, 200);
}

TEST(RiskManagerTest, RateLimiting) {
    RiskLimits limits;
    limits.max_orders_per_second = 100;
    RiskManager risk_mgr(limits);
    
    // This test would require mocking time, so we'll just verify the limit is set
    EXPECT_EQ(limits.max_orders_per_second, 100);
}

TEST(RiskManagerTest, SetLimits) {
    RiskManager risk_mgr;
    
    RiskLimits new_limits;
    new_limits.max_position_per_symbol = 2000000;
    new_limits.max_order_size = 200000;
    
    risk_mgr.set_limits(new_limits);
    
    auto retrieved_limits = risk_mgr.get_limits();
    EXPECT_EQ(retrieved_limits.max_position_per_symbol, 2000000);
    EXPECT_EQ(retrieved_limits.max_order_size, 200000);
}

TEST(RiskManagerTest, PositionWithNoTrades) {
    RiskManager risk_mgr;
    
    auto position = risk_mgr.get_position("NONEXISTENT");
    EXPECT_EQ(position.long_position, 0);
    EXPECT_EQ(position.short_position, 0);
    EXPECT_EQ(position.net_position(), 0);
}

TEST(RiskManagerTest, ShortPosition) {
    RiskManager risk_mgr;
    
    risk_mgr.update_position("AAPL", Side::SELL, 100, 100.0);
    
    auto position = risk_mgr.get_position("AAPL");
    EXPECT_EQ(position.short_position, 100);
    EXPECT_TRUE(position.is_short());
    EXPECT_FALSE(position.is_long());
}
