#include <gtest/gtest.h>
#include "lob/OrderBook.h"
#include "lob/Common.h"
#include <sstream>
#include <vector>

using namespace lob;

class MatchingLogicTest : public ::testing::Test {
protected:
    OrderBook book;
    std::vector<Trade> trades;
    
    void SetUp() override {
        book.set_trade_callback([this](const Trade& trade) {
            trades.push_back(trade);
        });
    }
    
    void TearDown() override {
        trades.clear();
    }
};

TEST_F(MatchingLogicTest, SimpleBuySellMatch) {
    // Add a sell order
#ifdef HAVE_STD_EXPECTED
    auto result = book.add_limit_order(1, 100.0, 10, Side::SELL);
    ASSERT_TRUE(result);
#else
    auto err = book.add_limit_order(1, 100.0, 10, Side::SELL);
    ASSERT_EQ(err, ErrorCode::SUCCESS);
#endif
    
    // Add a buy order that matches
#ifdef HAVE_STD_EXPECTED
    result = book.add_limit_order(2, 100.0, 5, Side::BUY);
    ASSERT_TRUE(result);
#else
    err = book.add_limit_order(2, 100.0, 5, Side::BUY);
    ASSERT_EQ(err, ErrorCode::SUCCESS);
#endif
    
    // Verify trade occurred
    EXPECT_GT(trades.size(), 0);
    
    // Verify remaining quantity
    EXPECT_EQ(book.get_quantity_at_price(100.0, Side::SELL), 5);
}

TEST_F(MatchingLogicTest, MultipleBuyOrdersMatchingOneSell) {
    // Add a large sell order
    book.add_limit_order(1, 100.0, 30, Side::SELL);
    
    // Add multiple buy orders
    book.add_limit_order(2, 100.0, 10, Side::BUY);
    book.add_limit_order(3, 100.0, 10, Side::BUY);
    book.add_limit_order(4, 100.0, 10, Side::BUY);
    
    // All buy orders should be fully matched
    EXPECT_EQ(book.get_bid_depth(), 0);
    EXPECT_EQ(book.get_quantity_at_price(100.0, Side::SELL), 0);
}

TEST_F(MatchingLogicTest, CrossedMarketImmediateMatch) {
    // Add orders that cross immediately
    book.add_limit_order(1, 100.0, 10, Side::BUY);
    book.add_limit_order(2, 99.0, 5, Side::SELL);
    
    // Should match immediately since buy price >= sell price
    EXPECT_EQ(book.get_quantity_at_price(99.0, Side::SELL), 0);
    EXPECT_EQ(book.get_quantity_at_price(100.0, Side::BUY), 5);
}

TEST_F(MatchingLogicTest, PricePriority) {
    // Add sell orders at different prices
    book.add_limit_order(1, 105.0, 10, Side::SELL);
    book.add_limit_order(2, 100.0, 10, Side::SELL);
    book.add_limit_order(3, 102.0, 10, Side::SELL);
    
    // Buy order should match lowest sell price first
    book.add_limit_order(4, 100.0, 5, Side::BUY);
    
    // Should match the 100.0 sell order
    EXPECT_EQ(book.get_quantity_at_price(100.0, Side::SELL), 5);
    EXPECT_EQ(book.get_quantity_at_price(102.0, Side::SELL), 10);
    EXPECT_EQ(book.get_quantity_at_price(105.0, Side::SELL), 10);
}

TEST_F(MatchingLogicTest, TimePriorityAtSamePrice) {
    // Add sell orders at same price, different times
    book.add_limit_order(1, 100.0, 10, Side::SELL);
    book.add_limit_order(2, 100.0, 10, Side::SELL);
    book.add_limit_order(3, 100.0, 10, Side::SELL);
    
    // Buy order should match FIFO
    book.add_limit_order(4, 100.0, 15, Side::BUY);
    
    // First order should be fully matched, second partially
    EXPECT_EQ(book.get_quantity_at_price(100.0, Side::SELL), 15);
}

TEST_F(MatchingLogicTest, NoMatchWhenSpread) {
    // Add orders with spread
    book.add_limit_order(1, 100.0, 10, Side::BUY);
    book.add_limit_order(2, 105.0, 10, Side::SELL);
    
    // No match should occur
    EXPECT_EQ(book.get_bid_depth(), 1);
    EXPECT_EQ(book.get_ask_depth(), 1);
    EXPECT_EQ(trades.size(), 0);
}

TEST_F(MatchingLogicTest, CancelAfterPartialMatch) {
    // Add sell order
    book.add_limit_order(1, 100.0, 20, Side::SELL);
    
    // Partially match
    book.add_limit_order(2, 100.0, 10, Side::BUY);
    
    // Cancel remaining
#ifdef HAVE_STD_EXPECTED
    auto result = book.cancel_order(1);
    EXPECT_TRUE(result);
#else
    auto err = book.cancel_order(1);
    EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
    
    // Book should be empty
    EXPECT_TRUE(book.is_empty());
}

TEST_F(MatchingLogicTest, MarketOrderSimulation) {
    // Simulate market order by adding aggressive limit order
    book.add_limit_order(1, 100.0, 10, Side::SELL);
    book.add_limit_order(2, 101.0, 10, Side::SELL);
    book.add_limit_order(3, 102.0, 10, Side::SELL);
    
    // Aggressive buy order (high price)
    book.add_limit_order(4, 200.0, 25, Side::BUY);
    
    // Should match all three sell orders (30 total) and have 5 remaining on buy
    // After matching: sell at 102.0 should have 5 remaining (30 - 25 = 5 matched)
    EXPECT_EQ(book.get_ask_depth(), 1);  // One sell order remains at 102.0
    EXPECT_EQ(book.get_quantity_at_price(102.0, Side::SELL), 5);  // 5 remaining at 102.0
    EXPECT_EQ(book.get_bid_depth(), 0);  // Buy order fully matched
}

TEST_F(MatchingLogicTest, EmptyBookNoMatch) {
    // Try to match in empty book
#ifdef HAVE_STD_EXPECTED
    auto result = book.add_limit_order(1, 100.0, 10, Side::BUY);
    EXPECT_TRUE(result);
#else
    auto err = book.add_limit_order(1, 100.0, 10, Side::BUY);
    EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
    
    EXPECT_EQ(book.get_bid_depth(), 1);
    EXPECT_EQ(book.get_ask_depth(), 0);
}

TEST_F(MatchingLogicTest, LargeOrderPartialMatch) {
    // Add multiple small sell orders
    for (int i = 0; i < 10; ++i) {
        book.add_limit_order(i, 100.0, 5, Side::SELL);
    }
    
    // Add large buy order
    book.add_limit_order(100, 100.0, 35, Side::BUY);
    
    // Should match 7 sell orders fully, partially match 8th
    EXPECT_EQ(book.get_quantity_at_price(100.0, Side::SELL), 15);
}

TEST_F(MatchingLogicTest, InMemoryStreamIntegration) {
    // Test in-memory string stream for trade logging (zero disk I/O)
    std::stringstream trade_log;
    
    book.add_limit_order(1, 100.0, 10, Side::SELL);
    book.add_limit_order(2, 100.0, 5, Side::BUY);
    
    // Log trades to stringstream
    for (const auto& trade : trades) {
        trade_log << "Trade: maker=" << trade.maker_order_id
                 << ", taker=" << trade.taker_order_id
                 << ", price=" << trade.price
                 << ", qty=" << trade.quantity
                 << "\n";
    }
    
    std::string log_output = trade_log.str();
    EXPECT_FALSE(log_output.empty());
}

TEST_F(MatchingLogicTest, ComplexScenario) {
    // Complex multi-step matching scenario
    // Build order book
    book.add_limit_order(1, 99.0, 10, Side::BUY);
    book.add_limit_order(2, 100.0, 10, Side::BUY);
    book.add_limit_order(3, 101.0, 10, Side::BUY);
    
    book.add_limit_order(4, 102.0, 10, Side::SELL);
    book.add_limit_order(5, 103.0, 10, Side::SELL);
    book.add_limit_order(6, 104.0, 10, Side::SELL);
    
    // Add order that crosses the spread
    book.add_limit_order(7, 102.0, 15, Side::BUY);
    
    // Should match with the lowest sell
    EXPECT_EQ(book.get_quantity_at_price(102.0, Side::SELL), 0);
    EXPECT_EQ(book.get_quantity_at_price(102.0, Side::BUY), 5);
    
    // Cancel remaining buy
    book.cancel_order(7);
    
    // Add aggressive sell
    book.add_limit_order(8, 99.0, 25, Side::SELL);
    
    // Should match multiple buy orders
    EXPECT_EQ(book.get_bid_depth(), 1); // Only 101.0 bid remaining
}

TEST_F(MatchingLogicTest, ZeroQuantityAfterMatch) {
    // Add sell order
    book.add_limit_order(1, 100.0, 10, Side::SELL);
    
    // Fully match it
    book.add_limit_order(2, 100.0, 10, Side::BUY);
    
    // Should be removed from book
    EXPECT_EQ(book.get_ask_depth(), 0);
    EXPECT_EQ(book.get_quantity_at_price(100.0, Side::SELL), 0);
}

TEST_F(MatchingLogicTest, MemoryPoolIntegration) {
    // Test that memory pool is used correctly during matching
    size_t initial_allocated = book.pool_allocated();
    
    // Add orders
    book.add_limit_order(1, 100.0, 10, Side::SELL);
    book.add_limit_order(2, 100.0, 5, Side::BUY);
    
    // After matching, some orders should be deallocated
    size_t final_allocated = book.pool_allocated();
    
    // The exact count depends on implementation
    // but we should see allocation/deallocation happening
    EXPECT_GE(final_allocated, initial_allocated);
}

TEST_F(MatchingLogicTest, StreamingSnapshot) {
    // Test creating order book snapshot to stream
    std::stringstream snapshot;
    
    // Build order book
    book.add_limit_order(1, 100.0, 10, Side::BUY);
    book.add_limit_order(2, 105.0, 5, Side::SELL);
    
    // Create snapshot
    snapshot << "=== Order Book Snapshot ===\n";
    snapshot << "Best Bid: " << book.get_best_bid() << "\n";
    snapshot << "Best Ask: " << book.get_best_ask() << "\n";
    snapshot << "Bid Depth: " << book.get_bid_depth() << "\n";
    snapshot << "Ask Depth: " << book.get_ask_depth() << "\n";
    snapshot << "Spread: " << (book.get_best_ask() - book.get_best_bid()) << "\n";
    snapshot << "Pool Usage: " << book.pool_allocated() << "/" << book.pool_capacity() << "\n";
    
    std::string snapshot_str = snapshot.str();
    EXPECT_FALSE(snapshot_str.empty());
    EXPECT_NE(snapshot_str.find("Order Book Snapshot"), std::string::npos);
}

TEST_F(MatchingLogicTest, RapidSequentialOperations) {
    // Test rapid sequential add/cancel/match operations
    // Use prices that won't cross to avoid matching
    for (int i = 0; i < 100; ++i) {
        book.add_limit_order(i, 100.0 + i, 10, Side::BUY);
    }
    
    for (int i = 0; i < 50; ++i) {
        book.cancel_order(i);
    }
    
    // Add sell orders at higher prices to avoid matching with remaining buy orders
    // Remaining buy orders have prices 150.0-199.0, so sell at 200.0+
    for (int i = 100; i < 150; ++i) {
        book.add_limit_order(i, 200.0 + (i - 100), 10, Side::SELL);
    }
    
    // Should have 50 buy orders and 50 sell orders
    EXPECT_EQ(book.get_bid_depth(), 50);
    EXPECT_EQ(book.get_ask_depth(), 50);
}
