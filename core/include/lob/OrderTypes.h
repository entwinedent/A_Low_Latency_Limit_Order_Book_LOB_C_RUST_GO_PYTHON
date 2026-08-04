#pragma once

#include "Common.h"
#include "Order.h"
#include <cstdint>
#include <functional>

namespace lob {

// Order type enumeration
enum class OrderType : uint8_t {
    LIMIT = 0,           // Standard limit order
    MARKET = 1,          // Market order
    STOP_LOSS = 2,       // Stop-loss order
    TAKE_PROFIT = 3,     // Take-profit order
    STOP_LIMIT = 4,      // Stop-limit order
    ICEBERG = 5,         // Iceberg (hidden quantity) order
    TRAILING_STOP = 6,   // Trailing stop order
    FOK = 7,             // Fill-or-kill
    AON = 8              // All-or-none
};

// Time in force enumeration
enum class TimeInForce : uint8_t {
    GTC = 0,  // Good till cancel
    IOC = 1,  // Immediate or cancel
    FOK = 2,  // Fill or kill
    DAY = 3   // Day order
};

// Extended order structure with advanced types
struct ExtendedOrder {
    OrderID id;
    Price price;
    Quantity quantity;
    Side side;
    Timestamp timestamp;
    Order* next = nullptr;
    Order* prev = nullptr;
    
    OrderType order_type = OrderType::LIMIT;
    TimeInForce time_in_force = TimeInForce::GTC;
    Price stop_price = 0;           // For stop orders (fixed-point)
    Price take_profit_price = 0;    // For take-profit orders (fixed-point)
    Quantity display_quantity = 0;     // For iceberg orders
    Quantity hidden_quantity = 0;      // For iceberg orders
    Quantity trail_amount = 0;         // For trailing stop
    Price activation_price = 0;      // Current activation price for trailing stop (fixed-point)
    bool is_active = true;             // For conditional orders
    
    ExtendedOrder() = default;
    
    ExtendedOrder(OrderID order_id, Price order_price, Quantity order_quantity, Side order_side,
                   OrderType type = OrderType::LIMIT,
                   TimeInForce tif = TimeInForce::GTC)
        : id(order_id)
        , price(order_price)
        , quantity(order_quantity)
        , side(order_side)
        , timestamp(0)
        , order_type(type)
        , time_in_force(tif)
    {
        if (type == OrderType::ICEBERG) {
            display_quantity = order_quantity / 2;  // Show half, hide half
            hidden_quantity = order_quantity - display_quantity;
        }
    }
    
    // Check if order is a conditional order type
    bool is_conditional() const {
        return order_type == OrderType::STOP_LOSS ||
               order_type == OrderType::TAKE_PROFIT ||
               order_type == OrderType::STOP_LIMIT ||
               order_type == OrderType::TRAILING_STOP;
    }
    
    // Check if order should be activated
    bool should_activate(Price current_price, Side side) const {
        if (!is_conditional() || !is_active) return false;
        
        switch (order_type) {
            case OrderType::STOP_LOSS:
            case OrderType::STOP_LIMIT:
                // For buy: activate when price >= stop_price
                // For sell: activate when price <= stop_price
                return (side == Side::BUY && current_price >= stop_price) ||
                       (side == Side::SELL && current_price <= stop_price);
            
            case OrderType::TAKE_PROFIT:
                // For buy: activate when price <= take_profit_price
                // For sell: activate when price >= take_profit_price
                return (side == Side::BUY && current_price <= take_profit_price) ||
                       (side == Side::SELL && current_price >= take_profit_price);
            
            case OrderType::TRAILING_STOP:
                // Update activation price based on trail amount
                if (side == Side::BUY) {
                    // For buy: trail below market
                    Price new_activation = current_price - trail_amount;
                    if (new_activation > activation_price) {
                        const_cast<Price&>(activation_price) = new_activation;
                    }
                    return current_price <= activation_price;
                } else {
                    // For sell: trail above market
                    Price new_activation = current_price + trail_amount;
                    if (new_activation < activation_price || activation_price == 0.0) {
                        const_cast<Price&>(activation_price) = new_activation;
                    }
                    return current_price >= activation_price;
                }
            
            default:
                return false;
        }
    }
};

// Order validation callback type
using OrderValidationCallback = std::function<bool(const ExtendedOrder&)>;

// Risk management callback type
using RiskCheckCallback = std::function<bool(const ExtendedOrder&, Price, Quantity)>;

} // namespace lob
