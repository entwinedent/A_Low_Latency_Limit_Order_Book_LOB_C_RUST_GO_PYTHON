#pragma once

#include "Common.h"
#include "OrderTypes.h"
#include <unordered_map>
#include <cstdint>
#include <mutex>
#include <chrono>

namespace lob {

// Position tracking per symbol
struct Position {
    Quantity long_position = 0;
    Quantity short_position = 0;
    Price avg_long_price = 0;  // Fixed-point
    Price avg_short_price = 0;  // Fixed-point
    
    Quantity net_position() const {
        return long_position - short_position;
    }
    
    Quantity gross_position() const {
        return long_position + short_position;
    }
    
    bool is_long() const {
        return long_position > short_position;
    }
    
    bool is_short() const {
        return short_position > long_position;
    }
};

// Risk limits configuration
struct RiskLimits {
    Quantity max_position_per_symbol = 1000000;      // Max net position per symbol
    Quantity max_order_size = 100000;                 // Max single order size
    Quantity max_daily_volume = 10000000;             // Max daily trading volume
    Price max_price_deviation = 1000;                 // Max price deviation from last trade (10% in fixed-point)
    uint64_t max_orders_per_second = 10000;           // Rate limit
    bool enable_self_trade_prevention = true;          // Prevent self-trading
    bool enable_circuit_breaker = true;                // Enable circuit breaker
    Price circuit_breaker_threshold = 2000;            // 20% price move triggers halt (fixed-point)
};

// Risk manager for order validation and position tracking
class RiskManager {
public:
    explicit RiskManager(const RiskLimits& limits = RiskLimits())
        : limits_(limits)
        , daily_volume_(0)
        , orders_this_second_(0)
        , last_second_(0)
        , circuit_breaker_triggered_(false)
        , last_trade_price_(0)
    {}
    
    // Check if order passes risk checks
    bool check_order(const ExtendedOrder& order, const char* symbol = nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Check circuit breaker
        if (circuit_breaker_triggered_) {
            return false;
        }
        
        // Check order size
        if (order.quantity > limits_.max_order_size) {
            return false;
        }
        
        // Check price deviation (fixed-point arithmetic)
        if (last_trade_price_ > 0) {
            Price deviation = (order.price > last_trade_price_) ? 
                (order.price - last_trade_price_) : (last_trade_price_ - order.price);
            // Convert to percentage: (deviation * 10000) / last_trade_price
            Price deviation_percent = (deviation * PRICE_SCALE_FACTOR) / last_trade_price_;
            if (deviation_percent > limits_.max_price_deviation) {
                return false;
            }
        }
        
        // Check rate limit
        uint64_t current_second = get_timestamp_ms() / 1000;
        if (current_second != last_second_) {
            orders_this_second_ = 0;
            last_second_ = current_second;
        }
        if (orders_this_second_ >= limits_.max_orders_per_second) {
            return false;
        }
        orders_this_second_++;
        
        // Check position limits if symbol provided
        if (symbol) {
            auto it = positions_.find(symbol);
            if (it != positions_.end()) {
                const Position& pos = it->second;
                
                // Calculate new position after order
                Quantity new_long = pos.long_position;
                Quantity new_short = pos.short_position;
                
                if (order.side == Side::BUY) {
                    new_long += order.quantity;
                } else {
                    new_short += order.quantity;
                }
                
                Quantity new_net = new_long - new_short;
                if (std::abs(static_cast<int64_t>(new_net)) > 
                    static_cast<int64_t>(limits_.max_position_per_symbol)) {
                    return false;
                }
            }
        }
        
        // Check daily volume
        if (daily_volume_ + order.quantity > limits_.max_daily_volume) {
            return false;
        }
        
        return true;
    }
    
    // Update position after trade execution
    void update_position(const char* symbol, Side side, Quantity quantity, Price price) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        Position& pos = positions_[symbol];
        
        if (side == Side::BUY) {
            // Update average long price
            if (pos.long_position > 0) {
                pos.avg_long_price = (pos.avg_long_price * pos.long_position + price * quantity) / 
                                     (pos.long_position + quantity);
            } else {
                pos.avg_long_price = price;
            }
            pos.long_position += quantity;
        } else {
            // Update average short price
            if (pos.short_position > 0) {
                pos.avg_short_price = (pos.avg_short_price * pos.short_position + price * quantity) / 
                                      (pos.short_position + quantity);
            } else {
                pos.avg_short_price = price;
            }
            pos.short_position += quantity;
        }
        
        daily_volume_ += quantity;
        last_trade_price_ = price;
        
        // Check circuit breaker
        check_circuit_breaker(price);
    }
    
    // Check for self-trade prevention
    bool check_self_trade(const char* symbol, Side side, uint64_t account_id) {
        if (!limits_.enable_self_trade_prevention) {
            return true;
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Check if account has opposing position
        auto it = positions_.find(symbol);
        if (it != positions_.end()) {
            const Position& pos = it->second;
            
            // If buying and have short position, potential self-trade
            if (side == Side::BUY && pos.short_position > 0) {
                return false;
            }
            
            // If selling and have long position, potential self-trade
            if (side == Side::SELL && pos.long_position > 0) {
                return false;
            }
        }
        
        return true;
    }
    
    // Get position for symbol
    Position get_position(const char* symbol) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = positions_.find(symbol);
        if (it != positions_.end()) {
            return it->second;
        }
        return Position{};
    }
    
    // Reset daily volume (called at start of day)
    void reset_daily_volume() {
        std::lock_guard<std::mutex> lock(mutex_);
        daily_volume_ = 0;
    }
    
    // Reset circuit breaker
    void reset_circuit_breaker() {
        std::lock_guard<std::mutex> lock(mutex_);
        circuit_breaker_triggered_ = false;
    }
    
    // Check if circuit breaker is triggered
    bool is_circuit_breaker_triggered() const {
        return circuit_breaker_triggered_;
    }
    
    // Get risk limits
    const RiskLimits& get_limits() const { return limits_; }
    
    // Set risk limits
    void set_limits(const RiskLimits& limits) { limits_ = limits; }
    
private:
    void check_circuit_breaker(Price current_price) {
        if (!limits_.enable_circuit_breaker || last_trade_price_ == 0.0) {
            return;
        }
        
        double deviation = std::abs(current_price - last_trade_price_) / last_trade_price_;
        if (deviation >= limits_.circuit_breaker_threshold) {
            circuit_breaker_triggered_ = true;
        }
    }
    
    static uint64_t get_timestamp_ms() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    }
    
    RiskLimits limits_;
    std::unordered_map<std::string, Position> positions_;
    Quantity daily_volume_;
    uint64_t orders_this_second_;
    uint64_t last_second_;
    bool circuit_breaker_triggered_;
    Price last_trade_price_;
    mutable std::mutex mutex_;
};

} // namespace lob
