package engine

import (
	"fmt"
	"sync"
	"time"
	"unsafe"
)

const (
	poolCapacity     = 1_000_000
	priceScaleFactor = 10000 // 4 decimal places for fixed-point arithmetic
)

// Side represents the order side (Buy or Sell)
type Side int

const (
	SideBuy  Side = 0
	SideSell Side = 1
)

// ErrorCode represents the error codes returned by the engine
type ErrorCode int

const (
	ErrorSuccess         ErrorCode = 0
	ErrorInvalidOrderID  ErrorCode = -1
	ErrorInvalidPrice    ErrorCode = -2
	ErrorInvalidQuantity ErrorCode = -3
	ErrorInvalidSide     ErrorCode = -4
	ErrorOrderNotFound   ErrorCode = -5
	ErrorPoolExhausted   ErrorCode = -6
	ErrorUnknown         ErrorCode = -99
)

// Trade represents a trade event
type Trade struct {
	MakerOrderID uint64
	TakerOrderID uint64
	Price        int64 // Fixed-point (4 decimal places)
	Quantity     uint32
	Timestamp    uint64
	Side         Side
}

// PriceToFixed converts a float price to fixed-point integer
func PriceToFixed(price float64) int64 {
	return int64(price*float64(priceScaleFactor) + 0.5)
}

// PriceToFloat converts a fixed-point integer to float price
func PriceToFloat(price int64) float64 {
	return float64(price) / float64(priceScaleFactor)
}

type order struct {
	id       uint64
	price    int64 // Fixed-point
	quantity uint32
	side     Side
}

type priceLevel struct {
	price         int64 // Fixed-point
	orders        []*order
	totalQuantity uint32
}

// Engine represents the order book engine
type Engine struct {
	mu            sync.RWMutex
	tradeCallback TradeCallback
	orders        map[uint64]*order
	bids          map[int64]*priceLevel // Fixed-point keys
	asks          map[int64]*priceLevel // Fixed-point keys
	closed        bool
}

// TradeCallback is the function type for trade callbacks
type TradeCallback func(trade Trade)

// NewEngine creates a new order book engine
func NewEngine() *Engine {
	return &Engine{
		orders: make(map[uint64]*order),
		bids:   make(map[int64]*priceLevel),
		asks:   make(map[int64]*priceLevel),
	}
}

// NewEngineFromHandle creates an engine from an existing handle.
// In this implementation the handle is the engine pointer itself.
func NewEngineFromHandle(handle unsafe.Pointer) *Engine {
	if handle == nil {
		return nil
	}
	return (*Engine)(handle)
}

// Close destroys the engine and releases resources.
func (e *Engine) Close() {
	if e == nil {
		return
	}
	if e.closed {
		return
	}

	e.mu.Lock()
	defer e.mu.Unlock()
	e.closed = true
	e.orders = nil
	e.bids = nil
	e.asks = nil
}

// SetTradeCallback sets the callback function for trade events.
func (e *Engine) SetTradeCallback(callback TradeCallback) {
	if e == nil {
		return
	}
	e.mu.Lock()
	defer e.mu.Unlock()
	e.tradeCallback = callback
}

// AddLimitOrder adds a limit order to the order book.
func (e *Engine) AddLimitOrder(id uint64, price float64, quantity uint32, side Side) ErrorCode {
	if e == nil {
		return ErrorInvalidOrderID
	}

	e.mu.Lock()
	defer e.mu.Unlock()

	if e.closed {
		return ErrorUnknown
	}
	if quantity == 0 || quantity > 1000000 {
		return ErrorInvalidQuantity
	}
	if price < 0 || price > 1e12 {
		return ErrorInvalidPrice
	}
	if side != SideBuy && side != SideSell {
		return ErrorInvalidSide
	}
	if _, exists := e.orders[id]; exists {
		return ErrorInvalidOrderID
	}

	order := &order{id: id, price: PriceToFixed(price), quantity: quantity, side: side}
	e.orders[id] = order

	if side == SideBuy {
		e.matchAgainstAsks(order)
	} else {
		e.matchAgainstBids(order)
	}

	if order.quantity > 0 {
		e.addOrderToBook(order)
	} else {
		delete(e.orders, id)
	}

	return ErrorSuccess
}

// CancelOrder cancels an existing order.
func (e *Engine) CancelOrder(id uint64) ErrorCode {
	if e == nil {
		return ErrorOrderNotFound
	}

	e.mu.Lock()
	defer e.mu.Unlock()

	order, exists := e.orders[id]
	if !exists {
		return ErrorOrderNotFound
	}

	e.removeOrderFromBook(order)
	delete(e.orders, id)
	return ErrorSuccess
}

// GetBestBid returns the current best bid price (converted from fixed-point to float).
func (e *Engine) GetBestBid() float64 {
	if e == nil {
		return 0
	}
	e.mu.RLock()
	defer e.mu.RUnlock()
	return PriceToFloat(bestPrice(e.bids, true))
}

// GetBestAsk returns the current best ask price (converted from fixed-point to float).
func (e *Engine) GetBestAsk() float64 {
	if e == nil {
		return 0
	}
	e.mu.RLock()
	defer e.mu.RUnlock()
	return PriceToFloat(bestPrice(e.asks, false))
}

// GetQuantityAtPrice returns the total quantity at a specific price level.
func (e *Engine) GetQuantityAtPrice(price float64, side Side) uint32 {
	if e == nil {
		return 0
	}
	e.mu.RLock()
	defer e.mu.RUnlock()
	var levels map[int64]*priceLevel
	if side == SideBuy {
		levels = e.bids
	} else {
		levels = e.asks
	}
	if level, ok := levels[PriceToFixed(price)]; ok {
		return level.totalQuantity
	}
	return 0
}

// GetBidDepth returns the number of bid price levels.
func (e *Engine) GetBidDepth() uint {
	if e == nil {
		return 0
	}
	e.mu.RLock()
	defer e.mu.RUnlock()
	return uint(len(e.bids))
}

// GetAskDepth returns the number of ask price levels.
func (e *Engine) GetAskDepth() uint {
	if e == nil {
		return 0
	}
	e.mu.RLock()
	defer e.mu.RUnlock()
	return uint(len(e.asks))
}

// IsEmpty returns true if the order book is empty.
func (e *Engine) IsEmpty() bool {
	if e == nil {
		return true
	}
	e.mu.RLock()
	defer e.mu.RUnlock()
	return len(e.orders) == 0
}

// PoolCapacity returns the total capacity of the memory pool.
func (e *Engine) PoolCapacity() uint {
	if e == nil {
		return 0
	}
	return poolCapacity
}

// PoolAllocated returns the number of allocated orders in the pool.
func (e *Engine) PoolAllocated() uint {
	if e == nil {
		return 0
	}
	e.mu.RLock()
	defer e.mu.RUnlock()
	return uint(len(e.orders))
}

// PoolFree returns the number of free slots in the pool.
func (e *Engine) PoolFree() uint {
	if e == nil {
		return 0
	}
	return poolCapacity - uint(len(e.orders))
}

// String returns a string representation of the error code.
func (e ErrorCode) String() string {
	switch e {
	case ErrorSuccess:
		return "SUCCESS"
	case ErrorInvalidOrderID:
		return "INVALID_ORDER_ID"
	case ErrorInvalidPrice:
		return "INVALID_PRICE"
	case ErrorInvalidQuantity:
		return "INVALID_QUANTITY"
	case ErrorInvalidSide:
		return "INVALID_SIDE"
	case ErrorOrderNotFound:
		return "ORDER_NOT_FOUND"
	case ErrorPoolExhausted:
		return "POOL_EXHAUSTED"
	default:
		return fmt.Sprintf("UNKNOWN(%d)", e)
	}
}

// GetHandle returns the underlying handle (for advanced use cases).
func (e *Engine) GetHandle() unsafe.Pointer {
	if e == nil {
		return nil
	}
	return unsafe.Pointer(e)
}

func (e *Engine) matchAgainstAsks(order *order) {
	for order.quantity > 0 {
		_, level, ok := e.bestAskLevel()
		if !ok || order.price < level.price {
			break
		}

		maker := level.orders[0]
		tradeQty := order.quantity
		if maker.quantity < tradeQty {
			tradeQty = maker.quantity
		}

		e.executeTrade(maker, order, tradeQty)
		maker.quantity -= tradeQty
		order.quantity -= tradeQty
		level.totalQuantity -= tradeQty

		if maker.quantity == 0 {
			e.removeOrderFromBook(maker)
			delete(e.orders, maker.id)
		}
	}
}

func (e *Engine) matchAgainstBids(order *order) {
	for order.quantity > 0 {
		_, level, ok := e.bestBidLevel()
		if !ok || order.price > level.price {
			break
		}

		maker := level.orders[0]
		tradeQty := order.quantity
		if maker.quantity < tradeQty {
			tradeQty = maker.quantity
		}

		e.executeTrade(maker, order, tradeQty)
		maker.quantity -= tradeQty
		order.quantity -= tradeQty
		level.totalQuantity -= tradeQty

		if maker.quantity == 0 {
			e.removeOrderFromBook(maker)
			delete(e.orders, maker.id)
		}
	}
}

func (e *Engine) executeTrade(maker, taker *order, quantity uint32) {
	if e.tradeCallback == nil {
		return
	}
	trade := Trade{
		MakerOrderID: maker.id,
		TakerOrderID: taker.id,
		Price:        maker.price,
		Quantity:     quantity,
		Timestamp:    uint64(time.Now().UnixNano()),
		Side:         maker.side,
	}
	e.tradeCallback(trade)
}

func (e *Engine) addOrderToBook(order *order) {
	var levels map[int64]*priceLevel
	if order.side == SideBuy {
		levels = e.bids
	} else {
		levels = e.asks
	}

	level, exists := levels[order.price]
	if !exists {
		level = &priceLevel{price: order.price}
		levels[order.price] = level
	}

	level.orders = append(level.orders, order)
	level.totalQuantity += order.quantity
}

func (e *Engine) removeOrderFromBook(order *order) {
	var levels map[int64]*priceLevel
	if order.side == SideBuy {
		levels = e.bids
	} else {
		levels = e.asks
	}

	level, exists := levels[order.price]
	if !exists {
		return
	}

	for index, current := range level.orders {
		if current == order {
			level.orders = append(level.orders[:index], level.orders[index+1:]...)
			if len(level.orders) == 0 {
				delete(levels, order.price)
			} else {
				level.totalQuantity -= order.quantity
			}
			return
		}
	}
}

func (e *Engine) bestAskLevel() (int64, *priceLevel, bool) {
	var bestPrice int64
	var bestLevel *priceLevel
	var found bool
	for price, level := range e.asks {
		if !found || price < bestPrice {
			bestPrice = price
			bestLevel = level
			found = true
		}
	}
	return bestPrice, bestLevel, found
}

func (e *Engine) bestBidLevel() (int64, *priceLevel, bool) {
	var bestPrice int64
	var bestLevel *priceLevel
	var found bool
	for price, level := range e.bids {
		if !found || price > bestPrice {
			bestPrice = price
			bestLevel = level
			found = true
		}
	}
	return bestPrice, bestLevel, found
}

func bestPrice(levels map[int64]*priceLevel, highest bool) int64 {
	if len(levels) == 0 {
		return 0
	}
	var best int64
	for price := range levels {
		if highest {
			if price > best {
				best = price
			}
		} else if best == 0 || price < best {
			best = price
		}
	}
	return best
}
