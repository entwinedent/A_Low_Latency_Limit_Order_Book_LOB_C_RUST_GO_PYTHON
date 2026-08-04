#include "lob/CAPI.h"
#include "lob/OrderBook.h"
#include "lob/Common.h"
#include <stdexcept>
#include <spdlog/spdlog.h>

// C++ wrapper for trade callback
class CTradeCallback {
public:
    explicit CTradeCallback(TradeCallback callback) : callback_(callback) {}
    
    void operator()(const lob::Trade& trade) {
        if (callback_) [[likely]] {
            // Convert fixed-point price to double for C API
            double price_double = lob::price_to_double(trade.price);
            callback_(
                trade.maker_order_id,
                trade.taker_order_id,
                price_double,
                trade.quantity,
                trade.timestamp,
                static_cast<int>(trade.side)
            );
        }
    }
    
private:
    TradeCallback callback_;
};

extern "C" {

OrderBookHandle create_order_book() {
    try {
        auto* book = new lob::OrderBook();
        spdlog::info("Created order book instance");
        return static_cast<OrderBookHandle>(book);
    } catch (const std::exception& e) {
        spdlog::error("Failed to create order book: {}", e.what());
        return nullptr;
    } catch (...) {
        spdlog::error("Unknown exception in create_order_book");
        return nullptr;
    }
}

void destroy_order_book(OrderBookHandle handle) {
    if (handle) [[likely]] {
        try {
            delete static_cast<lob::OrderBook*>(handle);
            spdlog::info("Destroyed order book instance");
        } catch (const std::exception& e) {
            spdlog::error("Exception in destroy_order_book: {}", e.what());
        } catch (...) {
            spdlog::error("Unknown exception in destroy_order_book");
        }
    }
}

void set_trade_callback(OrderBookHandle handle, TradeCallback callback) {
    if (handle) [[likely]] {
        try {
            auto* book = static_cast<lob::OrderBook*>(handle);
            book->set_trade_callback(CTradeCallback(callback));
        } catch (const std::exception& e) {
            spdlog::error("Exception in set_trade_callback: {}", e.what());
        } catch (...) {
            spdlog::error("Unknown exception in set_trade_callback");
        }
    }
}

ErrorCode add_limit_order(
    OrderBookHandle handle,
    uint64_t id,
    double price,
    uint32_t quantity,
    int side
) {
    if (!handle) [[unlikely]] {
        return ERROR_INVALID_ORDER_ID;
    }
    
    try {
        auto* book = static_cast<lob::OrderBook*>(handle);
        lob::Side order_side = (side == SIDE_BUY) ? lob::Side::BUY : lob::Side::SELL;
        // Convert double to fixed-point for internal use
        lob::Price fixed_price = lob::price_to_fixed(price);
#ifdef HAVE_STD_EXPECTED
        auto result = book->add_limit_order(id, fixed_price, quantity, order_side);
        if (!result) {
            return static_cast<ErrorCode>(result.error());
        }
        return ERROR_SUCCESS;
#else
        lob::ErrorCode result = book->add_limit_order(id, fixed_price, quantity, order_side);
        return static_cast<ErrorCode>(result);
#endif
    } catch (const std::exception& e) {
        spdlog::error("Exception in add_limit_order: {}", e.what());
        return ERROR_UNKNOWN;
    } catch (...) {
        spdlog::error("Unknown exception in add_limit_order");
        return ERROR_UNKNOWN;
    }
}

ErrorCode cancel_order(OrderBookHandle handle, uint64_t id) {
    if (!handle) [[unlikely]] {
        return ERROR_ORDER_NOT_FOUND;
    }
    
    try {
        auto* book = static_cast<lob::OrderBook*>(handle);
#ifdef HAVE_STD_EXPECTED
        auto result = book->cancel_order(id);
        if (!result) {
            return static_cast<ErrorCode>(result.error());
        }
        return ERROR_SUCCESS;
#else
        lob::ErrorCode result = book->cancel_order(id);
        return static_cast<ErrorCode>(result);
#endif
    } catch (const std::exception& e) {
        spdlog::error("Exception in cancel_order: {}", e.what());
        return ERROR_UNKNOWN;
    } catch (...) {
        spdlog::error("Unknown exception in cancel_order");
        return ERROR_UNKNOWN;
    }
}

double get_best_bid(OrderBookHandle handle) {
    if (!handle) [[unlikely]] {
        return 0.0;
    }
    
    try {
        auto* book = static_cast<lob::OrderBook*>(handle);
        // Convert fixed-point to double for C API
        lob::Price fixed_price = book->get_best_bid();
        return lob::price_to_double(fixed_price);
    } catch (const std::exception& e) {
        spdlog::error("Exception in get_best_bid: {}", e.what());
        return 0.0;
    } catch (...) {
        spdlog::error("Unknown exception in get_best_bid");
        return 0.0;
    }
}

double get_best_ask(OrderBookHandle handle) {
    if (!handle) [[unlikely]] {
        return 0.0;
    }
    
    try {
        auto* book = static_cast<lob::OrderBook*>(handle);
        // Convert fixed-point to double for C API
        lob::Price fixed_price = book->get_best_ask();
        return lob::price_to_double(fixed_price);
    } catch (const std::exception& e) {
        spdlog::error("Exception in get_best_ask: {}", e.what());
        return 0.0;
    } catch (...) {
        spdlog::error("Unknown exception in get_best_ask");
        return 0.0;
    }
}

uint32_t get_quantity_at_price(OrderBookHandle handle, double price, int side) {
    if (!handle) [[unlikely]] {
        return 0;
    }
    
    try {
        auto* book = static_cast<lob::OrderBook*>(handle);
        lob::Side order_side = (side == SIDE_BUY) ? lob::Side::BUY : lob::Side::SELL;
        // Convert double to fixed-point for internal use
        lob::Price fixed_price = lob::price_to_fixed(price);
        return book->get_quantity_at_price(fixed_price, order_side);
    } catch (const std::exception& e) {
        spdlog::error("Exception in get_quantity_at_price: {}", e.what());
        return 0;
    } catch (...) {
        spdlog::error("Unknown exception in get_quantity_at_price");
        return 0;
    }
}

size_t get_bid_depth(OrderBookHandle handle) {
    if (!handle) [[unlikely]] {
        return 0;
    }
    
    try {
        auto* book = static_cast<lob::OrderBook*>(handle);
        return book->get_bid_depth();
    } catch (const std::exception& e) {
        spdlog::error("Exception in get_bid_depth: {}", e.what());
        return 0;
    } catch (...) {
        spdlog::error("Unknown exception in get_bid_depth");
        return 0;
    }
}

size_t get_ask_depth(OrderBookHandle handle) {
    if (!handle) [[unlikely]] {
        return 0;
    }
    
    try {
        auto* book = static_cast<lob::OrderBook*>(handle);
        return book->get_ask_depth();
    } catch (const std::exception& e) {
        spdlog::error("Exception in get_ask_depth: {}", e.what());
        return 0;
    } catch (...) {
        spdlog::error("Unknown exception in get_ask_depth");
        return 0;
    }
}

int is_empty(OrderBookHandle handle) {
    if (!handle) [[unlikely]] {
        return 1;
    }
    
    try {
        auto* book = static_cast<lob::OrderBook*>(handle);
        return book->is_empty() ? 1 : 0;
    } catch (const std::exception& e) {
        spdlog::error("Exception in is_empty: {}", e.what());
        return 1;
    } catch (...) {
        spdlog::error("Unknown exception in is_empty");
        return 1;
    }
}

size_t pool_capacity(OrderBookHandle handle) {
    if (!handle) [[unlikely]] {
        return 0;
    }
    
    try {
        auto* book = static_cast<lob::OrderBook*>(handle);
        return book->pool_capacity();
    } catch (const std::exception& e) {
        spdlog::error("Exception in pool_capacity: {}", e.what());
        return 0;
    } catch (...) {
        spdlog::error("Unknown exception in pool_capacity");
        return 0;
    }
}

size_t pool_allocated(OrderBookHandle handle) {
    if (!handle) [[unlikely]] {
        return 0;
    }
    
    try {
        auto* book = static_cast<lob::OrderBook*>(handle);
        return book->pool_allocated();
    } catch (const std::exception& e) {
        spdlog::error("Exception in pool_allocated: {}", e.what());
        return 0;
    } catch (...) {
        spdlog::error("Unknown exception in pool_allocated");
        return 0;
    }
}

size_t pool_free(OrderBookHandle handle) {
    if (!handle) [[unlikely]] {
        return 0;
    }
    
    try {
        auto* book = static_cast<lob::OrderBook*>(handle);
        return book->pool_free();
    } catch (const std::exception& e) {
        spdlog::error("Exception in pool_free: {}", e.what());
        return 0;
    } catch (...) {
        spdlog::error("Unknown exception in pool_free");
        return 0;
    }
}

} // extern "C"
