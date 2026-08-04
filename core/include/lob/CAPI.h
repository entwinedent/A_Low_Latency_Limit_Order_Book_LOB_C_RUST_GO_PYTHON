#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle type for OrderBook
typedef void* OrderBookHandle;

// Side enumeration for C API
typedef int Side;
#define SIDE_BUY 0
#define SIDE_SELL 1

// Error codes for C API
typedef int ErrorCode;
#define ERROR_SUCCESS 0
#define ERROR_INVALID_ORDER_ID -1
#define ERROR_INVALID_PRICE -2
#define ERROR_INVALID_QUANTITY -3
#define ERROR_INVALID_SIDE -4
#define ERROR_ORDER_NOT_FOUND -5
#define ERROR_POOL_EXHAUSTED -6
#define ERROR_UNKNOWN -99

// Trade callback function type
typedef void (*TradeCallback)(
    uint64_t maker_order_id,
    uint64_t taker_order_id,
    double price,
    uint32_t quantity,
    uint64_t timestamp,
    int side
);

// Create a new order book instance
OrderBookHandle create_order_book();

// Destroy an order book instance
void destroy_order_book(OrderBookHandle handle);

// Set trade callback for an order book
void set_trade_callback(OrderBookHandle handle, TradeCallback callback);

// Add a limit order to the order book
ErrorCode add_limit_order(
    OrderBookHandle handle,
    uint64_t id,
    double price,
    uint32_t quantity,
    int side
);

// Cancel an existing order
ErrorCode cancel_order(OrderBookHandle handle, uint64_t id);

// Get the best bid price
double get_best_bid(OrderBookHandle handle);

// Get the best ask price
double get_best_ask(OrderBookHandle handle);

// Get total quantity at a specific price level
uint32_t get_quantity_at_price(OrderBookHandle handle, double price, int side);

// Get bid depth (number of price levels)
size_t get_bid_depth(OrderBookHandle handle);

// Get ask depth (number of price levels)
size_t get_ask_depth(OrderBookHandle handle);

// Check if order book is empty
int is_empty(OrderBookHandle handle);

// Get memory pool statistics
size_t pool_capacity(OrderBookHandle handle);
size_t pool_allocated(OrderBookHandle handle);
size_t pool_free(OrderBookHandle handle);

#ifdef __cplusplus
}
#endif
