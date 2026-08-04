#pragma once

#include <cstdint>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <cmath>

#ifdef HAVE_STD_EXPECTED
#include <expected>
#endif

#ifdef HAVE_STD_PRINT
#include <print>
#include <format>
#endif

#ifdef HAVE_DEDUCING_THIS
// Deducing this is supported - can use explicit object parameter syntax
#endif

#ifdef HAVE_STD_MDSPAN
#include <mdspan>
#elif defined(HAVE_KOKKOS_MDSPAN)
#include <mdspan/mdspan.hpp>
namespace std {
    // Alias Kokkos mdspan to std namespace for compatibility
    using Kokkos::mdspan;
    using Kokkos::extents;
    using Kokkos::dextents;
}
#endif

namespace lob {

// Order side enumeration
enum class Side : uint8_t {
    BUY = 0,
    SELL = 1
};

// Error codes
enum class ErrorCode : int8_t {
    SUCCESS = 0,
    INVALID_ORDER_ID = -1,
    INVALID_PRICE = -2,
    INVALID_QUANTITY = -3,
    INVALID_SIDE = -4,
    ORDER_NOT_FOUND = -5,
    POOL_EXHAUSTED = -6,
    UNKNOWN_ERROR = -99
};

// C++23 std::expected support
#ifdef HAVE_STD_EXPECTED
// Result type alias for error handling
template<typename T>
using Result = std::expected<T, ErrorCode>;

// Helper to create error result
template<typename T>
inline auto make_error(ErrorCode code) -> Result<T> {
    return std::unexpected(code);
}

// Helper to create success result
template<typename T>
inline auto make_success(T value) -> Result<T> {
    return value;
}

// Void result type for operations without return value
using VoidResult = Result<void>;
inline VoidResult make_void_success() { return {}; }
inline VoidResult make_void_error(ErrorCode code) { return std::unexpected(code); }
#endif

// Type aliases for clarity
using OrderID = uint64_t;
using Price = int64_t;  // Fixed-point arithmetic: 4 decimal places (tick size = 0.0001)
using Quantity = uint32_t;
using Timestamp = uint64_t;

// Fixed-point arithmetic constants
constexpr int PRICE_SCALE_FACTOR = 10000;  // 4 decimal places
constexpr Price MIN_PRICE = 1;  // 0.0001 in fixed-point
constexpr Price MAX_PRICE = 10000000000;  // 1000000.0 in fixed-point
constexpr Quantity MAX_QUANTITY = 1000000;

// Helper functions for fixed-point conversion
inline Price price_to_fixed(double price) {
    return static_cast<Price>(std::lround(price * PRICE_SCALE_FACTOR));
}

inline double price_to_double(Price price) {
    return static_cast<double>(price) / PRICE_SCALE_FACTOR;
}

inline std::string price_to_string(Price price) {
    double value = price_to_double(price);
#ifdef HAVE_STD_PRINT
    return std::format("{:.4f}", value);
#else
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4) << value;
    return oss.str();
#endif
}

// Helper functions
inline const char* side_to_string(Side side) {
    return side == Side::BUY ? "BUY" : "SELL";
}

inline const char* error_to_string(ErrorCode error) {
    switch (error) {
        case ErrorCode::SUCCESS: return "SUCCESS";
        case ErrorCode::INVALID_ORDER_ID: return "INVALID_ORDER_ID";
        case ErrorCode::INVALID_PRICE: return "INVALID_PRICE";
        case ErrorCode::INVALID_QUANTITY: return "INVALID_QUANTITY";
        case ErrorCode::INVALID_SIDE: return "INVALID_SIDE";
        case ErrorCode::ORDER_NOT_FOUND: return "ORDER_NOT_FOUND";
        case ErrorCode::POOL_EXHAUSTED: return "POOL_EXHAUSTED";
        default: return "UNKNOWN_ERROR";
    }
}

// Trade event structure
struct Trade {
    OrderID maker_order_id;
    OrderID taker_order_id;
    Price price;
    Quantity quantity;
    Timestamp timestamp;
    Side side;

#ifdef HAVE_DEDUCING_THIS
    // Example of deducing this: method that can work with both const and non-const instances
    auto to_string(this auto&& self) {
#ifdef HAVE_STD_PRINT
        return std::format("Trade{{maker={}, taker={}, price={}, qty={}, side={}}}",
                           self.maker_order_id, self.taker_order_id,
                           price_to_string(self.price), self.quantity,
                           side_to_string(self.side));
#else
        std::ostringstream oss;
        oss << "Trade{maker=" << self.maker_order_id
           << ", taker=" << self.taker_order_id
           << ", price=" << price_to_string(self.price)
           << ", qty=" << self.quantity
           << ", side=" << side_to_string(self.side)
           << "}";
        return oss.str();
#endif
    }
#endif
};

inline std::ostream& operator<<(std::ostream& os, const Trade& trade) {
    os << "Trade{maker=" << trade.maker_order_id
       << ", taker=" << trade.taker_order_id
       << ", price=" << trade.price
       << ", qty=" << trade.quantity
       << ", side=" << side_to_string(trade.side)
       << "}";
    return os;
}

} // namespace lob
