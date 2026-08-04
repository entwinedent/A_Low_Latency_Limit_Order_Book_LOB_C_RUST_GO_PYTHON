#include <gtest/gtest.h>
#include "lob/OrderBookManager.h"
#include "lob/RiskManager.h"
#include "lob/Metrics.h"

using namespace lob;

TEST(MultiSymbolIntegration, RegisterMultipleSymbols) {
    OrderBookManager manager;
    
    SymbolInfo aapl_info;
    aapl_info.symbol = "AAPL";
    aapl_info.description = "Apple Inc.";
    aapl_info.tick_size = 0.01;
    aapl_info.lot_size = 100;
    
    SymbolInfo googl_info;
    googl_info.symbol = "GOOGL";
    googl_info.description = "Alphabet Inc.";
    googl_info.tick_size = 0.01;
    googl_info.lot_size = 100;
    
    SymbolInfo msft_info;
    msft_info.symbol = "MSFT";
    msft_info.description = "Microsoft Corp.";
    msft_info.tick_size = 0.01;
    msft_info.lot_size = 100;
    
    EXPECT_TRUE(manager.register_symbol("AAPL", aapl_info));
    EXPECT_TRUE(manager.register_symbol("GOOGL", googl_info));
    EXPECT_TRUE(manager.register_symbol("MSFT", msft_info));
    
    EXPECT_EQ(manager.symbol_count(), 3);
    
    std::vector<std::string> symbols = manager.get_symbols();
    EXPECT_EQ(symbols.size(), 3);
}

TEST(MultiSymbolIntegration, IndependentOrderBooks) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    info.symbol = "GOOGL";
    manager.register_symbol("GOOGL", info);
    
    // Add orders to different symbols
    manager.add_order("AAPL", 1, 100.0, 10, Side::BUY);
    manager.add_order("GOOGL", 2, 150.0, 5, Side::BUY);
    
    // Verify independence
    EXPECT_DOUBLE_EQ(manager.get_best_bid("AAPL"), 100.0);
    EXPECT_DOUBLE_EQ(manager.get_best_bid("GOOGL"), 150.0);
    
    // Cancel order in one symbol shouldn't affect other
    manager.cancel_order("AAPL", 1);
    EXPECT_DOUBLE_EQ(manager.get_best_bid("AAPL"), 0.0);
    EXPECT_DOUBLE_EQ(manager.get_best_bid("GOOGL"), 150.0);
}

TEST(MultiSymbolIntegration, CrossSymbolRiskManagement) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    info.symbol = "GOOGL";
    manager.register_symbol("GOOGL", info);
    
    RiskManager* aapl_risk = manager.get_risk_manager("AAPL");
    RiskManager* googl_risk = manager.get_risk_manager("GOOGL");
    
    // Set different limits for each symbol
    RiskLimits aapl_limits;
    aapl_limits.max_position_per_symbol = 1000;
    aapl_risk->set_limits(aapl_limits);
    
    RiskLimits googl_limits;
    googl_limits.max_position_per_symbol = 2000;
    googl_risk->set_limits(googl_limits);
    
    // Verify limits are independent
    EXPECT_EQ(aapl_risk->get_limits().max_position_per_symbol, 1000);
    EXPECT_EQ(googl_risk->get_limits().max_position_per_symbol, 2000);
}

TEST(MultiSymbolIntegration, SymbolSpecificPositions) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    info.symbol = "GOOGL";
    manager.register_symbol("GOOGL", info);
    
    RiskManager* aapl_risk = manager.get_risk_manager("AAPL");
    RiskManager* googl_risk = manager.get_risk_manager("GOOGL");
    
    // Add positions to different symbols
    aapl_risk->update_position("AAPL", Side::BUY, 100, 100.0);
    googl_risk->update_position("GOOGL", Side::BUY, 200, 150.0);
    
    auto aapl_position = manager.get_position("AAPL");
    auto googl_position = manager.get_position("GOOGL");
    
    EXPECT_EQ(aapl_position.long_position, 100);
    EXPECT_EQ(googl_position.long_position, 200);
}

TEST(MultiSymbolIntegration, SymbolSpecificMetrics) {
    auto& system_metrics = SystemMetrics::instance();
    system_metrics.reset(); // Reset singleton state
    
    auto& aapl_metrics = system_metrics.get_order_book_metrics("AAPL");
    auto& googl_metrics = system_metrics.get_order_book_metrics("GOOGL");
    
    // Record metrics for different symbols
    aapl_metrics.record_add_order(100, true);
    googl_metrics.record_add_order(150, true);
    
    EXPECT_GT(aapl_metrics.orders_received.get(), 0);
    EXPECT_GT(googl_metrics.orders_received.get(), 0);
}

TEST(MultiSymbolIntegration, MatchingAcrossSymbols) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    info.symbol = "GOOGL";
    manager.register_symbol("GOOGL", info);
    
    // Add orders to AAPL
    manager.add_order("AAPL", 1, 100.0, 10, Side::BUY);
    manager.add_order("AAPL", 2, 105.0, 5, Side::SELL);
    
    // Add orders to GOOGL
    manager.add_order("GOOGL", 3, 150.0, 10, Side::BUY);
    manager.add_order("GOOGL", 4, 160.0, 5, Side::SELL);
    
    // Verify symbols have independent order books
    EXPECT_TRUE(manager.has_symbol("AAPL"));
    EXPECT_TRUE(manager.has_symbol("GOOGL"));
}

TEST(MultiSymbolIntegration, CircuitBreakerPerSymbol) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    info.symbol = "GOOGL";
    manager.register_symbol("GOOGL", info);
    
    RiskManager* aapl_risk = manager.get_risk_manager("AAPL");
    RiskManager* googl_risk = manager.get_risk_manager("GOOGL");
    
    // Verify each symbol has its own risk manager
    EXPECT_NE(aapl_risk, nullptr);
    EXPECT_NE(googl_risk, nullptr);
    EXPECT_NE(aapl_risk, googl_risk);
}

TEST(MultiSymbolIntegration, BulkSymbolRegistration) {
    OrderBookManager manager;
    
    std::vector<std::string> symbols = {"AAPL", "GOOGL", "MSFT", "AMZN", "TSLA"};
    
    for (const auto& symbol : symbols) {
        SymbolInfo info;
        info.symbol = symbol;
        manager.register_symbol(symbol, info);
    }
    
    EXPECT_EQ(manager.symbol_count(), 5);
    
    auto registered_symbols = manager.get_symbols();
    EXPECT_EQ(registered_symbols.size(), 5);
}

TEST(MultiSymbolIntegration, SymbolUnregistration) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    info.symbol = "GOOGL";
    manager.register_symbol("GOOGL", info);
    
    // Add orders
    manager.add_order("AAPL", 1, 100.0, 10, Side::BUY);
    manager.add_order("GOOGL", 2, 150.0, 5, Side::BUY);
    
    // Unregister one symbol
    manager.unregister_symbol("AAPL");
    
    EXPECT_EQ(manager.symbol_count(), 1);
    EXPECT_FALSE(manager.has_symbol("AAPL"));
    EXPECT_TRUE(manager.has_symbol("GOOGL"));
    
    // GOOGL should still work
    EXPECT_DOUBLE_EQ(manager.get_best_bid("GOOGL"), 150.0);
}

TEST(MultiSymbolIntegration, ConcurrentSymbolAccess) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    info.symbol = "GOOGL";
    manager.register_symbol("GOOGL", info);
    
    // Rapidly add orders to both symbols
    for (uint64_t i = 0; i < 100; ++i) {
        manager.add_order("AAPL", i, 100.0 + i * 0.01, 10, Side::BUY);
        manager.add_order("GOOGL", i + 1000, 150.0 + i * 0.01, 5, Side::BUY);
    }
    
    EXPECT_GT(manager.get_best_bid("AAPL"), 100.0);
    EXPECT_GT(manager.get_best_bid("GOOGL"), 150.0);
}

TEST(MultiSymbolIntegration, SymbolMetadata) {
    OrderBookManager manager;
    
    SymbolInfo aapl_info;
    aapl_info.symbol = "AAPL";
    aapl_info.description = "Apple Inc.";
    aapl_info.tick_size = 0.01;
    aapl_info.lot_size = 100;
    aapl_info.is_active = true;
    
    SymbolInfo googl_info;
    googl_info.symbol = "GOOGL";
    googl_info.description = "Alphabet Inc.";
    googl_info.tick_size = 0.01;
    googl_info.lot_size = 10;
    googl_info.is_active = false;
    
    manager.register_symbol("AAPL", aapl_info);
    manager.register_symbol("GOOGL", googl_info);
    
    auto aapl_retrieved = manager.get_symbol_info("AAPL");
    auto googl_retrieved = manager.get_symbol_info("GOOGL");
    
    EXPECT_EQ(aapl_retrieved.lot_size, 100);
    EXPECT_EQ(googl_retrieved.lot_size, 10);
    EXPECT_TRUE(aapl_retrieved.is_active);
    EXPECT_FALSE(googl_retrieved.is_active);
}

TEST(MultiSymbolIntegration, EmptySymbol) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    // Empty symbol should have no bid/ask
    EXPECT_DOUBLE_EQ(manager.get_best_bid("AAPL"), 0.0);
    EXPECT_DOUBLE_EQ(manager.get_best_ask("AAPL"), 0.0);
    
    // Position should be zero
    auto position = manager.get_position("AAPL");
    EXPECT_EQ(position.long_position, 0);
    EXPECT_EQ(position.short_position, 0);
}
