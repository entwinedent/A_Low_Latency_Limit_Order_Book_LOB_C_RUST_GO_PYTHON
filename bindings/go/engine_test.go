package engine

import (
	"testing"
)

func TestNewEngine(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	if engine == nil {
		t.Fatal("Expected non-nil engine")
	}

	if engine.IsEmpty() != true {
		t.Error("Expected empty order book initially")
	}
}

func TestAddLimitOrder(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	// Add a buy order
	err := engine.AddLimitOrder(1, 100.0, 10, SideBuy)
	if err != ErrorSuccess {
		t.Errorf("Expected SUCCESS, got %s", err)
	}

	// Add a sell order
	err = engine.AddLimitOrder(2, 105.0, 5, SideSell)
	if err != ErrorSuccess {
		t.Errorf("Expected SUCCESS, got %s", err)
	}

	// Check bid depth
	if engine.GetBidDepth() != 1 {
		t.Errorf("Expected bid depth 1, got %d", engine.GetBidDepth())
	}

	// Check ask depth
	if engine.GetAskDepth() != 1 {
		t.Errorf("Expected ask depth 1, got %d", engine.GetAskDepth())
	}
}

func TestTradeCallback(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	trades := make(chan Trade, 1)
	engine.SetTradeCallback(func(trade Trade) {
		trades <- trade
	})

	if err := engine.AddLimitOrder(1, 100.0, 10, SideSell); err != ErrorSuccess {
		t.Fatalf("expected SUCCESS, got %s", err)
	}
	if err := engine.AddLimitOrder(2, 100.0, 5, SideBuy); err != ErrorSuccess {
		t.Fatalf("expected SUCCESS, got %s", err)
	}

	select {
	case trade := <-trades:
		if trade.MakerOrderID != 1 || trade.TakerOrderID != 2 || trade.Quantity != 5 || trade.Price != 100.0 {
			t.Fatalf("unexpected trade payload: %+v", trade)
		}
	default:
		t.Fatal("expected a trade callback to be invoked")
	}
}

func TestGetBestBidAsk(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	// Add orders
	engine.AddLimitOrder(1, 100.0, 10, SideBuy)
	engine.AddLimitOrder(2, 99.0, 5, SideBuy)
	engine.AddLimitOrder(3, 105.0, 5, SideSell)
	engine.AddLimitOrder(4, 106.0, 10, SideSell)

	// Best bid should be 100.0 (highest buy)
	bestBid := engine.GetBestBid()
	if bestBid != 100.0 {
		t.Errorf("Expected best bid 100.0, got %f", bestBid)
	}

	// Best ask should be 105.0 (lowest sell)
	bestAsk := engine.GetBestAsk()
	if bestAsk != 105.0 {
		t.Errorf("Expected best ask 105.0, got %f", bestAsk)
	}
}

func TestCancelOrder(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	// Add an order
	engine.AddLimitOrder(1, 100.0, 10, SideBuy)

	// Cancel the order
	err := engine.CancelOrder(1)
	if err != ErrorSuccess {
		t.Errorf("Expected SUCCESS, got %s", err)
	}

	// Check if book is empty
	if !engine.IsEmpty() {
		t.Error("Expected empty order book after cancellation")
	}
}

func TestCancelNonExistentOrder(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	// Try to cancel non-existent order
	err := engine.CancelOrder(999)
	if err != ErrorOrderNotFound {
		t.Errorf("Expected ORDER_NOT_FOUND, got %s", err)
	}
}

func TestInvalidOrder(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	// Test invalid quantity
	err := engine.AddLimitOrder(1, 100.0, 0, SideBuy)
	if err != ErrorInvalidQuantity {
		t.Errorf("Expected INVALID_QUANTITY, got %s", err)
	}

	// Test invalid price
	err = engine.AddLimitOrder(2, -1.0, 10, SideBuy)
	if err != ErrorInvalidPrice {
		t.Errorf("Expected INVALID_PRICE, got %s", err)
	}

	// Test invalid side
	err = engine.AddLimitOrder(3, 100.0, 10, Side(99))
	if err != ErrorInvalidSide {
		t.Errorf("Expected INVALID_SIDE, got %s", err)
	}
}

func TestAddLimitOrderOnClosedEngine(t *testing.T) {
	engine := NewEngine()
	engine.Close()

	if err := engine.AddLimitOrder(1, 100.0, 10, SideBuy); err != ErrorUnknown {
		t.Fatalf("expected UNKNOWN when adding order after Close, got %s", err)
	}
}

func TestDuplicateOrderID(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	if err := engine.AddLimitOrder(1, 100.0, 10, SideBuy); err != ErrorSuccess {
		t.Fatalf("expected SUCCESS, got %s", err)
	}
	if err := engine.AddLimitOrder(1, 101.0, 5, SideBuy); err != ErrorInvalidOrderID {
		t.Fatalf("expected INVALID_ORDER_ID for duplicate ID, got %s", err)
	}
}

func TestGetBestBidAskEmptyBook(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	if got := engine.GetBestBid(); got != 0 {
		t.Fatalf("expected best bid 0 for empty book, got %f", got)
	}
	if got := engine.GetBestAsk(); got != 0 {
		t.Fatalf("expected best ask 0 for empty book, got %f", got)
	}
}

func TestRemoveOrderFromBookWithMissingLevel(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	engine.orders = make(map[uint64]*order)
	engine.bids = make(map[float64]*priceLevel)
	engine.asks = make(map[float64]*priceLevel)

	engine.removeOrderFromBook(&order{id: 1, price: 100.0, quantity: 10, side: SideBuy})

	if got := engine.GetQuantityAtPrice(100.0, SideBuy); got != 0 {
		t.Fatalf("expected 0 quantity for missing bid level, got %d", got)
	}
}

func TestPoolStatistics(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	if engine.PoolCapacity() != poolCapacity {
		t.Errorf("Expected pool capacity %d, got %d", poolCapacity, engine.PoolCapacity())
	}

	// Initially, pool should be empty
	if engine.PoolAllocated() != 0 {
		t.Errorf("Expected 0 allocated, got %d", engine.PoolAllocated())
	}
	if engine.PoolFree() != poolCapacity {
		t.Errorf("Expected pool free %d, got %d", poolCapacity, engine.PoolFree())
	}

	// Add some orders
	for i := uint64(1); i <= 10; i++ {
		engine.AddLimitOrder(i, 100.0+float64(i), 10, SideBuy)
	}

	// Check allocated count
	allocated := engine.PoolAllocated()
	if allocated != 10 {
		t.Errorf("Expected 10 allocated, got %d", allocated)
	}
	if engine.PoolFree() != poolCapacity-10 {
		t.Errorf("Expected pool free %d, got %d", poolCapacity-10, engine.PoolFree())
	}

	// Cancel half the orders
	for i := uint64(1); i <= 5; i++ {
		engine.CancelOrder(i)
	}

	// Allocated should now be 5 (remaining orders)
	allocated = engine.PoolAllocated()
	if allocated != 5 {
		t.Errorf("Expected 5 allocated after cancellation, got %d", allocated)
	}
}

func TestOrderMatching(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	// Add a sell order
	engine.AddLimitOrder(1, 100.0, 10, SideSell)

	// Add a buy order that matches
	err := engine.AddLimitOrder(2, 100.0, 5, SideBuy)
	if err != ErrorSuccess {
		t.Errorf("Expected SUCCESS, got %s", err)
	}

	// The buy order should be fully matched (no remaining)
	// The sell order should have 5 remaining
	quantity := engine.GetQuantityAtPrice(100.0, SideSell)
	if quantity != 5 {
		t.Errorf("Expected 5 remaining at price 100.0, got %d", quantity)
	}
}

func TestMatchingAgainstExistingBids(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	if err := engine.AddLimitOrder(1, 100.0, 10, SideBuy); err != ErrorSuccess {
		t.Fatalf("expected SUCCESS, got %s", err)
	}
	if err := engine.AddLimitOrder(2, 100.0, 5, SideSell); err != ErrorSuccess {
		t.Fatalf("expected SUCCESS, got %s", err)
	}

	if quantity := engine.GetQuantityAtPrice(100.0, SideBuy); quantity != 5 {
		t.Fatalf("expected remaining buy quantity 5, got %d", quantity)
	}
	if quantity := engine.GetQuantityAtPrice(100.0, SideSell); quantity != 0 {
		t.Fatalf("expected no remaining sell quantity, got %d", quantity)
	}
}

func TestPartialFillBuyOrder(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	trades := make([]Trade, 0, 2)
	engine.SetTradeCallback(func(trade Trade) {
		trades = append(trades, trade)
	})

	if err := engine.AddLimitOrder(1, 100.0, 5, SideSell); err != ErrorSuccess {
		t.Fatalf("expected SUCCESS, got %s", err)
	}
	if err := engine.AddLimitOrder(2, 100.0, 5, SideSell); err != ErrorSuccess {
		t.Fatalf("expected SUCCESS, got %s", err)
	}

	if err := engine.AddLimitOrder(3, 100.0, 8, SideBuy); err != ErrorSuccess {
		t.Fatalf("expected SUCCESS, got %s", err)
	}

	if len(trades) != 2 {
		t.Fatalf("expected 2 trades, got %d", len(trades))
	}
	if remaining := engine.GetQuantityAtPrice(100.0, SideSell); remaining != 2 {
		t.Fatalf("expected 2 remaining sell quantity, got %d", remaining)
	}
}

func TestPartialFillSellOrder(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	trades := make([]Trade, 0, 2)
	engine.SetTradeCallback(func(trade Trade) {
		trades = append(trades, trade)
	})

	if err := engine.AddLimitOrder(1, 100.0, 5, SideBuy); err != ErrorSuccess {
		t.Fatalf("expected SUCCESS, got %s", err)
	}
	if err := engine.AddLimitOrder(2, 100.0, 5, SideBuy); err != ErrorSuccess {
		t.Fatalf("expected SUCCESS, got %s", err)
	}

	if err := engine.AddLimitOrder(3, 100.0, 8, SideSell); err != ErrorSuccess {
		t.Fatalf("expected SUCCESS, got %s", err)
	}

	if len(trades) != 2 {
		t.Fatalf("expected 2 trades, got %d", len(trades))
	}
	if remaining := engine.GetQuantityAtPrice(100.0, SideBuy); remaining != 2 {
		t.Fatalf("expected 2 remaining buy quantity, got %d", remaining)
	}
}

func TestRemoveOrderFromBookRetainsLevel(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	if err := engine.AddLimitOrder(1, 100.0, 5, SideBuy); err != ErrorSuccess {
		t.Fatalf("expected SUCCESS, got %s", err)
	}
	if err := engine.AddLimitOrder(2, 100.0, 5, SideBuy); err != ErrorSuccess {
		t.Fatalf("expected SUCCESS, got %s", err)
	}

	if err := engine.CancelOrder(1); err != ErrorSuccess {
		t.Fatalf("expected SUCCESS, got %s", err)
	}

	if depth := engine.GetBidDepth(); depth != 1 {
		t.Fatalf("expected bid depth 1, got %d", depth)
	}
	if quantity := engine.GetQuantityAtPrice(100.0, SideBuy); quantity != 5 {
		t.Fatalf("expected remaining buy quantity 5, got %d", quantity)
	}
}

func TestRemoveOrderFromBookNonMatchingOrder(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	engine.orders = make(map[uint64]*order)
	engine.bids = make(map[float64]*priceLevel)
	engine.asks = make(map[float64]*priceLevel)

	engine.addOrderToBook(&order{id: 1, price: 100.0, quantity: 10, side: SideBuy})
	engine.removeOrderFromBook(&order{id: 2, price: 100.0, quantity: 5, side: SideBuy})

	if quantity := engine.GetQuantityAtPrice(100.0, SideBuy); quantity != 10 {
		t.Fatalf("expected unmatched order to remain at quantity 10, got %d", quantity)
	}
}

func TestNilReceiverBranches(t *testing.T) {
	var engine *Engine

	engine.Close()
	engine.SetTradeCallback(func(trade Trade) {})
	if err := engine.AddLimitOrder(1, 100.0, 10, SideBuy); err != ErrorInvalidOrderID {
		t.Fatalf("expected INVALID_ORDER_ID, got %s", err)
	}
	if err := engine.CancelOrder(1); err != ErrorOrderNotFound {
		t.Fatalf("expected ORDER_NOT_FOUND, got %s", err)
	}
	if got := engine.GetBestBid(); got != 0 {
		t.Fatalf("expected best bid 0, got %f", got)
	}
	if got := engine.GetBestAsk(); got != 0 {
		t.Fatalf("expected best ask 0, got %f", got)
	}
	if got := engine.GetQuantityAtPrice(100.0, SideBuy); got != 0 {
		t.Fatalf("expected quantity 0, got %d", got)
	}
	if got := engine.GetBidDepth(); got != 0 {
		t.Fatalf("expected bid depth 0, got %d", got)
	}
	if got := engine.GetAskDepth(); got != 0 {
		t.Fatalf("expected ask depth 0, got %d", got)
	}
	if got := engine.IsEmpty(); got != true {
		t.Fatalf("expected empty book for nil engine, got %t", got)
	}
	if got := engine.PoolCapacity(); got != 0 {
		t.Fatalf("expected pool capacity 0, got %d", got)
	}
	if got := engine.PoolAllocated(); got != 0 {
		t.Fatalf("expected pool allocated 0, got %d", got)
	}
	if got := engine.PoolFree(); got != 0 {
		t.Fatalf("expected pool free 0, got %d", got)
	}
	if got := engine.GetHandle(); got != nil {
		t.Fatalf("expected nil handle, got %v", got)
	}
}

func TestNewEngineFromHandle(t *testing.T) {
	engine := NewEngine()
	defer engine.Close()

	fromHandle := NewEngineFromHandle(engine.GetHandle())
	if fromHandle == nil {
		t.Fatal("expected engine from handle")
	}
	defer fromHandle.Close()

	if fromHandle.IsEmpty() != engine.IsEmpty() {
		t.Fatalf("expected matching emptiness state: got %v want %v", fromHandle.IsEmpty(), engine.IsEmpty())
	}
}

func TestNilHandleRoundTrip(t *testing.T) {
	if got := NewEngineFromHandle(nil); got != nil {
		t.Fatalf("expected nil engine for nil handle, got %#v", got)
	}
}

func TestErrorString(t *testing.T) {
	tests := []struct {
		error    ErrorCode
		expected string
	}{
		{ErrorSuccess, "SUCCESS"},
		{ErrorInvalidOrderID, "INVALID_ORDER_ID"},
		{ErrorInvalidPrice, "INVALID_PRICE"},
		{ErrorInvalidQuantity, "INVALID_QUANTITY"},
		{ErrorInvalidSide, "INVALID_SIDE"},
		{ErrorOrderNotFound, "ORDER_NOT_FOUND"},
		{ErrorPoolExhausted, "POOL_EXHAUSTED"},
		{ErrorUnknown, "UNKNOWN(-99)"},
	}

	for _, tt := range tests {
		if got := tt.error.String(); got != tt.expected {
			t.Errorf("ErrorString(%d) = %s, want %s", tt.error, got, tt.expected)
		}
	}
}
