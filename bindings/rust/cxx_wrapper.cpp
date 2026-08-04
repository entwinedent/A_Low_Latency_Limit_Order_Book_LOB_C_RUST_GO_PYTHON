#include "cxx_wrapper.h"
#include "../../core/include/lob/CAPI.h"

namespace cxx_bridge {
    
    std::unique_ptr<OrderBookHandle> create_order_book() {
        auto handle = std::make_unique<OrderBookHandle>();
        handle->ptr = ::create_order_book();
        return handle;
    }
    
    int add_limit_order(OrderBookHandle& handle, uint64_t order_id, double price, uint32_t quantity, int side) {
        return ::add_limit_order(handle.ptr, order_id, price, quantity, side);
    }
    
    int cancel_order(OrderBookHandle& handle, uint64_t order_id) {
        return ::cancel_order(handle.ptr, order_id);
    }
    
    double get_best_bid(const OrderBookHandle& handle) {
        return ::get_best_bid(handle.ptr);
    }
    
    double get_best_ask(const OrderBookHandle& handle) {
        return ::get_best_ask(handle.ptr);
    }
    
    uint32_t get_quantity_at_price(const OrderBookHandle& handle, double price, int side) {
        return ::get_quantity_at_price(handle.ptr, price, side);
    }
    
    size_t get_bid_depth(const OrderBookHandle& handle) {
        return ::get_bid_depth(handle.ptr);
    }
    
    size_t get_ask_depth(const OrderBookHandle& handle) {
        return ::get_ask_depth(handle.ptr);
    }
    
    bool is_empty(const OrderBookHandle& handle) {
        return ::is_empty(handle.ptr) != 0;
    }
    
    size_t pool_capacity(const OrderBookHandle& handle) {
        return ::pool_capacity(handle.ptr);
    }
    
    size_t pool_allocated(const OrderBookHandle& handle) {
        return ::pool_allocated(handle.ptr);
    }
    
    size_t pool_free(const OrderBookHandle& handle) {
        return ::pool_free(handle.ptr);
    }
}


