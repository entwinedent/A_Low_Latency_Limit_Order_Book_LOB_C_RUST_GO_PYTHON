#pragma once

#include "OrderBook.h"
#include "RiskManager.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <shared_mutex>

namespace lob {

// Symbol information
struct SymbolInfo {
    std::string symbol;
    std::string description;
    Price tick_size = 100;  // 0.01 in fixed-point
    Quantity lot_size = 100;
    bool is_active = true;
};

// Multi-symbol order book manager
class OrderBookManager {
public:
    OrderBookManager() = default;
    
    // Register a new symbol
    bool register_symbol(const std::string& symbol, const SymbolInfo& info = SymbolInfo{}) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        if (order_books_.find(symbol) != order_books_.end()) {
            return false; // Symbol already registered
        }
        
        symbol_info_[symbol] = info;
        order_books_[symbol] = std::make_unique<OrderBook>();
        risk_managers_[symbol] = std::make_unique<RiskManager>();
        
        return true;
    }
    
    // Unregister a symbol
    bool unregister_symbol(const std::string& symbol) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        if (order_books_.find(symbol) == order_books_.end()) {
            return false;
        }
        
        order_books_.erase(symbol);
        risk_managers_.erase(symbol);
        symbol_info_.erase(symbol);
        
        return true;
    }
    
    // Get order book for symbol (read-only access)
    OrderBook* get_order_book(const std::string& symbol) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto it = order_books_.find(symbol);
        if (it != order_books_.end()) {
            return it->second.get();
        }
        return nullptr;
    }
    
    // Get risk manager for symbol
    RiskManager* get_risk_manager(const std::string& symbol) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto it = risk_managers_.find(symbol);
        if (it != risk_managers_.end()) {
            return it->second.get();
        }
        return nullptr;
    }
    
    // Get symbol info
    SymbolInfo get_symbol_info(const std::string& symbol) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto it = symbol_info_.find(symbol);
        if (it != symbol_info_.end()) {
            return it->second;
        }
        return SymbolInfo{};
    }
    
    // Add order to specific symbol
    ErrorCode add_order(const std::string& symbol, OrderID id, Price price, 
                       Quantity quantity, Side side) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto book = get_order_book_unlocked(symbol);
        auto risk_mgr = get_risk_manager_unlocked(symbol);
        
        if (!book || !risk_mgr) {
            return ErrorCode::INVALID_ORDER_ID; // Symbol not found
        }
        
        // Check risk limits
        ExtendedOrder order(id, price, quantity, side);
        if (!risk_mgr->check_order(order, symbol.c_str())) {
            return ErrorCode::POOL_EXHAUSTED; // Risk check failed
        }
        
#ifdef HAVE_STD_EXPECTED
        auto result = book->add_limit_order(id, price, quantity, side);
        if (!result) {
            return static_cast<ErrorCode>(result.error());
        }
        return ErrorCode::SUCCESS;
#else
        return book->add_limit_order(id, price, quantity, side);
#endif
    }
    
    // Cancel order from specific symbol
    ErrorCode cancel_order(const std::string& symbol, OrderID id) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto book = get_order_book_unlocked(symbol);
        if (!book) {
            return ErrorCode::ORDER_NOT_FOUND;
        }
        
#ifdef HAVE_STD_EXPECTED
        auto result = book->cancel_order(id);
        if (!result) {
            return static_cast<ErrorCode>(result.error());
        }
        return ErrorCode::SUCCESS;
#else
        return book->cancel_order(id);
#endif
    }
    
    // Get best bid for symbol
    Price get_best_bid(const std::string& symbol) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto book = get_order_book_unlocked(symbol);
        if (!book) {
            return 0.0;
        }
        
        return book->get_best_bid();
    }
    
    // Get best ask for symbol
    Price get_best_ask(const std::string& symbol) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto book = get_order_book_unlocked(symbol);
        if (!book) {
            return 0.0;
        }
        
        return book->get_best_ask();
    }
    
    // Get position for symbol
    Position get_position(const std::string& symbol) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto risk_mgr = get_risk_manager_unlocked(symbol);
        if (!risk_mgr) {
            return Position{};
        }
        
        return risk_mgr->get_position(symbol.c_str());
    }
    
    // Get all registered symbols
    std::vector<std::string> get_symbols() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        std::vector<std::string> symbols;
        symbols.reserve(symbol_info_.size());
        
        for (const auto& [symbol, _] : symbol_info_) {
            symbols.push_back(symbol);
        }
        
        return symbols;
    }
    
    // Get number of registered symbols
    size_t symbol_count() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return order_books_.size();
    }
    
    // Check if symbol exists
    bool has_symbol(const std::string& symbol) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return order_books_.find(symbol) != order_books_.end();
    }
    
private:
    OrderBook* get_order_book_unlocked(const std::string& symbol) {
        auto it = order_books_.find(symbol);
        return it != order_books_.end() ? it->second.get() : nullptr;
    }
    
    RiskManager* get_risk_manager_unlocked(const std::string& symbol) {
        auto it = risk_managers_.find(symbol);
        return it != risk_managers_.end() ? it->second.get() : nullptr;
    }
    
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> order_books_;
    std::unordered_map<std::string, std::unique_ptr<RiskManager>> risk_managers_;
    std::unordered_map<std::string, SymbolInfo> symbol_info_;
};

} // namespace lob
