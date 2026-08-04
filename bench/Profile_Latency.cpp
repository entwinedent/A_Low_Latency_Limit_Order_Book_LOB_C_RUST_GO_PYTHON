#include "lob/OrderBook.h"
#include "lob/Common.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <iomanip>

using namespace lob;

// High-resolution timer using chrono
class Timer {
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}
    
    double elapsed_ns() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::nano>(end - start_).count();
    }
    
    double elapsed_us() const {
        return elapsed_ns() / 1000.0;
    }
    
    void reset() {
        start_ = std::chrono::high_resolution_clock::now();
    }
    
private:
    std::chrono::high_resolution_clock::time_point start_;
};

// RDTSC-based cycle counter (x86/x64 only)
// Using __rdtscp for serialization-aware cycle counting
#if defined(__x86_64__) || defined(_M_X64)
#if defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(__rdtscp)
inline uint64_t rdtsc() {
    unsigned int aux;
    return __rdtscp(&aux);  // __rdtscp serializes CPU, preventing out-of-order execution
}
#else
inline uint64_t rdtsc() {
    unsigned int lo, hi, aux;
    __asm__ __volatile__ ("rdtscp" : "=a" (lo), "=d" (hi), "=c" (aux));
    return ((uint64_t)hi << 32) | lo;
}
#endif
#else
inline uint64_t rdtsc() {
    // Fallback for non-x86 platforms
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}
#endif

// Cycle counter class
class CycleCounter {
public:
    CycleCounter() : start_(rdtsc()) {}
    
    uint64_t elapsed() const {
        return rdtsc() - start_;
    }
    
    void reset() {
        start_ = rdtsc();
    }
    
private:
    uint64_t start_;
};

// Statistics calculator
class Statistics {
public:
    void add_sample(double value) {
        samples_.push_back(value);
    }
    
    double mean() const {
        if (samples_.empty()) return 0.0;
        double sum = 0.0;
        for (double s : samples_) sum += s;
        return sum / samples_.size();
    }
    
    double min() const {
        if (samples_.empty()) return 0.0;
        return *std::min_element(samples_.begin(), samples_.end());
    }
    
    double max() const {
        if (samples_.empty()) return 0.0;
        return *std::max_element(samples_.begin(), samples_.end());
    }
    
    double percentile(double p) const {
        if (samples_.empty()) return 0.0;
        std::vector<double> sorted = samples_;
        std::sort(sorted.begin(), sorted.end());
        
        size_t idx = static_cast<size_t>(p * sorted.size());
        if (idx >= sorted.size()) idx = sorted.size() - 1;
        return sorted[idx];
    }
    
    size_t count() const {
        return samples_.size();
    }
    
private:
    std::vector<double> samples_;
};

// Profiling functions
void profile_add_order(size_t iterations) {
    OrderBook book;
    Statistics stats;
    
    std::cout << "\n=== Profiling Add Order ===" << std::endl;
    
    for (size_t i = 0; i < iterations; ++i) {
        Timer timer;
        book.add_limit_order(i, 1000000 + ((i % 50) * 100), 10, Side::BUY);  // 100.00 + (i%50)*0.01
        stats.add_sample(timer.elapsed_ns());
    }
    
    std::cout << "Iterations: " << stats.count() << std::endl;
    std::cout << "Mean latency: " << std::fixed << std::setprecision(2) << stats.mean() << " ns" << std::endl;
    std::cout << "Min latency: " << stats.min() << " ns" << std::endl;
    std::cout << "Max latency: " << stats.max() << " ns" << std::endl;
    std::cout << "P50 latency: " << stats.percentile(0.50) << " ns" << std::endl;
    std::cout << "P90 latency: " << stats.percentile(0.90) << " ns" << std::endl;
    std::cout << "P95 latency: " << stats.percentile(0.95) << " ns" << std::endl;
    std::cout << "P99 latency: " << stats.percentile(0.99) << " ns" << std::endl;
    std::cout << "P99.9 latency: " << stats.percentile(0.999) << " ns" << std::endl;
}

void profile_cancel_order(size_t iterations) {
    OrderBook book;
    Statistics stats;
    
    // Pre-populate the book
    std::vector<uint64_t> order_ids;
    for (size_t i = 0; i < iterations; ++i) {
        book.add_limit_order(i, 1000000 + ((i % 50) * 100), 10, Side::BUY);  // 100.00 + (i%50)*0.01
        order_ids.push_back(i);
    }
    
    std::cout << "\n=== Profiling Cancel Order ===" << std::endl;
    
    for (size_t i = 0; i < iterations; ++i) {
        Timer timer;
        book.cancel_order(order_ids[i]);
        stats.add_sample(timer.elapsed_ns());
    }
    
    std::cout << "Iterations: " << stats.count() << std::endl;
    std::cout << "Mean latency: " << std::fixed << std::setprecision(2) << stats.mean() << " ns" << std::endl;
    std::cout << "Min latency: " << stats.min() << " ns" << std::endl;
    std::cout << "Max latency: " << stats.max() << " ns" << std::endl;
    std::cout << "P50 latency: " << stats.percentile(0.50) << " ns" << std::endl;
    std::cout << "P90 latency: " << stats.percentile(0.90) << " ns" << std::endl;
    std::cout << "P95 latency: " << stats.percentile(0.95) << " ns" << std::endl;
    std::cout << "P99 latency: " << stats.percentile(0.99) << " ns" << std::endl;
    std::cout << "P99.9 latency: " << stats.percentile(0.999) << " ns" << std::endl;
}

void profile_match_order(size_t iterations) {
    OrderBook book;
    Statistics stats;
    
    // Pre-populate with sell orders
    for (size_t i = 0; i < iterations; ++i) {
        book.add_limit_order(i, 1000000, 10, Side::SELL);  // 100.00
    }
    
    std::cout << "\n=== Profiling Match Order ===" << std::endl;
    
    for (size_t i = 0; i < iterations; ++i) {
        Timer timer;
        book.add_limit_order(iterations + i, 1000000, 5, Side::BUY);  // 100.00
        stats.add_sample(timer.elapsed_ns());
    }
    
    std::cout << "Iterations: " << stats.count() << std::endl;
    std::cout << "Mean latency: " << std::fixed << std::setprecision(2) << stats.mean() << " ns" << std::endl;
    std::cout << "Min latency: " << stats.min() << " ns" << std::endl;
    std::cout << "Max latency: " << stats.max() << " ns" << std::endl;
    std::cout << "P50 latency: " << stats.percentile(0.50) << " ns" << std::endl;
    std::cout << "P90 latency: " << stats.percentile(0.90) << " ns" << std::endl;
    std::cout << "P95 latency: " << stats.percentile(0.95) << " ns" << std::endl;
    std::cout << "P99 latency: " << stats.percentile(0.99) << " ns" << std::endl;
    std::cout << "P99.9 latency: " << stats.percentile(0.999) << " ns" << std::endl;
}

void profile_query_operations(size_t iterations) {
    OrderBook book;
    Statistics bid_stats, ask_stats, depth_stats;
    
    // Pre-populate the book
    for (size_t i = 0; i < 100; ++i) {
        book.add_limit_order(i, 1000000 + (i * 100), 10, Side::BUY);  // 100.00 + i*0.01
        book.add_limit_order(i + 1000, 1000000 + (i * 100), 10, Side::SELL);  // 100.00 + i*0.01
    }
    
    std::cout << "\n=== Profiling Query Operations ===" << std::endl;
    
    for (size_t i = 0; i < iterations; ++i) {
        Timer timer;
        book.get_best_bid();
        bid_stats.add_sample(timer.elapsed_ns());
        
        timer.reset();
        book.get_best_ask();
        ask_stats.add_sample(timer.elapsed_ns());
        
        timer.reset();
        book.get_bid_depth();
        book.get_ask_depth();
        depth_stats.add_sample(timer.elapsed_ns());
    }
    
    std::cout << "Best Bid - Mean: " << std::fixed << std::setprecision(2) << bid_stats.mean() << " ns" << std::endl;
    std::cout << "Best Ask - Mean: " << ask_stats.mean() << " ns" << std::endl;
    std::cout << "Depth Query - Mean: " << depth_stats.mean() << " ns" << std::endl;
}

void profile_hft_simulation(size_t iterations) {
    OrderBook book;
    Statistics stats;
    
    std::mt19937 gen(42);
    std::uniform_int_distribution<int64_t> price_dist(950000, 1050000);  // 95.00 to 105.00
    std::uniform_int_distribution<uint32_t> qty_dist(1, 100);
    std::uniform_int_distribution<int> side_dist(0, 1);
    
    std::cout << "\n=== Profiling HFT Simulation ===" << std::endl;
    
    uint64_t id = 1;
    for (size_t i = 0; i < iterations; ++i) {
        Timer timer;
        
        int64_t price = price_dist(gen);
        uint32_t qty = qty_dist(gen);
        Side side = (side_dist(gen) == 0) ? Side::BUY : Side::SELL;
        
        book.add_limit_order(id++, price, qty, side);
        
        // Occasionally cancel an order
        if (id % 10 == 0 && id > 100) {
            book.cancel_order(id - 50);
        }
        
        stats.add_sample(timer.elapsed_ns());
    }
    
    std::cout << "Iterations: " << stats.count() << std::endl;
    std::cout << "Mean latency: " << std::fixed << std::setprecision(2) << stats.mean() << " ns" << std::endl;
    std::cout << "Min latency: " << stats.min() << " ns" << std::endl;
    std::cout << "Max latency: " << stats.max() << " ns" << std::endl;
    std::cout << "P50 latency: " << stats.percentile(0.50) << " ns" << std::endl;
    std::cout << "P90 latency: " << stats.percentile(0.90) << " ns" << std::endl;
    std::cout << "P95 latency: " << stats.percentile(0.95) << " ns" << std::endl;
    std::cout << "P99 latency: " << stats.percentile(0.99) << " ns" << std::endl;
    std::cout << "P99.9 latency: " << stats.percentile(0.999) << " ns" << std::endl;
}

void profile_memory_pool(size_t iterations) {
    OrderBook book;
    
    std::cout << "\n=== Memory Pool Statistics ===" << std::endl;
    
    // Add orders
    for (size_t i = 0; i < iterations; ++i) {
        book.add_limit_order(i, 1000000 + ((i % 50) * 100), 10, Side::BUY);  // 100.00 + (i%50)*0.01
    }
    
    std::cout << "Pool Capacity: " << book.pool_capacity() << std::endl;
    std::cout << "Pool Allocated: " << book.pool_allocated() << std::endl;
    std::cout << "Pool Free: " << book.pool_free() << std::endl;
    std::cout << "Utilization: " << std::fixed << std::setprecision(2) 
              << (100.0 * book.pool_allocated() / book.pool_capacity()) << "%" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Order Book Engine Latency Profiler" << std::endl;
    std::cout << "========================================" << std::endl;
    
    const size_t iterations = 100000;
    
    profile_add_order(iterations);
    profile_cancel_order(iterations);
    profile_match_order(iterations);
    profile_query_operations(iterations);
    profile_hft_simulation(iterations);
    profile_memory_pool(iterations);
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Profiling Complete" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
