#include <gtest/gtest.h>
#include "lob/IntrusiveList.h"
#include "lob/Order.h"
#include <sstream>

using namespace lob;

TEST(IntrusiveListTest, BasicOperations) {
    IntrusiveList list;
    
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
    EXPECT_EQ(list.front(), nullptr);
    EXPECT_EQ(list.back(), nullptr);
}

TEST(IntrusiveListTest, PushBack) {
    IntrusiveList list;
    
    Order order1(1, 100.0, 10, Side::BUY, 0);
    Order order2(2, 105.0, 5, Side::SELL, 1);
    
    list.push_back(&order1);
    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.front(), &order1);
    EXPECT_EQ(list.back(), &order1);
    
    list.push_back(&order2);
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), &order1);
    EXPECT_EQ(list.back(), &order2);
}

TEST(IntrusiveListTest, Remove) {
    IntrusiveList list;
    
    Order order1(1, 100.0, 10, Side::BUY, 0);
    Order order2(2, 105.0, 5, Side::SELL, 1);
    Order order3(3, 110.0, 15, Side::BUY, 2);
    
    list.push_back(&order1);
    list.push_back(&order2);
    list.push_back(&order3);
    
    EXPECT_EQ(list.size(), 3);
    
    // Remove middle element
    list.remove(&order2);
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), &order1);
    EXPECT_EQ(list.back(), &order3);
    
    // Remove first element
    list.remove(&order1);
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.front(), &order3);
    EXPECT_EQ(list.back(), &order3);
    
    // Remove last element
    list.remove(&order3);
    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.empty());
}

TEST(IntrusiveListTest, RemoveNonExistent) {
    IntrusiveList list;
    
    Order order1(1, 100.0, 10, Side::BUY, 0);
    Order order2(2, 105.0, 5, Side::SELL, 1);
    
    list.push_back(&order1);
    
    // Try to remove order not in list
    list.remove(&order2);
    EXPECT_EQ(list.size(), 1);
    
    // Remove null pointer (should be safe)
    list.remove(nullptr);
    EXPECT_EQ(list.size(), 1);
}

TEST(IntrusiveListTest, Clear) {
    IntrusiveList list;
    
    Order order1(1, 100.0, 10, Side::BUY, 0);
    Order order2(2, 105.0, 5, Side::SELL, 1);
    
    list.push_back(&order1);
    list.push_back(&order2);
    
    EXPECT_EQ(list.size(), 2);
    
    list.clear();
    
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
    EXPECT_EQ(list.front(), nullptr);
    EXPECT_EQ(list.back(), nullptr);
}

TEST(IntrusiveListTest, Iterator) {
    IntrusiveList list;
    
    Order order1(1, 100.0, 10, Side::BUY, 0);
    Order order2(2, 105.0, 5, Side::SELL, 1);
    Order order3(3, 110.0, 15, Side::BUY, 2);
    
    list.push_back(&order1);
    list.push_back(&order2);
    list.push_back(&order3);
    
    int count = 0;
    for (auto it = list.begin(); it != list.end(); ++it) {
        count++;
    }
    
    EXPECT_EQ(count, 3);
}

TEST(IntrusiveListTest, IteratorTraversal) {
    IntrusiveList list;
    
    Order order1(1, 100.0, 10, Side::BUY, 0);
    Order order2(2, 105.0, 5, Side::SELL, 1);
    Order order3(3, 110.0, 15, Side::BUY, 2);
    
    list.push_back(&order1);
    list.push_back(&order2);
    list.push_back(&order3);
    
    auto it = list.begin();
    EXPECT_EQ(*it, &order1);
    ++it;
    EXPECT_EQ(*it, &order2);
    ++it;
    EXPECT_EQ(*it, &order3);
    ++it;
    EXPECT_EQ(it, list.end());
}

TEST(IntrusiveListTest, FIFOOrdering) {
    IntrusiveList list;
    
    Order order1(1, 100.0, 10, Side::BUY, 0);
    Order order2(2, 105.0, 5, Side::SELL, 1);
    Order order3(3, 110.0, 15, Side::BUY, 2);
    
    list.push_back(&order1);
    list.push_back(&order2);
    list.push_back(&order3);
    
    // FIFO ordering: first in, first out
    EXPECT_EQ(list.front(), &order1);
    list.remove(&order1);
    EXPECT_EQ(list.front(), &order2);
    list.remove(&order2);
    EXPECT_EQ(list.front(), &order3);
}

TEST(IntrusiveListTest, StringStreamLogging) {
    // Test in-memory string stream for logging (zero disk I/O)
    IntrusiveList list;
    std::stringstream ss;
    
    Order order1(1, 100.0, 10, Side::BUY, 0);
    Order order2(2, 105.0, 5, Side::SELL, 1);
    
    list.push_back(&order1);
    list.push_back(&order2);
    
    // Log list state to stringstream
    ss << "List size: " << list.size();
    ss << ", Front order ID: " << list.front()->id;
    ss << ", Back order ID: " << list.back()->id;
    
    std::string log_output = ss.str();
    EXPECT_FALSE(log_output.empty());
    EXPECT_NE(log_output.find("List size: 2"), std::string::npos);
}

TEST(IntrusiveListTest, LargeList) {
    IntrusiveList list;
    
    std::vector<Order> orders;
    orders.reserve(1000);  // Reserve space to avoid reallocation
    for (uint64_t i = 0; i < 1000; ++i) {
        orders.emplace_back(i, 100.0 + i, 10, Side::BUY, i);
        list.push_back(&orders.back());
    }
    
    EXPECT_EQ(list.size(), 1000);
    
    // Count via iterator
    int count = 0;
    for (auto it = list.begin(); it != list.end(); ++it) {
        count++;
    }
    EXPECT_EQ(count, 1000);
}
