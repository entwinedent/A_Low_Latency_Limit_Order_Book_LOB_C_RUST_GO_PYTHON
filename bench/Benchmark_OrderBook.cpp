#include <benchmark/benchmark.h>
#include "lob/OrderBook.h"
#include "lob/Common.h"
#include <vector>
#include <random>

using namespace lob;

// Benchmark fixture for order book operations
class OrderBookBenchmark : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override {
        book = std::make_unique<OrderBook>();
    }
    
    void TearDown(const ::benchmark::State& state) override {
        book.reset();
    }
    
    std::unique_ptr<OrderBook> book;
};

// Benchmark: Add buy order
BENCHMARK_F(OrderBookBenchmark, AddBuyOrder)(benchmark::State& state) {
    uint64_t id = 1;
    for (auto _ : state) {
        book->add_limit_order(id++, 1000000, 10, Side::BUY);  // 100.00 in fixed-point
    }
    state.SetItemsProcessed(state.iterations());
}

// Benchmark: Add sell order
BENCHMARK_F(OrderBookBenchmark, AddSellOrder)(benchmark::State& state) {
    uint64_t id = 1;
    for (auto _ : state) {
        book->add_limit_order(id++, 1000000, 10, Side::SELL);  // 100.00 in fixed-point
    }
    state.SetItemsProcessed(state.iterations());
}

// Benchmark: Cancel order
BENCHMARK_F(OrderBookBenchmark, CancelOrder)(benchmark::State& state) {
    // Pre-populate the book
    std::vector<uint64_t> order_ids;
    for (uint64_t i = 1; i <= 1000; ++i) {
        book->add_limit_order(i, 1000000 + (i * 100), 10, Side::BUY);  // 100.00 + i*0.01
        order_ids.push_back(i);
    }
    
    size_t idx = 0;
    for (auto _ : state) {
        book->cancel_order(order_ids[idx % order_ids.size()]);
        idx++;
    }
    state.SetItemsProcessed(state.iterations());
}

// Benchmark: Get best bid
BENCHMARK_F(OrderBookBenchmark, GetBestBid)(benchmark::State& state) {
    // Pre-populate the book
    for (uint64_t i = 1; i <= 100; ++i) {
        book->add_limit_order(i, 100.0 + i, 10, Side::BUY);
    }
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(book->get_best_bid());
    }
}

// Benchmark: Get best ask
BENCHMARK_F(OrderBookBenchmark, GetBestAsk)(benchmark::State& state) {
    // Pre-populate the book
    for (uint64_t i = 1; i <= 100; ++i) {
        book->add_limit_order(i, 100.0 + i, 10, Side::SELL);
    }
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(book->get_best_ask());
    }
}

// Benchmark: Matching orders
BENCHMARK_F(OrderBookBenchmark, MatchOrders)(benchmark::State& state) {
    // Add sell orders
    for (uint64_t i = 1; i <= 100; ++i) {
        book->add_limit_order(i, 100.0, 10, Side::SELL);
    }
    
    uint64_t buy_id = 1000;
    for (auto _ : state) {
        book->add_limit_order(buy_id++, 100.0, 5, Side::BUY);
    }
    state.SetItemsProcessed(state.iterations());
}

// Benchmark: Order insertion with price levels
BENCHMARK_F(OrderBookBenchmark, InsertMultiplePriceLevels)(benchmark::State& state) {
    uint64_t id = 1;
    for (auto _ : state) {
        double price = 100.0 + (id % 50);
        book->add_limit_order(id++, price, 10, Side::BUY);
    }
    state.SetItemsProcessed(state.iterations());
}

// Benchmark: High-frequency trading simulation
BENCHMARK_F(OrderBookBenchmark, HFTSimulation)(benchmark::State& state) {
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> price_dist(95.0, 105.0);
    std::uniform_int_distribution<uint32_t> qty_dist(1, 100);
    std::uniform_int_distribution<int> side_dist(0, 1);
    
    uint64_t id = 1;
    for (auto _ : state) {
        double price = price_dist(gen);
        uint32_t qty = qty_dist(gen);
        Side side = (side_dist(gen) == 0) ? Side::BUY : Side::SELL;
        
        book->add_limit_order(id++, price, qty, side);
        
        // Occasionally cancel an order
        if (id % 10 == 0 && id > 100) {
            book->cancel_order(id - 50);
        }
    }
    state.SetItemsProcessed(state.iterations());
}

// Benchmark: Memory pool allocation
BENCHMARK_F(OrderBookBenchmark, MemoryPoolAllocation)(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(book->pool_allocated());
        benchmark::DoNotOptimize(book->pool_free());
    }
}

// Benchmark: Order book depth query
BENCHMARK_F(OrderBookBenchmark, DepthQuery)(benchmark::State& state) {
    // Pre-populate the book
    for (uint64_t i = 1; i <= 100; ++i) {
        book->add_limit_order(i, 100.0 + i, 10, Side::BUY);
        book->add_limit_order(i + 1000, 100.0 + i, 10, Side::SELL);
    }
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(book->get_bid_depth());
        benchmark::DoNotOptimize(book->get_ask_depth());
    }
}

// Benchmark: Cross-market operations
BENCHMARK_F(OrderBookBenchmark, CrossMarketOperations)(benchmark::State& state) {
    // Build crossed market
    for (uint64_t i = 1; i <= 50; ++i) {
        book->add_limit_order(i, 100.0, 10, Side::BUY);
        book->add_limit_order(i + 1000, 99.0, 10, Side::SELL);
    }
    
    uint64_t id = 2000;
    for (auto _ : state) {
        // Each addition should trigger immediate matching
        book->add_limit_order(id++, 100.0, 5, Side::BUY);
    }
    state.SetItemsProcessed(state.iterations());
}

// Benchmark: Large order book operations
BENCHMARK_F(OrderBookBenchmark, LargeOrderBook)(benchmark::State& state) {
    // Build large order book
    for (uint64_t i = 1; i <= 10000; ++i) {
        double price = 100.0 + (i % 100);
        Side side = (i % 2 == 0) ? Side::BUY : Side::SELL;
        book->add_limit_order(i, price, 10, side);
    }
    
    uint64_t id = 20000;
    for (auto _ : state) {
        double price = 100.0 + (id % 100);
        Side side = (id % 2 == 0) ? Side::BUY : Side::SELL;
        book->add_limit_order(id++, price, 10, side);
    }
    state.SetItemsProcessed(state.iterations());
}

// Benchmark: Quantity at price query
BENCHMARK_F(OrderBookBenchmark, QuantityAtPrice)(benchmark::State& state) {
    // Pre-populate the book
    for (uint64_t i = 1; i <= 100; ++i) {
        book->add_limit_order(i, 100.0, 10, Side::BUY);
    }
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(book->get_quantity_at_price(100.0, Side::BUY));
    }
}

// Benchmark: Empty check
BENCHMARK_F(OrderBookBenchmark, IsEmptyCheck)(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(book->is_empty());
    }
}

// Benchmark: Multiple order books (scalability)
static void MultipleOrderBooks(benchmark::State& state) {
    const int num_books = 10;
    std::vector<std::unique_ptr<OrderBook>> books;
    
    for (int i = 0; i < num_books; ++i) {
        books.push_back(std::make_unique<OrderBook>());
    }
    
    uint64_t id = 1;
    for (auto _ : state) {
        int book_idx = id % num_books;
        books[book_idx]->add_limit_order(id++, 100.0, 10, Side::BUY);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(MultipleOrderBooks);

// Benchmark: Thread safety (single-threaded baseline)
static void SingleThreadedBaseline(benchmark::State& state) {
    OrderBook book;
    uint64_t id = 1;
    
    for (auto _ : state) {
        book.add_limit_order(id++, 100.0, 10, Side::BUY);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(SingleThreadedBaseline);

// Run the benchmarks
BENCHMARK_MAIN();
