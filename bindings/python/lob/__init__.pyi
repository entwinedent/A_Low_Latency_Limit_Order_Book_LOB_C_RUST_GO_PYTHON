"""
Low-Latency Order Book Engine - Python Bindings Type Stubs

This module provides type stubs for the high-performance C++ order book engine.
"""

import ctypes
from collections.abc import Callable
from threading import RLock

class Side(ctypes.c_int):
    """Order side enumeration."""
    BUY: int
    SELL: int

class ErrorCode(ctypes.c_int):
    """Error code enumeration."""
    SUCCESS: int
    INVALID_ORDER_ID: int
    INVALID_PRICE: int
    INVALID_QUANTITY: int
    INVALID_SIDE: int
    ORDER_NOT_FOUND: int
    POOL_EXHAUSTED: int
    UNKNOWN: int

class Trade:
    """Represents a trade event."""
    
    maker_order_id: int
    taker_order_id: int
    price: float
    quantity: int
    timestamp: int
    side: Side
    
    def __init__(
        self,
        maker_order_id: int,
        taker_order_id: int,
        price: float,
        quantity: int,
        timestamp: int,
        side: Side,
    ) -> None: ...
    

class OrderBook:
    """
    Python wrapper for the low-latency order book engine.
    
    This class provides a thread-safe interface to the C++ order book implementation,
    supporting limit orders, cancellations, and real-time trade callbacks.
    """
    
    _handle: ctypes.c_void_p
    _lock: RLock
    _trade_callback: Callable[[Trade], None] | None
    _c_callback: ctypes.CFUNCTYPE | None
    
    def __init__(self) -> None: ...
    
    def __del__(self) -> None: ...
    
    def set_trade_callback(self, callback: Callable[[Trade], None]) -> None: ...
    
    def add_limit_order(
        self,
        order_id: int,
        price: float,
        quantity: int,
        side: Side
    ) -> ErrorCode: ...
    
    def cancel_order(self, order_id: int) -> ErrorCode: ...
    
    def get_best_bid(self) -> float: ...
    
    def get_best_ask(self) -> float: ...
    
    def get_quantity_at_price(self, price: float, side: Side) -> int: ...
    
    def get_bid_depth(self) -> int: ...
    
    def get_ask_depth(self) -> int: ...
    
    def is_empty(self) -> bool: ...
    
    def pool_capacity(self) -> int: ...
    
    def pool_allocated(self) -> int: ...
    
    def pool_free(self) -> int: ...
    
    def snapshot_to_string(self) -> str: ...

__all__: list[str] = ['ErrorCode', 'OrderBook', 'Side', 'Trade']
