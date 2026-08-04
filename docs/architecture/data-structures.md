# Data Structures Architecture

## Overview

The order book engine uses carefully chosen data structures optimized for cache locality and algorithmic efficiency.

## Core Data Structures

### 1. Memory Pool

**Purpose**: Zero-allocation order management

**Implementation**: 
- Contiguous aligned memory block
- Free list stack for O(1) allocation
- Placement new for object construction

**Complexity**:
- Allocate: O(1)
- Deallocate: O(1)
- Memory: O(N) where N is capacity

### 2. Intrusive List

**Purpose**: O(1) order insertion/removal within price levels

**Implementation**:
- Doubly-linked list with embedded pointers
- No separate allocation for list nodes
- Cache-friendly sequential access

**Complexity**:
- Push front/back: O(1)
- Remove: O(1)
- Memory: O(1) per order (embedded in Order struct)

### 3. Price Level Map

**Purpose**: Sorted price levels for bid/ask sides

**Implementation**:
- `std::map<Price, PriceLevel, std::greater<Price>>` for bids (descending)
- `std::map<Price, PriceLevel, std::less<Price>>` for asks (ascending)
- Red-black tree with O(log N) operations

**Complexity**:
- Insert: O(log N)
- Remove: O(log N)
- Find best: O(log N) (can be O(1) with caching)
- Memory: O(P) where P is unique price levels

### 4. Order Hash Map

**Purpose**: O(1) order lookup for cancellation

**Implementation**:
- `std::unordered_map<OrderID, Order*>`
- Chained hash table with O(1) average case
- Custom hash function for OrderID

**Complexity**:
- Insert: O(1) average
- Find: O(1) average
- Remove: O(1) average
- Memory: O(N) where N is active orders

## Data Layout

### Order Structure

```cpp
struct alignas(64) Order {
    OrderID id;              // 8 bytes
    Price price;             // 8 bytes
    Quantity quantity;       // 4 bytes
    Side side;               // 1 byte
    Timestamp timestamp;     // 8 bytes
    Order* next;             // 8 bytes
    Order* prev;             // 8 bytes
    // Total: 45 bytes + padding to 64 bytes
};
```

**Alignment**: 64-byte cache line alignment prevents false sharing

### Price Level Structure

```cpp
struct PriceLevel {
    Price price;              // 8 bytes
    IntrusiveList orders;     // 16 bytes (head/tail pointers)
    Quantity total_quantity;  // 4 bytes
    // Total: 28 bytes
};
```

## Cache Optimization

### Cache Line Alignment

- **Order**: 64-byte aligned (fits in one cache line)
- **Memory Pool**: Contiguous block for spatial locality
- **Free List**: Stack pattern for temporal locality

### Prefetching

Hardware prefetch hints for predictable access:

```cpp
// Prefetch next order in list
_mm_prefetch((const char*)order->next, _MM_HINT_T0);
```

### False Sharing Prevention

All frequently accessed structures are cache-line aligned:
- Order structures
- Memory pool metadata
- Statistics counters

## Future Optimizations

### Flat Maps

Replace `std::map` with `std::flat_map` (C++23):

- **Advantage**: Better cache locality (contiguous storage)
- **Trade-off**: O(N) insertion, O(log N) lookup
- **Use Case**: Static price levels, infrequent updates

### B-Tree Maps

Consider `std::map` alternatives:

- **absl::btree_map**: Better cache locality than red-black tree
- **robin_hood::unordered_flat_map**: Open addressing for hash map
- **ankerl::nanobench**: Benchmark to determine best fit

### SIMD Vectorization

Vectorize price comparisons:

```cpp
// Compare 8 prices simultaneously using AVX2
__m256d prices = _mm256_load_pd(price_array);
__m256d target = _mm256_set1_pd(target_price);
__m256d cmp = _mm256_cmp_pd(prices, target, _MM_CMPINT_LT);
```

## Memory Footprint

### Per-Order Memory

- **Order struct**: 64 bytes (aligned)
- **Hash map entry**: ~16 bytes
- **Total**: ~80 bytes per order

### For 1M Orders

- **Order pool**: 64 MB
- **Hash map**: ~16 MB
- **Price levels**: Variable (depends on unique prices)
- **Total**: ~80-100 MB

## Trade-offs

### std::map vs std::unordered_map

**std::map** (used for price levels):
- Sorted iteration
- O(log N) worst case
- Better cache locality than hash table

**std::unordered_map** (used for order lookup):
- O(1) average case
- No sorting needed
- Hash collision overhead

### Intrusive List vs std::list

**Intrusive List** (used):
- No separate allocation
- Better cache locality
- O(1) operations

**std::list** (not used):
- Separate node allocation
- Pointer indirection overhead
- Same O(1) complexity
