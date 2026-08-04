#include <gtest/gtest.h>
#include "lob/ArenaAllocator.h"
#include <vector>
#include <string>

using namespace lob;

TEST(ArenaAllocatorTest, BasicAllocation) {
    ArenaAllocator arena(1024);
    
    EXPECT_EQ(arena.capacity(), 1024);
    EXPECT_EQ(arena.used(), 0);
    EXPECT_TRUE(arena.is_empty());
}

TEST(ArenaAllocatorTest, AllocateAndReset) {
    ArenaAllocator arena(1024);
    
    // Allocate some memory
    void* ptr1 = arena.allocate(100);
    ASSERT_NE(ptr1, nullptr);
    size_t used_after_first = arena.used();
    EXPECT_GT(used_after_first, 0);
    EXPECT_FALSE(arena.is_empty());
    
    // Allocate more
    void* ptr2 = arena.allocate(200);
    ASSERT_NE(ptr2, nullptr);
    size_t used_after_second = arena.used();
    EXPECT_GT(used_after_second, used_after_first);
    
    // Reset arena
    arena.reset();
    EXPECT_EQ(arena.used(), 0);
    EXPECT_TRUE(arena.is_empty());
    
    // Can allocate again after reset
    void* ptr3 = arena.allocate(50);
    ASSERT_NE(ptr3, nullptr);
    EXPECT_GT(arena.used(), 0);
}

TEST(ArenaAllocatorTest, Alignment) {
    ArenaAllocator arena(1024);
    
    // Allocate with different alignments
    void* ptr1 = arena.allocate(100, 16);
    ASSERT_NE(ptr1, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr1) % 16, 0);
    
    void* ptr2 = arena.allocate(100, 64);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr2) % 64, 0);
}

TEST(ArenaAllocatorTest, Exhaustion) {
    ArenaAllocator arena(100);
    
    // Allocate up to capacity
    void* ptr1 = arena.allocate(90);
    ASSERT_NE(ptr1, nullptr);
    
    // Try to allocate more than available
    void* ptr2 = arena.allocate(20);
    EXPECT_EQ(ptr2, nullptr);
}

TEST(ArenaAllocatorTest, MultipleResets) {
    ArenaAllocator arena(1024);
    
    for (int i = 0; i < 10; ++i) {
        void* ptr = arena.allocate(100);
        ASSERT_NE(ptr, nullptr);
        arena.reset();
        EXPECT_EQ(arena.used(), 0);
    }
}

TEST(ArenaAllocatorTest, LargeArena) {
    ArenaAllocator arena(1024 * 1024); // 1MB
    
    EXPECT_EQ(arena.capacity(), 1024 * 1024);
    
    // Allocate many small blocks
    for (int i = 0; i < 100; ++i) {
        void* ptr = arena.allocate(1024);
        ASSERT_NE(ptr, nullptr);
    }
    
    EXPECT_EQ(arena.used(), 100 * 1024);
}

TEST(ArenaAllocatorTest, ZeroSizeAllocation) {
    ArenaAllocator arena(1024);
    
    void* ptr = arena.allocate(0);
    EXPECT_NE(ptr, nullptr); // Should return valid pointer even for zero size
}

TEST(ArenaAllocatorTest, ScopedAllocation) {
    ArenaAllocator arena(1024);
    
    {
        ArenaScoped<int> scoped(arena);
        ASSERT_NE(scoped.get(), nullptr);
        *scoped.get() = 42;
        EXPECT_EQ(*scoped.get(), 42);
    } // Destructor called here
    
    // Arena should still be usable
    void* ptr = arena.allocate(100);
    ASSERT_NE(ptr, nullptr);
}

TEST(ArenaAllocatorTest, SequentialAllocation) {
    ArenaAllocator arena(1024);
    
    std::vector<void*> pointers;
    for (int i = 0; i < 10; ++i) {
        void* ptr = arena.allocate(50);
        ASSERT_NE(ptr, nullptr);
        pointers.push_back(ptr);
    }
    
    // All pointers should be different (sequential allocation)
    for (size_t i = 0; i < pointers.size(); ++i) {
        for (size_t j = i + 1; j < pointers.size(); ++j) {
            EXPECT_NE(pointers[i], pointers[j]);
        }
    }
}

#ifdef _WIN32
TEST(ArenaAllocatorTest, StringAllocation) {
    ArenaAllocator arena(1024);
    
    char* str = static_cast<char*>(arena.allocate(100));
    ASSERT_NE(str, nullptr);
    
    strcpy_s(str, 100, "Hello, Arena!");
    EXPECT_STREQ(str, "Hello, Arena!");
}
#else
TEST(ArenaAllocatorTest, StringAllocation) {
    ArenaAllocator arena(1024);
    
    char* str = static_cast<char*>(arena.allocate(100));
    ASSERT_NE(str, nullptr);
    
    strncpy(str, "Hello, Arena!", 99);
    str[99] = '\0';
    EXPECT_STREQ(str, "Hello, Arena!");
}
#endif

TEST(ArenaAllocatorTest, CapacityCheck) {
    ArenaAllocator arena(512);
    
    EXPECT_EQ(arena.capacity(), 512);
    
    void* ptr = arena.allocate(512);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(arena.used(), 512);
    
    // Should be exhausted
    void* ptr2 = arena.allocate(1);
    EXPECT_EQ(ptr2, nullptr);
}
