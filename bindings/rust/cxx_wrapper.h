#pragma once

#include <cstdint>
#include <memory>

// C++ wrapper functions for cxx bridge
// These will be called by the cxx-generated glue code

namespace cxx_bridge {
    // Opaque type for OrderBook handle
    struct OrderBookHandle {
        void* ptr;
    };
    
    // Create a new order book and return as opaque pointer
    std::unique_ptr<OrderBookHandle> create_order_book();
    
    // Add a limit order
    int add_limit_order(OrderBookHandle& handle, uint64_t id, double price, uint32_t quantity, int side);
    
    // Cancel an order
    int cancel_order(OrderBookHandle& handle, uint64_t id);
    
    // Get best bid
    double get_best_bid(const OrderBookHandle& handle);
    
    // Get best ask
    double get_best_ask(const OrderBookHandle& handle);
    
    // Get quantity at price
    uint32_t get_quantity_at_price(const OrderBookHandle& handle, double price, int side);
    
    // Get bid depth
    size_t get_bid_depth(const OrderBookHandle& handle);
    
    // Get ask depth
    size_t get_ask_depth(const OrderBookHandle& handle);
    
    // Check if empty
    bool is_empty(const OrderBookHandle& handle);
    
    // Pool statistics
    size_t pool_capacity(const OrderBookHandle& handle);
    size_t pool_allocated(const OrderBookHandle& handle);
    size_t pool_free(const OrderBookHandle& handle);
}




