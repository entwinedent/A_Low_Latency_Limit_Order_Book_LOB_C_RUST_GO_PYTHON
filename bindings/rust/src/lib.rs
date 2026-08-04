mod ffi;

pub use ffi::{ErrorCode, Side};

// Safe Rust wrapper around the C API using cxx manual bridge
pub struct OrderBookWrapper {
    handle: cxx::UniquePtr<ffi::OrderBookHandle>,
}

impl OrderBookWrapper {
    pub fn new() -> Result<Self, ErrorCode> {
        Self::from_handle(unsafe { ffi::create_order_book() })
    }

    fn from_handle(handle: cxx::UniquePtr<ffi::OrderBookHandle>) -> Result<Self, ErrorCode> {
        if handle.is_null() {
            Err(ffi::ERROR_UNKNOWN)
        } else {
            Ok(OrderBookWrapper { handle })
        }
    }
    
    pub fn add_limit_order(
        &mut self,
        id: u64,
        price: f64,
        quantity: u32,
        side: Side,
    ) -> ErrorCode {
        unsafe { ffi::add_limit_order(self.handle.pin_mut(), id, price, quantity, side) }
    }
    
    pub fn cancel_order(&mut self, id: u64) -> ErrorCode {
        unsafe { ffi::cancel_order(self.handle.pin_mut(), id) }
    }
    
    pub fn get_best_bid(&self) -> f64 {
        unsafe { ffi::get_best_bid(&self.handle) }
    }
    
    pub fn get_best_ask(&self) -> f64 {
        unsafe { ffi::get_best_ask(&self.handle) }
    }
    
    pub fn get_quantity_at_price(&self, price: f64, side: Side) -> u32 {
        unsafe { ffi::get_quantity_at_price(&self.handle, price, side) }
    }
    
    pub fn get_bid_depth(&self) -> usize {
        unsafe { ffi::get_bid_depth(&self.handle) }
    }
    
    pub fn get_ask_depth(&self) -> usize {
        unsafe { ffi::get_ask_depth(&self.handle) }
    }
    
    pub fn is_empty(&self) -> bool {
        unsafe { ffi::is_empty(&self.handle) }
    }
    
    pub fn pool_capacity(&self) -> usize {
        unsafe { ffi::pool_capacity(&self.handle) }
    }
    
    pub fn pool_allocated(&self) -> usize {
        unsafe { ffi::pool_allocated(&self.handle) }
    }
    
    pub fn pool_free(&self) -> usize {
        unsafe { ffi::pool_free(&self.handle) }
    }
}

// Type alias for backwards compatibility
pub type OrderBook = OrderBookWrapper;

#[cfg(test)]
mod tests {
    use super::*;
    use cxx::UniquePtr;
    
    #[test]
    fn test_new_order_book() {
        let book = OrderBook::new().unwrap();
        assert!(book.is_empty());
    }

    #[test]
    fn test_from_handle_null() {
        let null_handle: UniquePtr<ffi::OrderBookHandle> = UniquePtr::null();
        let result = OrderBookWrapper::from_handle(null_handle);
        assert!(result.is_err());
        assert_eq!(result.err().unwrap(), ffi::ERROR_UNKNOWN);
    }
    
    #[test]
    fn test_add_limit_order() {
        let mut book = OrderBook::new().unwrap();
        
        let err = book.add_limit_order(1, 100.0, 10, ffi::SIDE_BUY);
        assert_eq!(err, ffi::ERROR_SUCCESS);
        
        assert_eq!(book.get_bid_depth(), 1);
    }
    
    #[test]
    fn test_get_best_bid_ask() {
        let mut book = OrderBook::new().unwrap();
        
        book.add_limit_order(1, 100.0, 10, ffi::SIDE_BUY);
        book.add_limit_order(2, 99.0, 5, ffi::SIDE_BUY);
        book.add_limit_order(3, 105.0, 5, ffi::SIDE_SELL);
        book.add_limit_order(4, 106.0, 10, ffi::SIDE_SELL);
        
        assert_eq!(book.get_best_bid(), 100.0);
        assert_eq!(book.get_best_ask(), 105.0);
    }
    
    #[test]
    fn test_cancel_order() {
        let mut book = OrderBook::new().unwrap();
        
        book.add_limit_order(1, 100.0, 10, ffi::SIDE_BUY);
        let err = book.cancel_order(1);
        assert_eq!(err, ffi::ERROR_SUCCESS);
        assert!(book.is_empty());
    }
    
    #[test]
    fn test_cancel_nonexistent_order() {
        let mut book = OrderBook::new().unwrap();
        
        let err = book.cancel_order(999);
        assert_eq!(err, ffi::ERROR_ORDER_NOT_FOUND);
    }
    
    #[test]
    fn test_invalid_order() {
        let mut book = OrderBook::new().unwrap();
        
        // Test invalid quantity
        let err = book.add_limit_order(1, 100.0, 0, ffi::SIDE_BUY);
        assert_eq!(err, ffi::ERROR_INVALID_QUANTITY);
        
        // Test invalid price
        let err = book.add_limit_order(2, -1.0, 10, ffi::SIDE_BUY);
        assert_eq!(err, ffi::ERROR_INVALID_PRICE);
    }
    
    #[test]
    fn test_pool_info() {
        let book = OrderBook::new().unwrap();
        
        assert!(book.pool_capacity() > 0);
        assert_eq!(book.pool_allocated(), 0);
        assert_eq!(book.pool_free(), book.pool_capacity());
    }

    #[test]
    fn test_remaining_wrapper_paths() {
        let mut book = OrderBook::new().unwrap();

        assert_eq!(book.add_limit_order(1, 100.0, 10, ffi::SIDE_BUY), ffi::ERROR_SUCCESS);
        assert_eq!(book.get_quantity_at_price(100.0, ffi::SIDE_BUY), 10);
        assert_eq!(book.get_bid_depth(), 1);
        assert_eq!(book.get_ask_depth(), 0);
        assert!(!book.is_empty());
        assert_eq!(book.pool_allocated(), 1);
        assert_eq!(book.pool_free(), book.pool_capacity() - 1);
    }

    #[test]
    fn test_matching_against_existing_bids() {
        let mut book = OrderBook::new().unwrap();

        assert_eq!(book.add_limit_order(1, 100.0, 10, ffi::SIDE_BUY), ffi::ERROR_SUCCESS);
        assert_eq!(book.add_limit_order(2, 100.0, 5, ffi::SIDE_SELL), ffi::ERROR_SUCCESS);
        assert_eq!(book.get_quantity_at_price(100.0, ffi::SIDE_BUY), 5);
        assert_eq!(book.get_quantity_at_price(100.0, ffi::SIDE_SELL), 0);
    }

    #[test]
    fn test_ffi_constants_are_referenced() {
        let _ = (
            ffi::SIDE_BUY,
            ffi::SIDE_SELL,
            ffi::ERROR_SUCCESS,
            ffi::ERROR_INVALID_ORDER_ID,
            ffi::ERROR_INVALID_PRICE,
            ffi::ERROR_INVALID_QUANTITY,
            ffi::ERROR_INVALID_SIDE,
            ffi::ERROR_ORDER_NOT_FOUND,
            ffi::ERROR_POOL_EXHAUSTED,
            ffi::ERROR_UNKNOWN,
        );
    }
}
