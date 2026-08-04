#include <gtest/gtest.h>
#include "lob/OrderBookManager.h"
#include "lob/OrderBook.h"
#include <algorithm>
#include <vector>

using namespace lob;

TEST(OrderBookManagerTest, DefaultConstruction) {
    OrderBookManager manager;
    
    EXPECT_EQ(manager.symbol_count(), 0);
    EXPECT_FALSE(manager.has_symbol("AAPL"));
}

TEST(OrderBookManagerTest, RegisterSymbol) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    info.description = "Apple Inc.";
    info.tick_size = 0.01;
    info.lot_size = 100;
    info.is_active = true;
    
    EXPECT_TRUE(manager.register_symbol("AAPL", info));
    EXPECT_EQ(manager.symbol_count(), 1);
    EXPECT_TRUE(manager.has_symbol("AAPL"));
}

TEST(OrderBookManagerTest, RegisterDuplicateSymbol) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    
    EXPECT_TRUE(manager.register_symbol("AAPL", info));
    EXPECT_FALSE(manager.register_symbol("AAPL", info)); // Should fail
    EXPECT_EQ(manager.symbol_count(), 1);
}

TEST(OrderBookManagerTest, UnregisterSymbol) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    
    manager.register_symbol("AAPL", info);
    EXPECT_TRUE(manager.has_symbol("AAPL"));
    
    EXPECT_TRUE(manager.unregister_symbol("AAPL"));
    EXPECT_FALSE(manager.has_symbol("AAPL"));
    EXPECT_EQ(manager.symbol_count(), 0);
}

TEST(OrderBookManagerTest, UnregisterNonexistentSymbol) {
    OrderBookManager manager;
    
    EXPECT_FALSE(manager.unregister_symbol("NONEXISTENT"));
}

TEST(OrderBookManagerTest, GetOrderBook) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    OrderBook* book = manager.get_order_book("AAPL");
    ASSERT_NE(book, nullptr);
    
    OrderBook* nonexistent = manager.get_order_book("NONEXISTENT");
    EXPECT_EQ(nonexistent, nullptr);
}

TEST(OrderBookManagerTest, GetRiskManager) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    RiskManager* risk_mgr = manager.get_risk_manager("AAPL");
    ASSERT_NE(risk_mgr, nullptr);
    
    RiskManager* nonexistent = manager.get_risk_manager("NONEXISTENT");
    EXPECT_EQ(nonexistent, nullptr);
}

TEST(OrderBookManagerTest, GetSymbolInfo) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    info.description = "Apple Inc.";
    info.tick_size = 0.01;
    info.lot_size = 100;
    
    manager.register_symbol("AAPL", info);
    
    SymbolInfo retrieved = manager.get_symbol_info("AAPL");
    EXPECT_EQ(retrieved.symbol, "AAPL");
    EXPECT_EQ(retrieved.description, "Apple Inc.");
    EXPECT_DOUBLE_EQ(retrieved.tick_size, 0.01);
    EXPECT_EQ(retrieved.lot_size, 100);
}

TEST(OrderBookManagerTest, GetSymbolInfoNonexistent) {
    OrderBookManager manager;
    
    SymbolInfo info = manager.get_symbol_info("NONEXISTENT");
    EXPECT_EQ(info.symbol, "");
}

TEST(OrderBookManagerTest, AddOrder) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    ErrorCode err = manager.add_order("AAPL", 1, 100.0, 10, Side::BUY);
    EXPECT_EQ(err, ErrorCode::SUCCESS);
}

TEST(OrderBookManagerTest, AddOrderNonexistentSymbol) {
    OrderBookManager manager;
    
    ErrorCode err = manager.add_order("NONEXISTENT", 1, 100.0, 10, Side::BUY);
    EXPECT_NE(err, ErrorCode::SUCCESS);
}

TEST(OrderBookManagerTest, CancelOrder) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    manager.add_order("AAPL", 1, 100.0, 10, Side::BUY);
    
    ErrorCode err = manager.cancel_order("AAPL", 1);
    EXPECT_EQ(err, ErrorCode::SUCCESS);
}

TEST(OrderBookManagerTest, CancelOrderNonexistentSymbol) {
    OrderBookManager manager;
    
    ErrorCode err = manager.cancel_order("NONEXISTENT", 1);
    EXPECT_NE(err, ErrorCode::SUCCESS);
}

TEST(OrderBookManagerTest, GetBestBid) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    manager.add_order("AAPL", 1, 100.0, 10, Side::BUY);
    
    Price bid = manager.get_best_bid("AAPL");
    EXPECT_DOUBLE_EQ(bid, 100.0);
}

TEST(OrderBookManagerTest, GetBestBidNonexistentSymbol) {
    OrderBookManager manager;
    
    Price bid = manager.get_best_bid("NONEXISTENT");
    EXPECT_DOUBLE_EQ(bid, 0.0);
}

TEST(OrderBookManagerTest, GetBestAsk) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    manager.add_order("AAPL", 1, 100.0, 10, Side::SELL);
    
    Price ask = manager.get_best_ask("AAPL");
    EXPECT_DOUBLE_EQ(ask, 100.0);
}

TEST(OrderBookManagerTest, GetBestAskNonexistentSymbol) {
    OrderBookManager manager;
    
    Price ask = manager.get_best_ask("NONEXISTENT");
    EXPECT_DOUBLE_EQ(ask, 0.0);
}

TEST(OrderBookManagerTest, GetPosition) {
    OrderBookManager manager;
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    Position position = manager.get_position("AAPL");
    EXPECT_EQ(position.long_position, 0);
    EXPECT_EQ(position.short_position, 0);
}

TEST(OrderBookManagerTest, GetSymbols) {
    OrderBookManager manager;
    
    SymbolInfo info1;
    info1.symbol = "AAPL";
    manager.register_symbol("AAPL", info1);
    
    SymbolInfo info2;
    info2.symbol = "GOOGL";
    manager.register_symbol("GOOGL", info2);
    
    std::vector<std::string> symbols = manager.get_symbols();
    EXPECT_EQ(symbols.size(), 2);
    
    std::sort(symbols.begin(), symbols.end());
    EXPECT_EQ(symbols[0], "AAPL");
    EXPECT_EQ(symbols[1], "GOOGL");
}

TEST(OrderBookManagerTest, MultipleSymbolsIndependent) {
    OrderBookManager manager;
    
    SymbolInfo aapl_info;
    aapl_info.symbol = "AAPL";
    manager.register_symbol("AAPL", aapl_info);
    
    SymbolInfo googl_info;
    googl_info.symbol = "GOOGL";
    manager.register_symbol("GOOGL", googl_info);
    
    manager.add_order("AAPL", 1, 100.0, 10, Side::BUY);
    manager.add_order("GOOGL", 2, 150.0, 5, Side::BUY);
    
    EXPECT_DOUBLE_EQ(manager.get_best_bid("AAPL"), 100.0);
    EXPECT_DOUBLE_EQ(manager.get_best_bid("GOOGL"), 150.0);
}

TEST(OrderBookManagerTest, SymbolWithDefaultInfo) {
    OrderBookManager manager;
    
    // Register with default SymbolInfo
    manager.register_symbol("AAPL");
    
    auto info = manager.get_symbol_info("AAPL");
    // SymbolInfo may have default values, just verify it's retrievable
    EXPECT_TRUE(manager.has_symbol("AAPL"));
}

TEST(OrderBookManagerTest, SymbolCount) {
    OrderBookManager manager;
    
    EXPECT_EQ(manager.symbol_count(), 0);
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    EXPECT_EQ(manager.symbol_count(), 1);
    
    info.symbol = "GOOGL";
    manager.register_symbol("GOOGL", info);
    
    EXPECT_EQ(manager.symbol_count(), 2);
    
    manager.unregister_symbol("AAPL");
    
    EXPECT_EQ(manager.symbol_count(), 1);
}

TEST(OrderBookManagerTest, HasSymbol) {
    OrderBookManager manager;
    
    EXPECT_FALSE(manager.has_symbol("AAPL"));
    
    SymbolInfo info;
    info.symbol = "AAPL";
    manager.register_symbol("AAPL", info);
    
    EXPECT_TRUE(manager.has_symbol("AAPL"));
    EXPECT_FALSE(manager.has_symbol("GOOGL"));
}
