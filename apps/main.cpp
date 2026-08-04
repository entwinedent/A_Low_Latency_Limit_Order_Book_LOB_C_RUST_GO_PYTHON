#include "lob/OrderBook.h"
#include "lob/Common.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>

using namespace lob;

void print_help() {
    std::cout << "\n=== Order Book Engine CLI ===\n";
    std::cout << "Commands:\n";
    std::cout << "  add <id> <price> <qty> <side>  - Add limit order (side: buy/sell)\n";
    std::cout << "  cancel <id>                      - Cancel order\n";
    std::cout << "  bid                              - Get best bid\n";
    std::cout << "  ask                              - Get best ask\n";
    std::cout << "  depth                            - Get bid/ask depth\n";
    std::cout << "  qty <price> <side>               - Get quantity at price\n";
    std::cout << "  pool                             - Show memory pool stats\n";
    std::cout << "  snapshot                         - Print order book snapshot\n";
    std::cout << "  benchmark <n>                   - Run quick benchmark\n";
    std::cout << "  help                             - Show this help\n";
    std::cout << "  quit                             - Exit\n";
    std::cout << "===============================\n\n";
}

void print_snapshot(OrderBook& book) {
    std::cout << "\n=== Order Book Snapshot ===\n";
    std::cout << "Best Bid: " << std::fixed << std::setprecision(4) << price_to_double(book.get_best_bid()) << "\n";
    std::cout << "Best Ask: " << price_to_double(book.get_best_ask()) << "\n";
    std::cout << "Bid Depth: " << book.get_bid_depth() << "\n";
    std::cout << "Ask Depth: " << book.get_ask_depth() << "\n";
    std::cout << "Spread: " << price_to_double(book.get_best_ask() - book.get_best_bid()) << "\n";
    std::cout << "Pool Usage: " << book.pool_allocated() << "/" << book.pool_capacity() << "\n";
    std::cout << "========================\n\n";
}

void run_benchmark(OrderBook& book, int n) {
    std::cout << "\n=== Running Benchmark (" << n << " operations) ===\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Add orders
    for (int i = 0; i < n; ++i) {
        double price = 100.0 + (i % 50);
        Side side = (i % 2 == 0) ? Side::BUY : Side::SELL;
        book.add_limit_order(i, price_to_fixed(price), 10, side);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Added " << n << " orders in " << duration.count() << " μs\n";
    std::cout << "Average: " << (duration.count() * 1000.0 / n) << " ns per order\n";
    std::cout << "Orders per second: " << (n * 1000000.0 / duration.count()) << "\n";
    std::cout << "========================\n\n";
}

int main() {
    OrderBook book;
    
    // Set up trade callback
    book.set_trade_callback([](const Trade& trade) {
        std::cout << "TRADE: " << trade << "\n";
    });
    
    print_help();
    
    std::string line;
    while (true) {
        std::cout << "lob> ";
        std::getline(std::cin, line);
        
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        
        if (command == "quit" || command == "exit") {
            break;
        } else if (command == "help") {
            print_help();
        } else if (command == "add") {
            uint64_t id;
            double price;
            uint32_t qty;
            std::string side_str;
            
            if (iss >> id >> price >> qty >> side_str) {
                Side side = (side_str == "buy" || side_str == "BUY") ? Side::BUY : Side::SELL;
                auto err = book.add_limit_order(id, price_to_fixed(price), qty, side);
                
                if (err == ErrorCode::SUCCESS) {
                    std::cout << "Order added successfully\n";
                } else {
                    std::cout << "Error: " << error_to_string(err) << "\n";
                }
            } else {
                std::cout << "Invalid command format. Usage: add <id> <price> <qty> <side>\n";
            }
        } else if (command == "cancel") {
            uint64_t id;
            if (iss >> id) {
                auto err = book.cancel_order(id);
                
                if (err == ErrorCode::SUCCESS) {
                    std::cout << "Order cancelled successfully\n";
                } else {
                    std::cout << "Error: " << error_to_string(err) << "\n";
                }
            } else {
                std::cout << "Invalid command format. Usage: cancel <id>\n";
            }
        } else if (command == "bid") {
            std::cout << "Best Bid: " << std::fixed << std::setprecision(4) << price_to_double(book.get_best_bid()) << "\n";
        } else if (command == "ask") {
            std::cout << "Best Ask: " << std::fixed << std::setprecision(4) << price_to_double(book.get_best_ask()) << "\n";
        } else if (command == "depth") {
            std::cout << "Bid Depth: " << book.get_bid_depth() << "\n";
            std::cout << "Ask Depth: " << book.get_ask_depth() << "\n";
        } else if (command == "qty") {
            double price;
            std::string side_str;
            
            if (iss >> price >> side_str) {
                Side side = (side_str == "buy" || side_str == "BUY") ? Side::BUY : Side::SELL;
                uint32_t qty = book.get_quantity_at_price(price_to_fixed(price), side);
                std::cout << "Quantity at " << std::fixed << std::setprecision(4) << price << " (" << side_to_string(side) << "): " << qty << "\n";
            } else {
                std::cout << "Invalid command format. Usage: qty <price> <side>\n";
            }
        } else if (command == "pool") {
            std::cout << "Pool Capacity: " << book.pool_capacity() << "\n";
            std::cout << "Pool Allocated: " << book.pool_allocated() << "\n";
            std::cout << "Pool Free: " << book.pool_free() << "\n";
        } else if (command == "snapshot") {
            print_snapshot(book);
        } else if (command == "benchmark") {
            int n;
            if (iss >> n) {
                run_benchmark(book, n);
            } else {
                std::cout << "Invalid command format. Usage: benchmark <n>\n";
            }
        } else {
            std::cout << "Unknown command. Type 'help' for available commands.\n";
        }
    }
    
    std::cout << "Goodbye!\n";
    return 0;
}
