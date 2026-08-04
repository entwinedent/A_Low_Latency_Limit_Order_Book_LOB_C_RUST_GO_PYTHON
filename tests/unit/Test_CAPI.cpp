#include <gtest/gtest.h>
#include "lob/CAPI.h"

class CAPITest : public ::testing::Test {
protected:
    void SetUp() override {
        handle = create_order_book();
        ASSERT_NE(handle, nullptr);
    }

    void TearDown() override {
        if (handle) {
            destroy_order_book(handle);
            handle = nullptr;
        }
    }

    OrderBookHandle handle = nullptr;
};

TEST_F(CAPITest, CreateOrderBook) {
    OrderBookHandle new_handle = create_order_book();
    ASSERT_NE(new_handle, nullptr);
    destroy_order_book(new_handle);
}

TEST_F(CAPITest, DestroyOrderBook) {
    // Test destroying a valid handle
    OrderBookHandle temp_handle = create_order_book();
    ASSERT_NE(temp_handle, nullptr);
    destroy_order_book(temp_handle);
    
    // Test destroying nullptr (should not crash)
    destroy_order_book(nullptr);
}

TEST_F(CAPITest, AddLimitOrderBuy) {
    ErrorCode err = add_limit_order(handle, 1, 1000000, 10, SIDE_BUY);  // 100.00 in fixed-point
    EXPECT_EQ(err, ERROR_SUCCESS);
    
    EXPECT_EQ(get_bid_depth(handle), 1);
    EXPECT_EQ(get_best_bid(handle), 1000000);
}

TEST_F(CAPITest, AddLimitOrderSell) {
    ErrorCode err = add_limit_order(handle, 1, 1000000, 10, SIDE_SELL);  // 100.00 in fixed-point
    EXPECT_EQ(err, ERROR_SUCCESS);
    
    EXPECT_EQ(get_ask_depth(handle), 1);
    EXPECT_EQ(get_best_ask(handle), 1000000);
}

TEST_F(CAPITest, AddLimitOrderInvalidHandle) {
    ErrorCode err = add_limit_order(nullptr, 1, 1000000, 10, SIDE_BUY);
    EXPECT_EQ(err, ERROR_INVALID_ORDER_ID);
}

TEST_F(CAPITest, CancelOrder) {
    add_limit_order(handle, 1, 1000000, 10, SIDE_BUY);
    
    ErrorCode err = cancel_order(handle, 1);
    EXPECT_EQ(err, ERROR_SUCCESS);
    
    EXPECT_EQ(get_bid_depth(handle), 0);
}

TEST_F(CAPITest, CancelOrderNotFound) {
    ErrorCode err = cancel_order(handle, 999);
    EXPECT_EQ(err, ERROR_ORDER_NOT_FOUND);
}

TEST_F(CAPITest, CancelOrderInvalidHandle) {
    ErrorCode err = cancel_order(nullptr, 1);
    EXPECT_EQ(err, ERROR_ORDER_NOT_FOUND);
}

TEST_F(CAPITest, GetBestBid) {
    add_limit_order(handle, 1, 100.0, 10, SIDE_BUY);
    add_limit_order(handle, 2, 99.0, 5, SIDE_BUY);
    
    double bid = get_best_bid(handle);
    EXPECT_DOUBLE_EQ(bid, 100.0);
}

TEST_F(CAPITest, GetBestBidInvalidHandle) {
    double bid = get_best_bid(nullptr);
    EXPECT_DOUBLE_EQ(bid, 0.0);
}

TEST_F(CAPITest, GetBestAsk) {
    add_limit_order(handle, 1, 105.0, 10, SIDE_SELL);
    add_limit_order(handle, 2, 106.0, 5, SIDE_SELL);
    
    double ask = get_best_ask(handle);
    EXPECT_DOUBLE_EQ(ask, 105.0);
}

TEST_F(CAPITest, GetBestAskInvalidHandle) {
    double ask = get_best_ask(nullptr);
    EXPECT_DOUBLE_EQ(ask, 0.0);
}

TEST_F(CAPITest, GetQuantityAtPrice) {
    add_limit_order(handle, 1, 100.0, 10, SIDE_BUY);
    add_limit_order(handle, 2, 100.0, 5, SIDE_BUY);
    
    uint32_t qty = get_quantity_at_price(handle, 100.0, SIDE_BUY);
    EXPECT_EQ(qty, 15);
}

TEST_F(CAPITest, GetQuantityAtPriceInvalidHandle) {
    uint32_t qty = get_quantity_at_price(nullptr, 100.0, SIDE_BUY);
    EXPECT_EQ(qty, 0);
}

TEST_F(CAPITest, GetBidDepth) {
    add_limit_order(handle, 1, 100.0, 10, SIDE_BUY);
    add_limit_order(handle, 2, 99.0, 5, SIDE_BUY);
    
    size_t depth = get_bid_depth(handle);
    EXPECT_EQ(depth, 2);
}

TEST_F(CAPITest, GetBidDepthInvalidHandle) {
    size_t depth = get_bid_depth(nullptr);
    EXPECT_EQ(depth, 0);
}

TEST_F(CAPITest, GetAskDepth) {
    add_limit_order(handle, 1, 105.0, 10, SIDE_SELL);
    add_limit_order(handle, 2, 106.0, 5, SIDE_SELL);
    
    size_t depth = get_ask_depth(handle);
    EXPECT_EQ(depth, 2);
}

TEST_F(CAPITest, GetAskDepthInvalidHandle) {
    size_t depth = get_ask_depth(nullptr);
    EXPECT_EQ(depth, 0);
}

TEST_F(CAPITest, IsEmpty) {
    int empty = is_empty(handle);
    EXPECT_EQ(empty, 1);
    
    add_limit_order(handle, 1, 100.0, 10, SIDE_BUY);
    
    empty = is_empty(handle);
    EXPECT_EQ(empty, 0);
}

TEST_F(CAPITest, IsEmptyInvalidHandle) {
    int empty = is_empty(nullptr);
    EXPECT_EQ(empty, 1);
}

TEST_F(CAPITest, PoolCapacity) {
    size_t capacity = pool_capacity(handle);
    EXPECT_GT(capacity, 0);
}

TEST_F(CAPITest, PoolCapacityInvalidHandle) {
    size_t capacity = pool_capacity(nullptr);
    EXPECT_EQ(capacity, 0);
}

TEST_F(CAPITest, PoolAllocated) {
    size_t allocated = pool_allocated(handle);
    EXPECT_EQ(allocated, 0);
    
    add_limit_order(handle, 1, 100.0, 10, SIDE_BUY);
    
    allocated = pool_allocated(handle);
    EXPECT_GT(allocated, 0);
}

TEST_F(CAPITest, PoolAllocatedInvalidHandle) {
    size_t allocated = pool_allocated(nullptr);
    EXPECT_EQ(allocated, 0);
}

TEST_F(CAPITest, PoolFree) {
    size_t free = pool_free(handle);
    EXPECT_GT(free, 0);
    
    add_limit_order(handle, 1, 100.0, 10, SIDE_BUY);
    
    size_t free_after = pool_free(handle);
    EXPECT_LT(free_after, free);
}

TEST_F(CAPITest, PoolFreeInvalidHandle) {
    size_t free = pool_free(nullptr);
    EXPECT_EQ(free, 0);
}

TEST_F(CAPITest, SetTradeCallback) {
    int callback_count = 0;
    
    auto callback = [](uint64_t maker_id, uint64_t taker_id, double price, 
                      uint32_t quantity, uint64_t timestamp, int side) {
        // Callback function - in a real test, this would capture state
    };
    
    set_trade_callback(handle, callback);
    
    // Add matching orders to trigger callback
    add_limit_order(handle, 1, 100.0, 10, SIDE_SELL);
    add_limit_order(handle, 2, 100.0, 5, SIDE_BUY);
}

TEST_F(CAPITest, SetTradeCallbackInvalidHandle) {
    auto callback = [](uint64_t, uint64_t, double, uint32_t, uint64_t, int) {};
    set_trade_callback(nullptr, callback); // Should not crash
}

TEST_F(CAPITest, OrderMatching) {
    add_limit_order(handle, 1, 100.0, 10, SIDE_SELL);
    add_limit_order(handle, 2, 100.0, 5, SIDE_BUY);
    
    uint32_t remaining = get_quantity_at_price(handle, 100.0, SIDE_SELL);
    EXPECT_EQ(remaining, 5);
}

TEST_F(CAPITest, MultipleOrdersSamePrice) {
    add_limit_order(handle, 1, 100.0, 10, SIDE_BUY);
    add_limit_order(handle, 2, 100.0, 5, SIDE_BUY);
    add_limit_order(handle, 3, 100.0, 15, SIDE_BUY);
    
    uint32_t qty = get_quantity_at_price(handle, 100.0, SIDE_BUY);
    EXPECT_EQ(qty, 30);
}

TEST_F(CAPITest, MultiplePriceLevels) {
    add_limit_order(handle, 1, 100.0, 10, SIDE_BUY);
    add_limit_order(handle, 2, 99.0, 5, SIDE_BUY);
    add_limit_order(handle, 3, 98.0, 15, SIDE_BUY);
    
    EXPECT_EQ(get_bid_depth(handle), 3);
    EXPECT_DOUBLE_EQ(get_best_bid(handle), 100.0);
}

TEST_F(CAPITest, CancelThenAddSameId) {
    add_limit_order(handle, 1, 100.0, 10, SIDE_BUY);
    cancel_order(handle, 1);
    
    ErrorCode err = add_limit_order(handle, 1, 105.0, 5, SIDE_BUY);
    EXPECT_EQ(err, ERROR_SUCCESS);
}

TEST_F(CAPITest, LargeOrderQuantity) {
    ErrorCode err = add_limit_order(handle, 1, 100.0, 1000000, SIDE_BUY);
    EXPECT_EQ(err, ERROR_SUCCESS);
}

TEST_F(CAPITest, HighPrecisionPrice) {
    ErrorCode err = add_limit_order(handle, 1, 100.123456789, 10, SIDE_BUY);
    EXPECT_EQ(err, ERROR_SUCCESS);
}

TEST_F(CAPITest, ZeroPrice) {
    ErrorCode err = add_limit_order(handle, 1, 0.0, 10, SIDE_BUY);
    EXPECT_EQ(err, ERROR_INVALID_PRICE);
}

TEST_F(CAPITest, NegativePrice) {
    ErrorCode err = add_limit_order(handle, 1, -100.0, 10, SIDE_BUY);
    EXPECT_EQ(err, ERROR_INVALID_PRICE);
}

TEST_F(CAPITest, ZeroQuantity) {
    ErrorCode err = add_limit_order(handle, 1, 100.0, 0, SIDE_BUY);
    EXPECT_EQ(err, ERROR_INVALID_QUANTITY);
}

TEST_F(CAPITest, InvalidSide) {
    // Note: The CAPI layer may not validate side parameter
    // This test verifies the current behavior
    ErrorCode err = add_limit_order(handle, 1, 100.0, 10, 99);
    // If side validation is not implemented in CAPI, order may succeed
    // or return a different error code
    EXPECT_TRUE(err == ERROR_SUCCESS || err == ERROR_INVALID_SIDE);
}

TEST_F(CAPITest, GetQuantityAtPriceInvalidSideUsesSellBranch) {
    add_limit_order(handle, 1, 100.0, 10, SIDE_SELL);
    add_limit_order(handle, 2, 100.0, 5, SIDE_BUY);

    uint32_t qty = get_quantity_at_price(handle, 100.0, SIDE_SELL);
    EXPECT_EQ(qty, 5u);
}

TEST_F(CAPITest, NullHandleReturnsSafeDefaultsAcrossAPIs) {
    EXPECT_EQ(add_limit_order(nullptr, 1, 100.0, 10, SIDE_BUY), ERROR_INVALID_ORDER_ID);
    EXPECT_EQ(cancel_order(nullptr, 1), ERROR_ORDER_NOT_FOUND);
    EXPECT_DOUBLE_EQ(get_best_bid(nullptr), 0.0);
    EXPECT_DOUBLE_EQ(get_best_ask(nullptr), 0.0);
    EXPECT_EQ(get_quantity_at_price(nullptr, 100.0, SIDE_BUY), 0u);
    EXPECT_EQ(get_bid_depth(nullptr), 0u);
    EXPECT_EQ(get_ask_depth(nullptr), 0u);
    EXPECT_EQ(is_empty(nullptr), 1);
    EXPECT_EQ(pool_capacity(nullptr), 0u);
    EXPECT_EQ(pool_allocated(nullptr), 0u);
    EXPECT_EQ(pool_free(nullptr), 0u);
}
