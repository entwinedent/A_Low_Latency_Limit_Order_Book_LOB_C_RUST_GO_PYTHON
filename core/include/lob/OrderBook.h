#pragma once

#include "Common.h"
#include "Order.h"
#include "MemoryPool.h"
#include "IntrusiveList.h"
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>

namespace lob {

// Price level structure: maintains a list of orders at a specific price
struct PriceLevel {
    Price price;
    IntrusiveList orders;
    Quantity total_quantity;
    
    PriceLevel() : price(0), total_quantity(0) {}
    explicit PriceLevel(Price p) : price(p), total_quantity(0) {}
    
    void add_order(Order* order) {
        orders.push_back(order);
        total_quantity += order->quantity;
    }
    
    void remove_order(Order* order) {
        orders.remove(order);
        total_quantity -= order->quantity;
    }
};

// Core order book implementation
class OrderBook {
public:
    // Memory pool capacity - adjust based on expected load
    static constexpr size_t POOL_CAPACITY = 1000000;
    
    using TradeCallback = std::function<void(const Trade&)>;
    
    explicit OrderBook(TradeCallback callback = nullptr);
    ~OrderBook() = default;
    
    // Add a limit order
    #ifdef HAVE_STD_EXPECTED
    Result<void> add_limit_order(OrderID id, Price price, Quantity quantity, Side side);
    #else
    ErrorCode add_limit_order(OrderID id, Price price, Quantity quantity, Side side);
    #endif
    
    // Cancel an existing order
    #ifdef HAVE_STD_EXPECTED
    Result<void> cancel_order(OrderID id);
    #else
    ErrorCode cancel_order(OrderID id);
    #endif
    
    // Get the best bid (highest buy price)
    Price get_best_bid() const;
    
    // Get the best ask (lowest sell price)
    Price get_best_ask() const;
    
    // Get total quantity at a specific price level
    Quantity get_quantity_at_price(Price price, Side side) const;
    
    // Get order book depth
    size_t get_bid_depth() const { return bids_.size(); }
    size_t get_ask_depth() const { return asks_.size(); }
    
    // Check if order book is empty
    bool is_empty() const { return bids_.empty() && asks_.empty(); }
    
    // Set trade callback
    void set_trade_callback(TradeCallback callback) { trade_callback_ = callback; }
    
    // Get memory pool statistics
    size_t pool_capacity() const { return memory_pool_.capacity(); }
    size_t pool_allocated() const { return memory_pool_.allocated_count(); }
    size_t pool_free() const { return memory_pool_.free_count(); }
    
private:
    // Match a buy order against existing sell orders
    void match_buy_order(Order* order);
    
    // Match a sell order against existing buy orders
    void match_sell_order(Order* order);
    
    // Execute a trade
    void execute_trade(Order* maker, Order* taker, Quantity quantity);
    
    // Add order to appropriate price level
    void add_to_price_level(Order* order);
    
    // Remove order from price level
    void remove_from_price_level(Order* order);
    
    // Allocate order from memory pool
    Order* allocate_order(OrderID id, Price price, Quantity quantity, Side side);
    
    // Deallocate order back to memory pool
    void deallocate_order(Order* order);
    
    // Remove empty price level
    void cleanup_price_level(Price price, Side side);
    
    // Get current timestamp
    Timestamp get_timestamp() const;
    
    // Data structures
    MemoryPool<Order, POOL_CAPACITY> memory_pool_;
    
    // Price levels: bids (descending), asks (ascending)
    std::map<Price, PriceLevel, std::greater<Price>> bids_;
    std::map<Price, PriceLevel, std::less<Price>> asks_;
    
    // Order lookup map for O(1) cancellation
    std::unordered_map<OrderID, Order*> order_map_;
    
    // Trade callback
    TradeCallback trade_callback_;
    
    // Order ID counter for timestamp simulation
    mutable OrderID order_counter_;
};

} // namespace lob
