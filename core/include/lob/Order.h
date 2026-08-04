#pragma once

#include "Common.h"
#include <cstdint>

namespace lob {

// Order structure with cache line alignment and intrusive list pointers
// alignas(64) ensures the structure is aligned to cache line boundaries
// to prevent false sharing in multi-threaded scenarios
struct alignas(64) Order {
    OrderID id;
    Price price;
    Quantity quantity;
    Side side;
    Timestamp timestamp;
    
    // Intrusive list pointers for O(1) insertion/removal
    Order* next = nullptr;
    Order* prev = nullptr;
    
    // Constructor
    Order(OrderID id_, Price price_, Quantity quantity_, Side side_, Timestamp timestamp_)
        : id(id_)
        , price(price_)
        , quantity(quantity_)
        , side(side_)
        , timestamp(timestamp_)
    {}
    
    // Check if order is fully filled
    bool is_filled() const {
        return quantity == 0;
    }
    
    // Get remaining quantity
    Quantity get_remaining_quantity() const {
        return quantity;
    }
};

} // namespace lob
