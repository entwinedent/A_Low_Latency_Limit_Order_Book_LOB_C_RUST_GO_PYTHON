#pragma once

#include <vector>
#include <cstddef>
#include <new>
#include <cstdint>
#include <memory>
#include <cstdlib>

#ifdef _WIN32
#include <malloc.h>
#endif

namespace lob {

// Pre-allocated memory pool for zero-allocation order management
// Uses placement new to construct objects in pre-allocated memory
template <typename T, size_t Capacity>
class MemoryPool {
public:
    MemoryPool() : pool_data_(nullptr) {
        // Allocate aligned raw memory without constructing objects
#ifdef _WIN32
        pool_data_ = _aligned_malloc(Capacity * sizeof(T), alignof(T));
#else
        pool_data_ = aligned_alloc(alignof(T), Capacity * sizeof(T));
#endif
        free_list_.reserve(Capacity);
        for (size_t i = 0; i < Capacity; ++i) {
            free_list_.push_back(reinterpret_cast<T*>(static_cast<char*>(pool_data_) + (i * sizeof(T))));
        }
    }
    
    ~MemoryPool() {
        // Free the raw memory
        if (pool_data_) {
#ifdef _WIN32
            _aligned_free(pool_data_);
#else
            free(pool_data_);
#endif
        }
    }
    
    // Prevent copying
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
    
    // Allocate an object from the pool using placement new
    template <typename... Args>
    T* allocate(Args&&... args) {
        if (free_list_.empty()) [[unlikely]] {
            return nullptr; // Pool exhausted
        }
        
        T* ptr = free_list_.back();
        free_list_.pop_back();
        return new (ptr) T(std::forward<Args>(args)...);
    }
    
    // Deallocate an object and return it to the free list
    void deallocate(T* ptr) {
        if (ptr) [[likely]] {
            ptr->~T(); // Call destructor manually
            free_list_.push_back(ptr);
        }
    }
    
    // Get statistics
    size_t capacity() const { return Capacity; }
    size_t allocated_count() const { return Capacity - free_list_.size(); }
    size_t free_count() const { return free_list_.size(); }
    bool is_full() const { return free_list_.empty(); }
    bool is_empty() const { return free_list_.size() == Capacity; }
    
private:
    void* pool_data_;              // Raw pre-allocated memory block
    std::vector<T*> free_list_;    // Stack of free pointers
};

} // namespace lob
