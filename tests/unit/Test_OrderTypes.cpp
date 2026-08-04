#include <gtest/gtest.h>
#include "lob/OrderTypes.h"
#include "lob/Common.h"

using namespace lob;

TEST(OrderTypesTest, BasicOrderConstruction) {
    ExtendedOrder order(1, 100.0, 10, Side::BUY);
    
    EXPECT_EQ(order.id, 1);
    EXPECT_DOUBLE_EQ(order.price, 100.0);
    EXPECT_EQ(order.quantity, 10);
    EXPECT_EQ(order.side, Side::BUY);
    EXPECT_EQ(order.order_type, OrderType::LIMIT);
    EXPECT_EQ(order.time_in_force, TimeInForce::GTC);
}

TEST(OrderTypesTest, StopLossOrder) {
    ExtendedOrder order(1, 100.0, 10, Side::SELL, OrderType::STOP_LOSS);
    order.stop_price = 95.0;
    
    EXPECT_TRUE(order.is_conditional());
    EXPECT_EQ(order.order_type, OrderType::STOP_LOSS);
    EXPECT_DOUBLE_EQ(order.stop_price, 95.0);
}

TEST(OrderTypesTest, TakeProfitOrder) {
    ExtendedOrder order(1, 100.0, 10, Side::SELL, OrderType::TAKE_PROFIT);
    order.take_profit_price = 105.0;
    
    EXPECT_TRUE(order.is_conditional());
    EXPECT_EQ(order.order_type, OrderType::TAKE_PROFIT);
    EXPECT_DOUBLE_EQ(order.take_profit_price, 105.0);
}

TEST(OrderTypesTest, IcebergOrder) {
    ExtendedOrder order(1, 100.0, 1000, Side::BUY, OrderType::ICEBERG);
    
    EXPECT_EQ(order.order_type, OrderType::ICEBERG);
    EXPECT_EQ(order.display_quantity, 500);  // Half of 1000
    EXPECT_EQ(order.hidden_quantity, 500);   // Half of 1000
}

TEST(OrderTypesTest, TrailingStopOrder) {
    ExtendedOrder order(1, 100.0, 10, Side::SELL, OrderType::TRAILING_STOP);
    order.trail_amount = 5.0;
    order.activation_price = 105.0;
    
    EXPECT_TRUE(order.is_conditional());
    EXPECT_EQ(order.order_type, OrderType::TRAILING_STOP);
    EXPECT_DOUBLE_EQ(order.trail_amount, 5.0);
    EXPECT_DOUBLE_EQ(order.activation_price, 105.0);
}

TEST(OrderTypesTest, StopLimitOrder) {
    ExtendedOrder order(1, 100.0, 10, Side::BUY, OrderType::STOP_LIMIT);
    order.stop_price = 95.0;
    
    EXPECT_TRUE(order.is_conditional());
    EXPECT_EQ(order.order_type, OrderType::STOP_LIMIT);
}

TEST(OrderTypesTest, FOKOrder) {
    ExtendedOrder order(1, 100.0, 10, Side::BUY, OrderType::FOK);
    
    EXPECT_EQ(order.order_type, OrderType::FOK);
    EXPECT_EQ(order.time_in_force, TimeInForce::GTC);
}

TEST(OrderTypesTest, AONOrder) {
    ExtendedOrder order(1, 100.0, 10, Side::BUY, OrderType::AON);
    
    EXPECT_EQ(order.order_type, OrderType::AON);
}

TEST(OrderTypesTest, TimeInForceIOC) {
    ExtendedOrder order(1, 100.0, 10, Side::BUY, OrderType::LIMIT, TimeInForce::IOC);
    
    EXPECT_EQ(order.time_in_force, TimeInForce::IOC);
}

TEST(OrderTypesTest, TimeInForceFOK) {
    ExtendedOrder order(1, 100.0, 10, Side::BUY, OrderType::LIMIT, TimeInForce::FOK);
    
    EXPECT_EQ(order.time_in_force, TimeInForce::FOK);
}

TEST(OrderTypesTest, TimeInForceDAY) {
    ExtendedOrder order(1, 100.0, 10, Side::BUY, OrderType::LIMIT, TimeInForce::DAY);
    
    EXPECT_EQ(order.time_in_force, TimeInForce::DAY);
}

TEST(OrderTypesTest, StopLossActivationBuy) {
    ExtendedOrder order(1, 100.0, 10, Side::BUY, OrderType::STOP_LOSS);
    order.stop_price = 105.0;
    
    // Should activate when price >= stop_price for buy
    EXPECT_TRUE(order.should_activate(105.0, Side::BUY));
    EXPECT_TRUE(order.should_activate(106.0, Side::BUY));
    EXPECT_FALSE(order.should_activate(104.0, Side::BUY));
}

TEST(OrderTypesTest, StopLossActivationSell) {
    ExtendedOrder order(1, 100.0, 10, Side::SELL, OrderType::STOP_LOSS);
    order.stop_price = 95.0;
    
    // Should activate when price <= stop_price for sell
    EXPECT_TRUE(order.should_activate(95.0, Side::SELL));
    EXPECT_TRUE(order.should_activate(94.0, Side::SELL));
    EXPECT_FALSE(order.should_activate(96.0, Side::SELL));
}

TEST(OrderTypesTest, TakeProfitActivationBuy) {
    ExtendedOrder order(1, 100.0, 10, Side::BUY, OrderType::TAKE_PROFIT);
    order.take_profit_price = 95.0;
    
    // Should activate when price <= take_profit_price for buy
    EXPECT_TRUE(order.should_activate(95.0, Side::BUY));
    EXPECT_TRUE(order.should_activate(94.0, Side::BUY));
    EXPECT_FALSE(order.should_activate(96.0, Side::BUY));
}

TEST(OrderTypesTest, TakeProfitActivationSell) {
    ExtendedOrder order(1, 100.0, 10, Side::SELL, OrderType::TAKE_PROFIT);
    order.take_profit_price = 105.0;
    
    // Should activate when price >= take_profit_price for sell
    EXPECT_TRUE(order.should_activate(105.0, Side::SELL));
    EXPECT_TRUE(order.should_activate(106.0, Side::SELL));
    EXPECT_FALSE(order.should_activate(104.0, Side::SELL));
}

TEST(OrderTypesTest, TrailingStopActivationSell) {
    ExtendedOrder order(1, 100.0, 10, Side::SELL, OrderType::TRAILING_STOP);
    order.trail_amount = 5.0;
    order.activation_price = 105.0;
    
    // Should activate when price >= activation_price for sell
    EXPECT_TRUE(order.should_activate(105.0, Side::SELL));
    EXPECT_TRUE(order.should_activate(106.0, Side::SELL));
    EXPECT_FALSE(order.should_activate(104.0, Side::SELL));
}

TEST(OrderTypesTest, TrailingStopActivationBuy) {
    ExtendedOrder order(1, 100.0, 10, Side::BUY, OrderType::TRAILING_STOP);
    order.trail_amount = 5.0;
    order.activation_price = 95.0;
    
    // Should activate when price <= activation_price for buy
    EXPECT_TRUE(order.should_activate(95.0, Side::BUY));
    EXPECT_TRUE(order.should_activate(94.0, Side::BUY));
    EXPECT_FALSE(order.should_activate(96.0, Side::BUY));
}

TEST(OrderTypesTest, NonConditionalOrder) {
    ExtendedOrder order(1, 100.0, 10, Side::BUY, OrderType::LIMIT);
    
    EXPECT_FALSE(order.is_conditional());
    EXPECT_FALSE(order.should_activate(100.0, Side::BUY));
}

TEST(OrderTypesTest, InactiveConditionalOrder) {
    ExtendedOrder order(1, 100.0, 10, Side::BUY, OrderType::STOP_LOSS);
    order.stop_price = 105.0;
    order.is_active = false;
    
    EXPECT_TRUE(order.is_conditional());
    EXPECT_FALSE(order.should_activate(106.0, Side::BUY));
}

TEST(OrderTypesTest, MarketOrder) {
    ExtendedOrder order(1, 0.0, 10, Side::BUY, OrderType::MARKET);
    
    EXPECT_EQ(order.order_type, OrderType::MARKET);
    EXPECT_DOUBLE_EQ(order.price, 0.0);  // Market orders have no price
}

TEST(OrderTypesTest, OrderTypeValues) {
    EXPECT_EQ(static_cast<int>(OrderType::LIMIT), 0);
    EXPECT_EQ(static_cast<int>(OrderType::MARKET), 1);
    EXPECT_EQ(static_cast<int>(OrderType::STOP_LOSS), 2);
    EXPECT_EQ(static_cast<int>(OrderType::TAKE_PROFIT), 3);
    EXPECT_EQ(static_cast<int>(OrderType::STOP_LIMIT), 4);
    EXPECT_EQ(static_cast<int>(OrderType::ICEBERG), 5);
    EXPECT_EQ(static_cast<int>(OrderType::TRAILING_STOP), 6);
    EXPECT_EQ(static_cast<int>(OrderType::FOK), 7);
    EXPECT_EQ(static_cast<int>(OrderType::AON), 8);
}

TEST(OrderTypesTest, TimeInForceValues) {
    EXPECT_EQ(static_cast<int>(TimeInForce::GTC), 0);
    EXPECT_EQ(static_cast<int>(TimeInForce::IOC), 1);
    EXPECT_EQ(static_cast<int>(TimeInForce::FOK), 2);
    EXPECT_EQ(static_cast<int>(TimeInForce::DAY), 3);
}
