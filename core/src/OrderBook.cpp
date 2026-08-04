#include "lob/OrderBook.h"
#include <chrono>
#include <spdlog/spdlog.h>

namespace lob {

OrderBook::OrderBook(TradeCallback callback)
    : trade_callback_(callback)
    , order_counter_(0)
{
    spdlog::info("OrderBook initialized with pool capacity: {}", POOL_CAPACITY);
}

#ifdef HAVE_STD_EXPECTED
Result<void> OrderBook::add_limit_order(OrderID id, Price price, Quantity quantity, Side side) {
#else
ErrorCode OrderBook::add_limit_order(OrderID id, Price price, Quantity quantity, Side side) {
#endif
    // Validate inputs
    if (quantity == 0 || quantity > MAX_QUANTITY) [[unlikely]] {
#ifdef HAVE_STD_EXPECTED
        return make_void_error(ErrorCode::INVALID_QUANTITY);
#else
        return ErrorCode::INVALID_QUANTITY;
#endif
    }
    
    if (price < MIN_PRICE || price > MAX_PRICE) [[unlikely]] {
#ifdef HAVE_STD_EXPECTED
        return make_void_error(ErrorCode::INVALID_PRICE);
#else
        return ErrorCode::INVALID_PRICE;
#endif
    }
    
    if (side != Side::BUY && side != Side::SELL) [[unlikely]] {
#ifdef HAVE_STD_EXPECTED
        return make_void_error(ErrorCode::INVALID_SIDE);
#else
        return ErrorCode::INVALID_SIDE;
#endif
    }
    
    if (order_map_.find(id) != order_map_.end()) [[unlikely]] {
#ifdef HAVE_STD_EXPECTED
        return make_void_error(ErrorCode::INVALID_ORDER_ID); // Duplicate order ID
#else
        return ErrorCode::INVALID_ORDER_ID; // Duplicate order ID
#endif
    }
    
    // Allocate order from memory pool
    Order* order = allocate_order(id, price, quantity, side);
    if (!order) [[unlikely]] {
        spdlog::error("Memory pool exhausted for order ID: {}", id);
#ifdef HAVE_STD_EXPECTED
        return make_void_error(ErrorCode::POOL_EXHAUSTED);
#else
        return ErrorCode::POOL_EXHAUSTED;
#endif
    }
    
    // Match the order
    if (side == Side::BUY) {
        match_buy_order(order);
    } else {
        match_sell_order(order);
    }
    
    // If order still has remaining quantity, add to book
    if (!order->is_filled()) [[likely]] {
        add_to_price_level(order);
        order_map_[id] = order;
    } else {
        // Order was fully matched, deallocate
        deallocate_order(order);
    }
    
#ifdef HAVE_STD_EXPECTED
    return make_void_success();
#else
    return ErrorCode::SUCCESS;
#endif
}

#ifdef HAVE_STD_EXPECTED
Result<void> OrderBook::cancel_order(OrderID id) {
#else
ErrorCode OrderBook::cancel_order(OrderID id) {
#endif
    auto it = order_map_.find(id);
    if (it == order_map_.end()) [[unlikely]] {
#ifdef HAVE_STD_EXPECTED
        return make_void_error(ErrorCode::ORDER_NOT_FOUND);
#else
        return ErrorCode::ORDER_NOT_FOUND;
#endif
    }
    
    Order* order = it->second;
    remove_from_price_level(order);
    order_map_.erase(it);
    deallocate_order(order);
    
    spdlog::debug("Cancelled order ID: {}", id);
#ifdef HAVE_STD_EXPECTED
    return make_void_success();
#else
    return ErrorCode::SUCCESS;
#endif
}

Price OrderBook::get_best_bid() const {
    if (bids_.empty()) [[unlikely]] {
        return 0.0;
    }
    return bids_.begin()->first;
}

Price OrderBook::get_best_ask() const {
    if (asks_.empty()) [[unlikely]] {
        return 0.0;
    }
    return asks_.begin()->first;
}

Quantity OrderBook::get_quantity_at_price(Price price, Side side) const {
    if (side == Side::BUY) {
        auto it = bids_.find(price);
        if (it != bids_.end()) {
            return it->second.total_quantity;
        }
    } else {
        auto it = asks_.find(price);
        if (it != asks_.end()) {
            return it->second.total_quantity;
        }
    }
    return 0;
}

void OrderBook::match_buy_order(Order* order) {
    while (!order->is_filled() && !asks_.empty()) [[likely]] {
        auto best_ask = asks_.begin();
        if (order->price < best_ask->first) [[unlikely]] {
            break; // No matching price
        }
        
        Order* maker = best_ask->second.orders.front();
        Quantity trade_qty = std::min(order->quantity, maker->quantity);
        
        execute_trade(maker, order, trade_qty);
        
        // Update maker order
        maker->quantity -= trade_qty;
        best_ask->second.total_quantity -= trade_qty;
        
        // Remove maker if fully filled
        if (maker->is_filled()) [[unlikely]] {
            best_ask->second.orders.remove(maker);
            order_map_.erase(maker->id);
            deallocate_order(maker);
            
            // Cleanup empty price level
            if (best_ask->second.orders.empty()) [[unlikely]] {
                asks_.erase(best_ask);
            }
        }
        
        // Update taker order
        order->quantity -= trade_qty;
    }
}

void OrderBook::match_sell_order(Order* order) {
    while (!order->is_filled() && !bids_.empty()) [[likely]] {
        auto best_bid = bids_.begin();
        if (order->price > best_bid->first) [[unlikely]] {
            break; // No matching price
        }
        
        Order* maker = best_bid->second.orders.front();
        Quantity trade_qty = std::min(order->quantity, maker->quantity);
        
        execute_trade(maker, order, trade_qty);
        
        // Update maker order
        maker->quantity -= trade_qty;
        best_bid->second.total_quantity -= trade_qty;
        
        // Remove maker if fully filled
        if (maker->is_filled()) [[unlikely]] {
            best_bid->second.orders.remove(maker);
            order_map_.erase(maker->id);
            deallocate_order(maker);
            
            // Cleanup empty price level
            if (best_bid->second.orders.empty()) [[unlikely]] {
                bids_.erase(best_bid);
            }
        }
        
        // Update taker order
        order->quantity -= trade_qty;
    }
}

void OrderBook::execute_trade(Order* maker, Order* taker, Quantity quantity) {
    Trade trade;
    trade.maker_order_id = maker->id;
    trade.taker_order_id = taker->id;
    trade.price = maker->price;
    trade.quantity = quantity;
    trade.timestamp = get_timestamp();
    trade.side = maker->side;
    
    if (trade_callback_) [[likely]] {
        trade_callback_(trade);
    }
    
    // spdlog::debug("Trade executed: {}", trade);
}

void OrderBook::add_to_price_level(Order* order) {
    if (order->side == Side::BUY) {
        auto& level = bids_[order->price];
        if (level.orders.empty()) [[unlikely]] {
            level = PriceLevel(order->price);
        }
        level.add_order(order);
    } else {
        auto& level = asks_[order->price];
        if (level.orders.empty()) [[unlikely]] {
            level = PriceLevel(order->price);
        }
        level.add_order(order);
    }
}

void OrderBook::remove_from_price_level(Order* order) {
    if (order->side == Side::BUY) {
        auto it = bids_.find(order->price);
        if (it != bids_.end()) [[likely]] {
            it->second.remove_order(order);
            if (it->second.orders.empty()) [[unlikely]] {
                bids_.erase(it);
            }
        }
    } else {
        auto it = asks_.find(order->price);
        if (it != asks_.end()) [[likely]] {
            it->second.remove_order(order);
            if (it->second.orders.empty()) [[unlikely]] {
                asks_.erase(it);
            }
        }
    }
}

Order* OrderBook::allocate_order(OrderID id, Price price, Quantity quantity, Side side) {
    return memory_pool_.allocate(id, price, quantity, side, get_timestamp());
}

void OrderBook::deallocate_order(Order* order) {
    memory_pool_.deallocate(order);
}

void OrderBook::cleanup_price_level(Price price, Side side) {
    if (side == Side::BUY) {
        auto it = bids_.find(price);
        if (it != bids_.end() && it->second.orders.empty()) [[unlikely]] {
            bids_.erase(it);
        }
    } else {
        auto it = asks_.find(price);
        if (it != asks_.end() && it->second.orders.empty()) [[unlikely]] {
            asks_.erase(it);
        }
    }
}

Timestamp OrderBook::get_timestamp() const {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()
    ).count();
}

} // namespace lob
