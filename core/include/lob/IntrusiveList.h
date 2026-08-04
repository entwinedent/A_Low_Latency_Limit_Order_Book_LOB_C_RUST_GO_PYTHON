#pragma once

#include "Order.h"
#include <cstdint>

namespace lob {

// Intrusive doubly-linked list for O(1) order management
// The list structure is embedded within the Order struct itself
class IntrusiveList {
public:
    IntrusiveList() : head_(nullptr), tail_(nullptr), size_(0) {}
    
    // Insert at the end of the list (FIFO ordering)
    void push_back(Order* order) {
        if (!head_) [[unlikely]] {
            head_ = tail_ = order;
            order->prev = nullptr;
            order->next = nullptr;
        } else [[likely]] {
            tail_->next = order;
            order->prev = tail_;
            order->next = nullptr;
            tail_ = order;
        }
        ++size_;
    }
    
    // Remove an order from the list
    void remove(Order* order) {
        if (!order) [[unlikely]] {
            return;
        }
        
        // Check if order is actually in this list
        // For intrusive lists, we need to verify the order is linked
        bool was_linked = (order->prev != nullptr || order->next != nullptr || head_ == order);
        
        if (order->prev) [[likely]] {
            order->prev->next = order->next;
        } else {
            head_ = order->next;
        }
        
        if (order->next) [[likely]] {
            order->next->prev = order->prev;
        } else {
            tail_ = order->prev;
        }
        
        order->next = nullptr;
        order->prev = nullptr;
        
        // Only decrement size if the order was actually in the list
        if (was_linked) {
            --size_;
        }
    }
    
    // Get the first order in the list
    Order* front() const {
        return head_;
    }
    
    // Get the last order in the list
    Order* back() const {
        return tail_;
    }
    
    // Check if list is empty
    bool empty() const {
        return head_ == nullptr;
    }
    
    // Get the number of orders in the list
    size_t size() const {
        return size_;
    }
    
    // Clear the list (doesn't deallocate orders, just unlinks them)
    void clear() {
        head_ = tail_ = nullptr;
        size_ = 0;
    }
    
    // Iterator for traversing the list
    class Iterator {
    public:
        Iterator(Order* current) : current_(current) {}
        
        Order* operator*() const { return current_; }
        Order* operator->() const { return current_; }
        
        Iterator& operator++() {
            if (current_) [[likely]] {
                current_ = current_->next;
            }
            return *this;
        }
        
        Iterator operator++(int) {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        bool operator==(const Iterator& other) const {
            return current_ == other.current_;
        }
        
        bool operator!=(const Iterator& other) const {
            return current_ != other.current_;
        }
        
    private:
        Order* current_;
    };
    
    Iterator begin() const { return Iterator(head_); }
    Iterator end() const { return Iterator(nullptr); }
    
private:
    Order* head_;
    Order* tail_;
    size_t size_;
};

} // namespace lob
