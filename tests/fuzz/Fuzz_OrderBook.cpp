#include "lob/OrderBook.h"
#include "lob/Common.h"
#include <random>
#include <cstdint>

using namespace lob;

// Fuzz test for order book operations
// This test uses random sequences of operations to find edge cases
// and potential memory corruption issues.

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Skip small inputs
    if (size < 16) {
        return 0;
    }
    
    // Create order book
    OrderBook book;
    
    // Use the input data as a seed for random number generation
    uint64_t seed = *reinterpret_cast<const uint64_t*>(data);
    std::mt19937 gen(seed);
    
    std::uniform_int_distribution<uint64_t> id_dist(1, 10000);
    std::uniform_real_distribution<double> price_dist(50.0, 150.0);
    std::uniform_int_distribution<uint32_t> qty_dist(1, 1000);
    std::uniform_int_distribution<int> side_dist(0, 1);
    std::uniform_int_distribution<int> op_dist(0, 2); // 0: add, 1: cancel, 2: query
    
    // Perform random operations based on input size
    size_t num_ops = size / 4;
    
    for (size_t i = 0; i < num_ops; ++i) {
        int op = op_dist(gen);
        
        switch (op) {
            case 0: // Add order
            {
                uint64_t id = id_dist(gen);
                double price = price_dist(gen);
                uint32_t qty = qty_dist(gen);
                Side side = (side_dist(gen) == 0) ? Side::BUY : Side::SELL;
                
                book.add_limit_order(id, price, qty, side);
                break;
            }
            case 1: // Cancel order
            {
                uint64_t id = id_dist(gen);
                book.cancel_order(id);
                break;
            }
            case 2: // Query operations
            {
                book.get_best_bid();
                book.get_best_ask();
                book.get_bid_depth();
                book.get_ask_depth();
                book.is_empty();
                book.pool_allocated();
                book.pool_free();
                break;
            }
        }
    }
    
    return 0;
}

// Main function for standalone fuzz testing
// Compile with: -fsanitize=fuzzer,address,undefined
int main(int argc, char** argv) {
    // Simple manual fuzz test without libFuzzer
    OrderBook book;
    
    // Add various orders
    for (uint64_t i = 1; i <= 100; ++i) {
        double price = 100.0 + (i % 50);
        uint32_t qty = 10 + (i % 90);
        Side side = (i % 2 == 0) ? Side::BUY : Side::SELL;
        
        book.add_limit_order(i, price, qty, side);
    }
    
    // Cancel random orders
    for (uint64_t i = 1; i <= 50; ++i) {
        book.cancel_order(i * 2);
    }
    
    // Add more orders
    for (uint64_t i = 101; i <= 200; ++i) {
        double price = 100.0 + (i % 50);
        uint32_t qty = 10 + (i % 90);
        Side side = (i % 2 == 0) ? Side::BUY : Side::SELL;
        
        book.add_limit_order(i, price, qty, side);
    }
    
    // Query operations
    book.get_best_bid();
    book.get_best_ask();
    book.get_bid_depth();
    book.get_ask_depth();
    book.is_empty();
    book.pool_allocated();
    book.pool_free();
    
    return 0;
}
