"""
Low-Latency Order Book Engine - Python Bindings

This module provides Python bindings for the high-performance C++ order book engine.
"""

import ctypes
import os
import sys
from collections.abc import Callable
from io import StringIO
from threading import RLock
from typing import Optional

# Load the shared library
def _load_library():
    """Load the appropriate shared library based on the platform."""
    if sys.platform == "win32":
        lib_name = "lob_core.dll"
    elif sys.platform == "darwin":
        lib_name = "liblob_core.dylib"
    else:
        lib_name = "liblob_core.so"
    
    # Try to load from build directory first
    build_dir = os.path.join(os.path.dirname(__file__), "..", "..", "..", "build", "core", "Release")
    lib_path = os.path.join(build_dir, lib_name)
    
    if os.path.exists(lib_path):
        return ctypes.CDLL(lib_path)
    
    # Fall back to system library search
    try:
        return ctypes.CDLL(lib_name)
    except OSError:
        raise ImportError(
            f"Could not load {lib_name}. Make sure the C++ library is built "
            f"and available in the build directory or system library path."
        )

try:
    _lib = _load_library()
except ImportError as e:
    raise ImportError(
        f"Failed to load order book library: {e}\n"
        "Please build the C++ library first using CMake."
    )

# Define C types and enums
class Side(ctypes.c_int):
    BUY = 0
    SELL = 1

class ErrorCode(ctypes.c_int):
    SUCCESS = 0
    INVALID_ORDER_ID = -1
    INVALID_PRICE = -2
    INVALID_QUANTITY = -3
    INVALID_SIDE = -4
    ORDER_NOT_FOUND = -5
    POOL_EXHAUSTED = -6
    UNKNOWN = -99

# Trade callback type
TRADE_CALLBACK = ctypes.CFUNCTYPE(
    None,
    ctypes.c_uint64,  # maker_order_id
    ctypes.c_uint64,  # taker_order_id
    ctypes.c_int64,   # price (fixed-point)
    ctypes.c_uint32,  # quantity
    ctypes.c_uint64,  # timestamp
    ctypes.c_int,     # side
)

# Define C function signatures
_lib.create_order_book.restype = ctypes.c_void_p
_lib.create_order_book.argtypes = []

_lib.destroy_order_book.restype = None
_lib.destroy_order_book.argtypes = [ctypes.c_void_p]

_lib.set_trade_callback.restype = None
_lib.set_trade_callback.argtypes = [ctypes.c_void_p, TRADE_CALLBACK]

_lib.add_limit_order.restype = ctypes.c_int
_lib.add_limit_order.argtypes = [
    ctypes.c_void_p,   # handle
    ctypes.c_uint64,   # id
    ctypes.c_int64,    # price (fixed-point)
    ctypes.c_uint32,   # quantity
    ctypes.c_int,      # side
]

_lib.cancel_order.restype = ctypes.c_int
_lib.cancel_order.argtypes = [
    ctypes.c_void_p,   # handle
    ctypes.c_uint64,   # id
]

_lib.get_best_bid.restype = ctypes.c_int64
_lib.get_best_bid.argtypes = [ctypes.c_void_p]

_lib.get_best_ask.restype = ctypes.c_int64
_lib.get_best_ask.argtypes = [ctypes.c_void_p]

_lib.get_quantity_at_price.restype = ctypes.c_uint32
_lib.get_quantity_at_price.argtypes = [
    ctypes.c_void_p,   # handle
    ctypes.c_int64,    # price (fixed-point)
    ctypes.c_int,      # side
]

_lib.get_bid_depth.restype = ctypes.c_size_t
_lib.get_bid_depth.argtypes = [ctypes.c_void_p]

_lib.get_ask_depth.restype = ctypes.c_size_t
_lib.get_ask_depth.argtypes = [ctypes.c_void_p]

_lib.is_empty.restype = ctypes.c_int
_lib.is_empty.argtypes = [ctypes.c_void_p]

_lib.pool_capacity.restype = ctypes.c_size_t
_lib.pool_capacity.argtypes = [ctypes.c_void_p]

_lib.pool_allocated.restype = ctypes.c_size_t
_lib.pool_allocated.argtypes = [ctypes.c_void_p]

_lib.pool_free.restype = ctypes.c_size_t
_lib.pool_free.argtypes = [ctypes.c_void_p]


# Fixed-point arithmetic constants
PRICE_SCALE_FACTOR = 10000  # 4 decimal places

def price_to_fixed(price: float) -> int:
    """Convert float price to fixed-point integer."""
    return int(price * PRICE_SCALE_FACTOR + 0.5)

def price_to_float(price: int) -> float:
    """Convert fixed-point integer to float price."""
    return price / PRICE_SCALE_FACTOR


class Trade:
    """Represents a trade event."""
    
    def __init__(
        self,
        maker_order_id: int,
        taker_order_id: int,
        price: int,  # Fixed-point
        quantity: int,
        timestamp: int,
        side: Side,
    ):
        self.maker_order_id = maker_order_id
        self.taker_order_id = taker_order_id
        self.price = price  # Fixed-point
        self.quantity = quantity
        self.timestamp = timestamp
        self.side = side
    
    @property
    def price_float(self) -> float:
        """Get price as float for convenience."""
        return price_to_float(self.price)
    
    def __repr__(self) -> str:
        side_str = "BUY" if self.side == Side.BUY else "SELL"
        return (
            f"Trade(maker={self.maker_order_id}, taker={self.taker_order_id}, "
            f"price={self.price_float:.4f}, qty={self.quantity}, side={side_str})"
        )


class OrderBook:
    """
    Python wrapper for the low-latency order book engine.
    
    This class provides a thread-safe interface to the C++ order book implementation,
    supporting limit orders, cancellations, and real-time trade callbacks.
    """
    
    def __init__(self):
        """Create a new order book instance."""
        self._handle = _lib.create_order_book()
        if not self._handle:
            raise RuntimeError("Failed to create order book")
        
        self._lock = RLock()
        self._trade_callback: Optional[Callable[[Trade], None]] = None
        self._c_callback: Optional[TRADE_CALLBACK] = None
    
    def __del__(self):
        """Clean up the order book instance."""
        if hasattr(self, '_handle') and self._handle:
            _lib.destroy_order_book(self._handle)
    
    def set_trade_callback(self, callback: Callable[[Trade], None]) -> None:
        """
        Set a callback function to receive trade events.
        
        Args:
            callback: A function that takes a Trade object as its argument.
        """
        with self._lock:
            self._trade_callback = callback
            
            if callback is not None:
                def c_callback_wrapper(
                    maker_order_id: int,
                    taker_order_id: int,
                    price: int,  # Fixed-point
                    quantity: int,
                    timestamp: int,
                    side: int,
                ):
                    trade = Trade(
                        maker_order_id, taker_order_id, price,
                        quantity, timestamp, Side(side)
                    )
                    self._trade_callback(trade)

                self._c_callback = TRADE_CALLBACK(c_callback_wrapper)
                _lib.set_trade_callback(self._handle, self._c_callback)
            else:
                self._c_callback = None
                _lib.set_trade_callback(self._handle, ctypes.cast(None, TRADE_CALLBACK))
    
    def add_limit_order(
        self,
        order_id: int,
        price: float,
        quantity: int,
        side: Side
    ) -> ErrorCode:
        """
        Add a limit order to the order book.
        
        Args:
            order_id: Unique identifier for the order
            price: Limit price for the order (float, will be converted to fixed-point)
            quantity: Order quantity
            side: Order side (Side.BUY or Side.SELL)
        
        Returns:
            ErrorCode indicating success or failure
        """
        with self._lock:
            return ErrorCode(
                _lib.add_limit_order(
                    self._handle,
                    order_id,
                    price_to_fixed(price),  # Convert to fixed-point
                    quantity,
                    side
                )
            )
    
    def cancel_order(self, order_id: int) -> ErrorCode:
        """
        Cancel an existing order.
        
        Args:
            order_id: ID of the order to cancel
        
        Returns:
            ErrorCode indicating success or failure
        """
        with self._lock:
            return ErrorCode(_lib.cancel_order(self._handle, order_id))
    
    def get_best_bid(self) -> float:
        """Get the current best bid price (converted from fixed-point to float)."""
        with self._lock:
            return price_to_float(_lib.get_best_bid(self._handle))
    
    def get_best_ask(self) -> float:
        """Get the current best ask price (converted from fixed-point to float)."""
        with self._lock:
            return price_to_float(_lib.get_best_ask(self._handle))
    
    def get_quantity_at_price(self, price: float, side: Side) -> int:
        """Get total quantity at a specific price level."""
        with self._lock:
            return _lib.get_quantity_at_price(self._handle, price_to_fixed(price), side)
    
    def get_bid_depth(self) -> int:
        """Get the number of bid price levels."""
        with self._lock:
            return _lib.get_bid_depth(self._handle)
    
    def get_ask_depth(self) -> int:
        """Get the number of ask price levels."""
        with self._lock:
            return _lib.get_ask_depth(self._handle)
    
    def is_empty(self) -> bool:
        """Check if the order book is empty."""
        with self._lock:
            return _lib.is_empty(self._handle) != 0
    
    def pool_capacity(self) -> int:
        """Get the total capacity of the memory pool."""
        with self._lock:
            return _lib.pool_capacity(self._handle)
    
    def pool_allocated(self) -> int:
        """Get the number of allocated orders in the pool."""
        with self._lock:
            return _lib.pool_allocated(self._handle)
    
    def pool_free(self) -> int:
        """Get the number of free slots in the pool."""
        with self._lock:
            return _lib.pool_free(self._handle)
    
    def snapshot_to_string(self) -> str:
        """
        Create a snapshot of the order book state as a string.
        
        This uses in-memory StringIO for zero disk I/O.
        """
        output = StringIO()
        output.write("Order Book Snapshot:\n")
        output.write(f"  Best Bid: {self.get_best_bid()}\n")
        output.write(f"  Best Ask: {self.get_best_ask()}\n")
        output.write(f"  Bid Depth: {self.get_bid_depth()}\n")
        output.write(f"  Ask Depth: {self.get_ask_depth()}\n")
        output.write(f"  Pool: {self.pool_allocated()}/{self.pool_capacity()} allocated\n")
        return output.getvalue()


__all__ = ['ErrorCode', 'OrderBook', 'Side', 'Trade']
