#pragma once

#include "Common.h"
#include <concepts>
#include <type_traits>

namespace lob {

// Concept for order-like types
template<typename T>
concept OrderLike = requires(T order) {
    { order.id } -> std::convertible_to<OrderID>;
    { order.price } -> std::convertible_to<Price>;
    { order.quantity } -> std::convertible_to<Quantity>;
    { order.side } -> std::convertible_to<Side>;
};

// Concept for numeric types used in trading
template<typename T>
concept Numeric = std::is_arithmetic_v<T> && !std::is_same_v<T, bool>;

// Concept for valid price type
template<typename T>
concept PriceType = Numeric<T> && requires(T price) {
    { price > 0 } -> std::convertible_to<bool>;
};

// Concept for valid quantity type
template<typename T>
concept QuantityType = Numeric<T> && std::is_integral_v<T> && requires(T qty) {
    { qty > 0 } -> std::convertible_to<bool>;
};

// Concept for order ID type
template<typename T>
concept OrderIDType = std::is_integral_v<T> && std::is_unsigned_v<T>;

// Concept for callable trade callback
template<typename F>
concept TradeCallback = requires(F callback, const Trade& trade) {
    { callback(trade) } -> std::same_as<void>;
};

// Concept for memory pool
template<typename T>
concept MemoryPool = requires(T pool) {
    { pool.allocate() } -> std::same_as<typename T::value_type*>;
    { pool.deallocate(std::declval<typename T::value_type*>()) } -> std::same_as<void>;
    { pool.capacity() } -> std::convertible_to<size_t>;
    { pool.allocated_count() } -> std::convertible_to<size_t>;
};

// Concept for order book
template<typename T>
concept OrderBookType = requires(T book, OrderID id, Price price, Quantity qty, Side side) {
    { book.add_limit_order(id, price, qty, side) } -> std::same_as<ErrorCode>;
    { book.cancel_order(id) } -> std::same_as<ErrorCode>;
    { book.get_best_bid() } -> std::convertible_to<Price>;
    { book.get_best_ask() } -> std::convertible_to<Price>;
    { book.is_empty() } -> std::convertible_to<bool>;
};

// Concept for hash map
template<typename T>
concept HashMap = requires(T map, typename T::key_type key) {
    typename T::key_type;
    typename T::mapped_type;
    { map.find(key) } -> std::same_as<typename T::iterator>;
    { map.insert(std::make_pair(key, typename T::mapped_type{})) } -> std::same_as<typename T::iterator>;
    { map.erase(key) } -> std::convertible_to<size_t>;
};

// Concept for sorted map
template<typename T>
concept SortedMap = requires(T map, typename T::key_type key) {
    typename T::key_type;
    typename T::mapped_type;
    { map.find(key) } -> std::same_as<typename T::iterator>;
    { map.lower_bound(key) } -> std::same_as<typename T::iterator>;
    { map.upper_bound(key) } -> std::same_as<typename T::iterator>;
};

// Concept for allocator
template<typename T>
concept Allocator = requires(T alloc, size_t n) {
    { alloc.allocate(n) } -> std::same_as<typename T::value_type*>;
    { alloc.deallocate(std::declval<typename T::value_type*>(), n) } -> std::same_as<void>;
};

// Concept for range
template<typename T>
concept Range = requires(T range) {
    { std::begin(range) } -> std::input_iterator;
    { std::end(range) } -> std::input_iterator;
};

// Compile-time validation helpers
template<typename T>
constexpr bool is_valid_price_v = PriceType<T>;

template<typename T>
constexpr bool is_valid_quantity_v = QuantityType<T>;

template<typename T>
constexpr bool is_valid_order_id_v = OrderIDType<T>;

// Type traits for trading types
template<typename T>
struct is_trading_type : std::false_type {};

template<>
struct is_trading_type<OrderID> : std::true_type {};

template<>
struct is_trading_type<Price> : std::true_type {};

template<>
struct is_trading_type<Quantity> : std::true_type {};

template<>
struct is_trading_type<Side> : std::true_type {};

template<typename T>
constexpr bool is_trading_type_v = is_trading_type<T>::value;

} // namespace lob
