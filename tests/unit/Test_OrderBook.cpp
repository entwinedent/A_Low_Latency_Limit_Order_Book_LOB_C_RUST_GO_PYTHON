#include <gtest/gtest.h>
#include "lob/OrderBook.h"
#include "lob/Common.h"
#include <sstream>
#include <vector>

using namespace lob;

class OrderBookTest : public ::testing::Test {
protected:
    OrderBook book;
    
    void SetUp() override {
        // Each test gets a fresh order book
    }
    
    void TearDown() override {
        // Cleanup handled by destructor
    }
};

TEST_F(OrderBookTest, InitialState) {
    EXPECT_TRUE(book.is_empty());
    EXPECT_EQ(book.get_best_bid(), 0);
    EXPECT_EQ(book.get_best_ask(), 0);
    EXPECT_EQ(book.get_bid_depth(), 0);
    EXPECT_EQ(book.get_ask_depth(), 0u);
}

TEST_F(OrderBookTest, AddBuyOrder) {
#ifdef HAVE_STD_EXPECTED
    auto result = book.add_limit_order(1, 1000000, 10, Side::BUY);  // 100.00 in fixed-point
    EXPECT_TRUE(result);
#else
    auto err = book.add_limit_order(1, 1000000, 10, Side::BUY);  // 100.00 in fixed-point
    EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
    
    EXPECT_FALSE(book.is_empty());
    EXPECT_EQ(book.get_bid_depth(), 1);
    EXPECT_EQ(book.get_best_bid(), 1000000);
}

TEST_F(OrderBookTest, AddSellOrder) {
#ifdef HAVE_STD_EXPECTED
    auto result = book.add_limit_order(1, 1000000, 10, Side::SELL);  // 100.00 in fixed-point
    EXPECT_TRUE(result);
#else
    auto err = book.add_limit_order(1, 1000000, 10, Side::SELL);  // 100.00 in fixed-point
    EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
    
    EXPECT_FALSE(book.is_empty());
    EXPECT_EQ(book.get_ask_depth(), 1);
    EXPECT_EQ(book.get_best_ask(), 1000000);
}

TEST_F(OrderBookTest, CancelOrder) {
#ifdef HAVE_STD_EXPECTED
    book.add_limit_order(1, 1000000, 10, Side::BUY);  // 100.00 in fixed-point
    auto result = book.cancel_order(1);
    EXPECT_TRUE(result);
#else
    book.add_limit_order(1, 1000000, 10, Side::BUY);  // 100.00 in fixed-point
    auto err = book.cancel_order(1);
    EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
    
    EXPECT_TRUE(book.is_empty());
}

TEST_F(OrderBookTest, CancelNonExistentOrder) {
#ifdef HAVE_STD_EXPECTED
    auto result = book.cancel_order(999);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), ErrorCode::ORDER_NOT_FOUND);
#else
    auto err = book.cancel_order(999);
    EXPECT_EQ(err, ErrorCode::ORDER_NOT_FOUND);
#endif
}

TEST_F(OrderBookTest, InvalidOrderQuantity) {
#ifdef HAVE_STD_EXPECTED
    auto result = book.add_limit_order(1, 1000000, 0, Side::BUY);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), ErrorCode::INVALID_QUANTITY);
#else
    auto err = book.add_limit_order(1, 1000000, 0, Side::BUY);
    EXPECT_EQ(err, ErrorCode::INVALID_QUANTITY);
#endif
}

TEST_F(OrderBookTest, InvalidOrderPrice) {
#ifdef HAVE_STD_EXPECTED
    auto result = book.add_limit_order(1, -1, 10, Side::BUY);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), ErrorCode::INVALID_PRICE);
#else
    auto err = book.add_limit_order(1, -1, 10, Side::BUY);
    EXPECT_EQ(err, ErrorCode::INVALID_PRICE);
#endif
}

TEST_F(OrderBookTest, InvalidOrderSide) {
#ifdef HAVE_STD_EXPECTED
    auto result = book.add_limit_order(1, 1000000, 10, static_cast<Side>(99));
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), ErrorCode::INVALID_SIDE);
#else
    auto err = book.add_limit_order(1, 1000000, 10, static_cast<Side>(99));
    EXPECT_EQ(err, ErrorCode::INVALID_SIDE);
#endif
}

TEST_F(OrderBookTest, DuplicateOrderId) {
#ifdef HAVE_STD_EXPECTED
    book.add_limit_order(1, 1000000, 10, Side::BUY);
    auto result = book.add_limit_order(1, 1050000, 5, Side::BUY);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), ErrorCode::INVALID_ORDER_ID);
#else
    book.add_limit_order(1, 1000000, 10, Side::BUY);
    auto err = book.add_limit_order(1, 1050000, 5, Side::BUY);
    EXPECT_EQ(err, ErrorCode::INVALID_ORDER_ID);
#endif
}

TEST_F(OrderBookTest, BestBidPrice) {
    book.add_limit_order(1, 1000000, 10, Side::BUY);
    book.add_limit_order(2, 990000, 5, Side::BUY);
    book.add_limit_order(3, 1010000, 15, Side::BUY);
    
    EXPECT_EQ(book.get_best_bid(), 1010000);
}

TEST_F(OrderBookTest, BestAskPrice) {
    book.add_limit_order(1, 1000000, 10, Side::SELL);
    book.add_limit_order(2, 990000, 5, Side::SELL);
    book.add_limit_order(3, 1010000, 15, Side::SELL);
    
    EXPECT_EQ(book.get_best_ask(), 990000);
}

TEST_F(OrderBookTest, OrderMatching) {
    // Add a sell order
#ifdef HAVE_STD_EXPECTED
    book.add_limit_order(1, 1000000, 10, Side::SELL);
    // Add a buy order that matches
    auto result = book.add_limit_order(2, 1000000, 5, Side::BUY);
    EXPECT_TRUE(result);
#else
    book.add_limit_order(1, 1000000, 10, Side::SELL);
    // Add a buy order that matches
    auto err = book.add_limit_order(2, 1000000, 5, Side::BUY);
    EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
    
    // The sell order should have 5 remaining
    EXPECT_EQ(book.get_quantity_at_price(1000000, Side::SELL), 5);
}

TEST_F(OrderBookTest, FullOrderMatch) {
    // Add a sell order
#ifdef HAVE_STD_EXPECTED
    book.add_limit_order(1, 1000000, 10, Side::SELL);
    // Add a buy order that fully matches
    auto result = book.add_limit_order(2, 1000000, 10, Side::BUY);
    EXPECT_TRUE(result);
#else
    book.add_limit_order(1, 1000000, 10, Side::SELL);
    // Add a buy order that fully matches
    auto err = book.add_limit_order(2, 1000000, 10, Side::BUY);
    EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
    
    // The sell order should be fully matched (removed)
    EXPECT_EQ(book.get_ask_depth(), 0);
}

TEST_F(OrderBookTest, NoMatchWhenPriceCrossing) {
    // Add a sell order at 105
#ifdef HAVE_STD_EXPECTED
    book.add_limit_order(1, 1050000, 10, Side::SELL);
    // Add a buy order at 100 (should not match)
    auto result = book.add_limit_order(2, 1000000, 10, Side::BUY);
    EXPECT_TRUE(result);
#else
    book.add_limit_order(1, 1050000, 10, Side::SELL);
    // Add a buy order at 100 (should not match)
    auto err = book.add_limit_order(2, 1000000, 10, Side::BUY);
    EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
    
    // Both orders should remain in the book
    EXPECT_EQ(book.get_bid_depth(), 1);
    EXPECT_EQ(book.get_ask_depth(), 1);
    EXPECT_EQ(book.get_best_bid(), 1000000);
    EXPECT_EQ(book.get_best_ask(), 1050000);
}

TEST_F(OrderBookTest, MultipleMatches) {
    // Add multiple sell orders
#ifdef HAVE_STD_EXPECTED
    book.add_limit_order(1, 1000000, 5, Side::SELL);
    book.add_limit_order(2, 1000000, 5, Side::SELL);
    book.add_limit_order(3, 1000000, 5, Side::SELL);
    // Add a buy order that matches multiple
    auto result = book.add_limit_order(4, 1000000, 12, Side::BUY);
    EXPECT_TRUE(result);
#else
    book.add_limit_order(1, 1000000, 5, Side::SELL);
    book.add_limit_order(2, 1000000, 5, Side::SELL);
    book.add_limit_order(3, 1000000, 5, Side::SELL);
    // Add a buy order that matches multiple
    auto err = book.add_limit_order(4, 1000000, 12, Side::BUY);
    EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
    
    // First two sell orders should be fully matched
    // Third sell order should have 3 remaining
    EXPECT_EQ(book.get_quantity_at_price(1000000, Side::SELL), 3);
}

TEST_F(OrderBookTest, PriceTimePriority) {
    // Add orders at same price, different times
#ifdef HAVE_STD_EXPECTED
    book.add_limit_order(1, 1000000, 10, Side::SELL);
    book.add_limit_order(2, 1000000, 10, Side::SELL);
    book.add_limit_order(3, 1000000, 10, Side::SELL);
    // Buy order should match FIFO (first order first)
    auto result = book.add_limit_order(4, 1000000, 15, Side::BUY);
    EXPECT_TRUE(result);
#else
    book.add_limit_order(1, 1000000, 10, Side::SELL);
    book.add_limit_order(2, 1000000, 10, Side::SELL);
    book.add_limit_order(3, 1000000, 10, Side::SELL);
    // Buy order should match FIFO (first order first)
    auto err = book.add_limit_order(4, 1000000, 15, Side::BUY);
    EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
    
    // First order should be fully matched
    // Second order should have 5 remaining
    EXPECT_EQ(book.get_quantity_at_price(1000000, Side::SELL), 15);
}

TEST_F(OrderBookTest, MemoryPoolStatistics) {
    EXPECT_EQ(book.pool_capacity(), OrderBook::POOL_CAPACITY);
    EXPECT_EQ(book.pool_allocated(), 0);
    EXPECT_EQ(book.pool_free(), OrderBook::POOL_CAPACITY);
    
    // Add some orders
    for (uint64_t i = 1; i <= 10; ++i) {
#ifdef HAVE_STD_EXPECTED
        book.add_limit_order(i, 1000000 + (i * 100), 10, Side::BUY);
#else
        book.add_limit_order(i, 1000000 + (i * 100), 10, Side::BUY);
#endif
    }
    
    EXPECT_EQ(book.pool_allocated(), 10);
    
    // Cancel some orders
    for (uint64_t i = 1; i <= 5; ++i) {
#ifdef HAVE_STD_EXPECTED
        book.cancel_order(i);
#else
        book.cancel_order(i);
#endif
    }
    
    EXPECT_EQ(book.pool_allocated(), 5);
}

TEST_F(OrderBookTest, TradeCallback) {
    std::vector<Trade> trades;
    
    book.set_trade_callback([&trades](const Trade& trade) {
        trades.push_back(trade);
    });
    
    // Add matching orders
#ifdef HAVE_STD_EXPECTED
    book.add_limit_order(1, 1000000, 10, Side::SELL);
    book.add_limit_order(2, 1000000, 5, Side::BUY);
#else
    book.add_limit_order(1, 1000000, 10, Side::SELL);
    book.add_limit_order(2, 1000000, 5, Side::BUY);
#endif
    
    // Check if callback was invoked
    // Note: This depends on the C++ implementation
    // The actual test would verify the trade data
}

TEST_F(OrderBookTest, StringStreamSnapshot) {
    // Test in-memory string stream for logging (zero disk I/O)
    std::stringstream ss;
    
#ifdef HAVE_STD_EXPECTED
    book.add_limit_order(1, 1000000, 10, Side::BUY);
    book.add_limit_order(2, 1050000, 5, Side::SELL);
#else
    book.add_limit_order(1, 1000000, 10, Side::BUY);
    book.add_limit_order(2, 1050000, 5, Side::SELL);
#endif
    
    // Create snapshot to stringstream
    ss << "Order Book Snapshot:\n";
    ss << "  Best Bid: " << book.get_best_bid() << "\n";
    ss << "  Best Ask: " << book.get_best_ask() << "\n";
    ss << "  Bid Depth: " << book.get_bid_depth() << "\n";
    ss << "  Ask Depth: " << book.get_ask_depth() << "\n";
    ss << "  Pool Allocated: " << book.pool_allocated() << "\n";
    
    std::string snapshot = ss.str();
    EXPECT_FALSE(snapshot.empty());
    EXPECT_NE(snapshot.find("Order Book Snapshot"), std::string::npos);
    EXPECT_NE(snapshot.find("Best Bid: 1000000"), std::string::npos);
}

TEST_F(OrderBookTest, HighVolumeOrders) {
    // Test with high volume of orders
    const int num_orders = 1000;
    
    for (int i = 0; i < num_orders; ++i) {
#ifdef HAVE_STD_EXPECTED
        auto result = book.add_limit_order(i, 1000000 + (i * 100), 10, Side::BUY);
        EXPECT_TRUE(result);
#else
        auto err = book.add_limit_order(i, 1000000 + (i * 100), 10, Side::BUY);
        EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
    }
    
    EXPECT_EQ(book.get_bid_depth(), num_orders);
    EXPECT_EQ(book.pool_allocated(), num_orders);
}

TEST_F(OrderBookTest, SpreadCalculation) {
#ifdef HAVE_STD_EXPECTED
    book.add_limit_order(1, 1000000, 10, Side::BUY);
    book.add_limit_order(2, 1050000, 10, Side::SELL);
#else
    book.add_limit_order(1, 1000000, 10, Side::BUY);
    book.add_limit_order(2, 1050000, 10, Side::SELL);
#endif
    
    Price spread = book.get_best_ask() - book.get_best_bid();
    EXPECT_EQ(spread, 50000);  // 5.00 in fixed-point
}

TEST_F(OrderBookTest, TradeCallbackIsInvokedForMatches) {
    std::vector<Trade> trades;
    book.set_trade_callback([&trades](const Trade& trade) {
        trades.push_back(trade);
    });

#ifdef HAVE_STD_EXPECTED
    book.add_limit_order(1, 1000000, 10, Side::SELL);
    book.add_limit_order(2, 1000000, 5, Side::BUY);
#else
    book.add_limit_order(1, 1000000, 10, Side::SELL);
    book.add_limit_order(2, 1000000, 5, Side::BUY);
#endif

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].maker_order_id, 1u);
    EXPECT_EQ(trades[0].taker_order_id, 2u);
    EXPECT_EQ(trades[0].quantity, 5u);
    EXPECT_EQ(trades[0].side, Side::SELL);
}

TEST_F(OrderBookTest, EmptyPriceLevelsAreRemovedAfterCancellation) {
#ifdef HAVE_STD_EXPECTED
    book.add_limit_order(1, 1000000, 10, Side::BUY);
    book.add_limit_order(2, 1050000, 10, Side::SELL);
    book.cancel_order(1);
    book.cancel_order(2);
#else
    book.add_limit_order(1, 1000000, 10, Side::BUY);
    book.add_limit_order(2, 1050000, 10, Side::SELL);
    book.cancel_order(1);
    book.cancel_order(2);
#endif

    EXPECT_TRUE(book.is_empty());
    EXPECT_EQ(book.get_bid_depth(), 0u);
    EXPECT_EQ(book.get_ask_depth(), 0u);
}

TEST_F(OrderBookTest, PoolExhaustionAndRecyclePath) {
    OrderBook empty_book;
    for (uint64_t i = 1; i <= OrderBook::POOL_CAPACITY; ++i) {
        const double price = 100.0 + (static_cast<double>(i % 1000) * 0.01);
#ifdef HAVE_STD_EXPECTED
        auto result = empty_book.add_limit_order(i, price, 10, Side::BUY);
        EXPECT_TRUE(result);
#else
        auto err = empty_book.add_limit_order(i, price, 10, Side::BUY);
        EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
    }

#ifdef HAVE_STD_EXPECTED
    auto result = empty_book.add_limit_order(OrderBook::POOL_CAPACITY + 1, 200.0, 10, Side::BUY);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), ErrorCode::POOL_EXHAUSTED);
#else
    auto err = empty_book.add_limit_order(OrderBook::POOL_CAPACITY + 1, 200.0, 10, Side::BUY);
    EXPECT_EQ(err, ErrorCode::POOL_EXHAUSTED);
#endif

    EXPECT_EQ(empty_book.pool_allocated(), OrderBook::POOL_CAPACITY);
    EXPECT_EQ(empty_book.pool_free(), 0u);

#ifdef HAVE_STD_EXPECTED
    empty_book.cancel_order(1);
#else
    empty_book.cancel_order(1);
#endif
    EXPECT_EQ(empty_book.pool_allocated(), OrderBook::POOL_CAPACITY - 1u);
}

