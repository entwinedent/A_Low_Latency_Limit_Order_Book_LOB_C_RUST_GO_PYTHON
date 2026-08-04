#include <gtest/gtest.h>
#include "lob/MemoryPool.h"
#include "lob/Order.h"
#include <sstream>

using namespace lob;

TEST(MemoryPoolTest, BasicAllocation) {
    MemoryPool<Order, 100> pool;
    
    EXPECT_EQ(pool.capacity(), 100);
    EXPECT_EQ(pool.allocated_count(), 0);
    EXPECT_EQ(pool.free_count(), 100);
    EXPECT_TRUE(pool.is_empty());
    EXPECT_FALSE(pool.is_full());
}

TEST(MemoryPoolTest, AllocateAndDeallocate) {
    MemoryPool<Order, 100> pool;
    
    // Allocate an order
    Order* order = pool.allocate(1, 100.0, 10, Side::BUY, 0);
    ASSERT_NE(order, nullptr);
    
    EXPECT_EQ(pool.allocated_count(), 1);
    EXPECT_EQ(pool.free_count(), 99);
    EXPECT_FALSE(pool.is_empty());
    
    // Check order data
    EXPECT_EQ(order->id, 1);
    EXPECT_DOUBLE_EQ(order->price, 100.0);
    EXPECT_EQ(order->quantity, 10);
    EXPECT_EQ(order->side, Side::BUY);
    
    // Deallocate the order
    pool.deallocate(order);
    
    EXPECT_EQ(pool.allocated_count(), 0);
    EXPECT_EQ(pool.free_count(), 100);
    EXPECT_TRUE(pool.is_empty());
}

TEST(MemoryPoolTest, MultipleAllocations) {
    MemoryPool<Order, 100> pool;
    
    std::vector<Order*> orders;
    for (uint64_t i = 0; i < 50; ++i) {
        Order* order = pool.allocate(i, 100.0 + i, 10, Side::BUY, i);
        ASSERT_NE(order, nullptr);
        orders.push_back(order);
    }
    
    EXPECT_EQ(pool.allocated_count(), 50);
    EXPECT_EQ(pool.free_count(), 50);
    
    // Deallocate all orders
    for (auto* order : orders) {
        pool.deallocate(order);
    }
    
    EXPECT_EQ(pool.allocated_count(), 0);
    EXPECT_EQ(pool.free_count(), 100);
}

TEST(MemoryPoolTest, PoolExhaustion) {
    MemoryPool<Order, 10> pool;
    
    std::vector<Order*> orders;
    for (uint64_t i = 0; i < 10; ++i) {
        Order* order = pool.allocate(i, 100.0, 10, Side::BUY, i);
        ASSERT_NE(order, nullptr);
        orders.push_back(order);
    }
    
    EXPECT_TRUE(pool.is_full());
    
    // Try to allocate when pool is full
    Order* order = pool.allocate(999, 100.0, 10, Side::BUY, 0);
    EXPECT_EQ(order, nullptr);
    
    // Clean up
    for (auto* order : orders) {
        pool.deallocate(order);
    }
}

TEST(MemoryPoolTest, ReuseAfterDeallocation) {
    MemoryPool<Order, 10> pool;
    
    // Allocate and deallocate
    Order* order1 = pool.allocate(1, 100.0, 10, Side::BUY, 0);
    ASSERT_NE(order1, nullptr);
    pool.deallocate(order1);
    
    // Allocate again - should reuse the same memory
    Order* order2 = pool.allocate(2, 105.0, 5, Side::SELL, 1);
    ASSERT_NE(order2, nullptr);
    
    // The pointer should be the same (LIFO free list)
    EXPECT_EQ(order1, order2);
    
    // But the data should be different
    EXPECT_EQ(order2->id, 2);
    EXPECT_DOUBLE_EQ(order2->price, 105.0);
    EXPECT_EQ(order2->quantity, 5);
    EXPECT_EQ(order2->side, Side::SELL);
    
    pool.deallocate(order2);
}

TEST(MemoryPoolTest, StringStreamIntegration) {
    // Test in-memory string stream for logging (zero disk I/O)
    MemoryPool<Order, 100> pool;
    std::stringstream ss;
    
    Order* order = pool.allocate(1, 100.0, 10, Side::BUY, 0);
    ASSERT_NE(order, nullptr);
    
    // Log to stringstream instead of file
    ss << "Allocated order ID: " << order->id 
       << ", Price: " << order->price
       << ", Quantity: " << order->quantity;
    
    std::string log_output = ss.str();
    EXPECT_FALSE(log_output.empty());
    EXPECT_NE(log_output.find("Allocated order ID"), std::string::npos);
    
    pool.deallocate(order);
}

TEST(MemoryPoolTest, LargePool) {
    // Test with a larger pool size
    MemoryPool<Order, 10000> pool;
    
    EXPECT_EQ(pool.capacity(), 10000);
    
    // Allocate many orders
    std::vector<Order*> orders;
    for (uint64_t i = 0; i < 1000; ++i) {
        Order* order = pool.allocate(i, 100.0, 10, Side::BUY, i);
        ASSERT_NE(order, nullptr);
        orders.push_back(order);
    }
    
    EXPECT_EQ(pool.allocated_count(), 1000);
    
    // Clean up
    for (auto* order : orders) {
        pool.deallocate(order);
    }
    
    EXPECT_EQ(pool.allocated_count(), 0);
}
