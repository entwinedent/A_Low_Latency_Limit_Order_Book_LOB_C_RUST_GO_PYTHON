// Safe C++ interop using cxx manual bridge
#[cxx::bridge]
mod ffi {
    // Shared types
    #[namespace = "cxx_bridge"]
    extern "C++" {
        include!("cxx_wrapper.h");
        
        // Opaque type for OrderBook handle
        type OrderBookHandle;
        
        // Create a new order book and return as opaque pointer
        unsafe fn create_order_book() -> UniquePtr<OrderBookHandle>;
        
        // Add a limit order
        unsafe fn add_limit_order(handle: Pin<&mut OrderBookHandle>, id: u64, price: f64, quantity: u32, side: i32) -> i32;
        
        // Cancel an order
        unsafe fn cancel_order(handle: Pin<&mut OrderBookHandle>, id: u64) -> i32;
        
        // Get best bid
        unsafe fn get_best_bid(handle: &OrderBookHandle) -> f64;
        
        // Get best ask
        unsafe fn get_best_ask(handle: &OrderBookHandle) -> f64;
        
        // Get quantity at price
        unsafe fn get_quantity_at_price(handle: &OrderBookHandle, price: f64, side: i32) -> u32;
        
        // Get bid depth
        unsafe fn get_bid_depth(handle: &OrderBookHandle) -> usize;
        
        // Get ask depth
        unsafe fn get_ask_depth(handle: &OrderBookHandle) -> usize;
        
        // Check if empty
        unsafe fn is_empty(handle: &OrderBookHandle) -> bool;
        
        // Pool statistics
        unsafe fn pool_capacity(handle: &OrderBookHandle) -> usize;
        unsafe fn pool_allocated(handle: &OrderBookHandle) -> usize;
        unsafe fn pool_free(handle: &OrderBookHandle) -> usize;
    }
}

// Re-export cxx-generated types and functions for use in lib.rs
pub use crate::ffi::ffi::{
    OrderBookHandle,
    create_order_book,
    add_limit_order,
    cancel_order,
    get_best_bid,
    get_best_ask,
    get_quantity_at_price,
    get_bid_depth,
    get_ask_depth,
    is_empty,
    pool_capacity,
    pool_allocated,
    pool_free,
};

// Use i32 for Side and ErrorCode to match C API typedefs
pub type Side = i32;
pub const SIDE_BUY: Side = 0;
pub const SIDE_SELL: Side = 1;

pub type ErrorCode = i32;
pub const ERROR_SUCCESS: ErrorCode = 0;
pub const ERROR_INVALID_ORDER_ID: ErrorCode = -1;
pub const ERROR_INVALID_PRICE: ErrorCode = -2;
pub const ERROR_INVALID_QUANTITY: ErrorCode = -3;
pub const ERROR_INVALID_SIDE: ErrorCode = -4;
pub const ERROR_ORDER_NOT_FOUND: ErrorCode = -5;
pub const ERROR_POOL_EXHAUSTED: ErrorCode = -6;
pub const ERROR_UNKNOWN: ErrorCode = -99;
