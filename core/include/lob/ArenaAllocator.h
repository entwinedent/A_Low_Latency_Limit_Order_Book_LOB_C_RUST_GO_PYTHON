#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <cstring>

namespace lob {

// Arena Allocator for zero-cost temporary allocations
// Uses a bump allocator pattern - allocations are sequential
// and the entire arena is reset in one operation
class ArenaAllocator {
public:
    explicit ArenaAllocator(size_t capacity = 1024 * 1024) 
        : capacity_(capacity)
        , buffer_(nullptr)
        , offset_(0)
    {
        // Allocate aligned memory
        buffer_ = _aligned_malloc(capacity_, 64);
        if (!buffer_) {
            throw std::bad_alloc();
        }
    }
    
    ~ArenaAllocator() {
        if (buffer_) {
            _aligned_free(buffer_);
        }
    }
    
    // Prevent copying
    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;
    
    // Allocate memory from arena
    void* allocate(size_t size, size_t alignment = 16) {
        // Align the current offset
        size_t aligned_offset = (offset_ + alignment - 1) & ~(alignment - 1);
        
        // Check if we have enough space
        if (aligned_offset + size > capacity_) [[unlikely]] {
            return nullptr; // Arena exhausted
        }
        
        void* ptr = static_cast<char*>(buffer_) + aligned_offset;
        offset_ = aligned_offset + size;
        return ptr;
    }
    
    // Reset the arena to initial state
    void reset() {
        offset_ = 0;
    }
    
    // Get current usage
    size_t used() const { return offset_; }
    
    // Get total capacity
    size_t capacity() const { return capacity_; }
    
    // Check if arena is empty
    bool is_empty() const { return offset_ == 0; }
    
private:
    size_t capacity_;      // Total capacity in bytes
    void* buffer_;        // Aligned memory block
    size_t offset_;       // Current offset (bump pointer)
};

// RAII wrapper for arena-scoped allocations
template <typename T>
class ArenaScoped {
public:
    ArenaScoped(ArenaAllocator& arena)
        : arena_(arena)
        , ptr_(arena.allocate(sizeof(T), alignof(T)))
    {
        if (ptr_) {
            new (ptr_) T();
        }
    }
    
    ~ArenaScoped() {
        if (ptr_) {
            static_cast<T*>(ptr_)->~T();
        }
        // No deallocation - arena will be reset
    }
    
    T* get() const { return static_cast<T*>(ptr_); }
    T* operator->() const { return get(); }
    T& operator*() const { return *get(); }
    
    explicit operator bool() const { return ptr_ != nullptr; }
    
private:
    ArenaAllocator& arena_;
    void* ptr_;
};

} // namespace lob
