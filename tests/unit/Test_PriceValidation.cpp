#include <gtest/gtest.h>
#include "lob/Common.h"
#include "lob/OrderBook.h"

using namespace lob;

class PriceValidationTest : public ::testing::Test {
protected:
    OrderBook book;
};

TEST_F(PriceValidationTest, MinimumPriceBoundary) {
#ifdef HAVE_STD_EXPECTED
    auto result = book.add_limit_order(1, MIN_PRICE, 10, Side::BUY);
    EXPECT_TRUE(result);
#else
    auto err = book.add_limit_order(1, MIN_PRICE, 10, Side::BUY);
    EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
}

TEST_F(PriceValidationTest, MaximumPriceBoundary) {
#ifdef HAVE_STD_EXPECTED
    auto result = book.add_limit_order(1, MAX_PRICE, 10, Side::BUY);
    EXPECT_TRUE(result);
#else
    auto err = book.add_limit_order(1, MAX_PRICE, 10, Side::BUY);
    EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
}

TEST_F(PriceValidationTest, PriceBelowMinimum) {
#ifdef HAVE_STD_EXPECTED
    auto result = book.add_limit_order(1, MIN_PRICE - 1, 10, Side::BUY);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), ErrorCode::INVALID_PRICE);
#else
    auto err = book.add_limit_order(1, MIN_PRICE - 1, 10, Side::BUY);
    EXPECT_EQ(err, ErrorCode::INVALID_PRICE);
#endif
}

TEST_F(PriceValidationTest, PriceAboveMaximum) {
#ifdef HAVE_STD_EXPECTED
    auto result = book.add_limit_order(1, MAX_PRICE + 1, 10, Side::BUY);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), ErrorCode::INVALID_PRICE);
#else
    auto err = book.add_limit_order(1, MAX_PRICE + 1, 10, Side::BUY);
    EXPECT_EQ(err, ErrorCode::INVALID_PRICE);
#endif
}

TEST_F(PriceValidationTest, ZeroPrice) {
#ifdef HAVE_STD_EXPECTED
    auto result = book.add_limit_order(1, 0, 10, Side::BUY);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), ErrorCode::INVALID_PRICE);
#else
    auto err = book.add_limit_order(1, 0, 10, Side::BUY);
    EXPECT_EQ(err, ErrorCode::INVALID_PRICE);
#endif
}

TEST_F(PriceValidationTest, NegativePrice) {
#ifdef HAVE_STD_EXPECTED
    auto result = book.add_limit_order(1, -10000, 10, Side::BUY);
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), ErrorCode::INVALID_PRICE);
#else
    auto err = book.add_limit_order(1, -10000, 10, Side::BUY);
    EXPECT_EQ(err, ErrorCode::INVALID_PRICE);
#endif
}

TEST_F(PriceValidationTest, ValidPriceRange) {
    // Test various valid prices within range
    std::vector<Price> valid_prices = {
        10000,      // 1.0
        100000,     // 10.0
        1000000,    // 100.0
        10000000,   // 1000.0
        100000000,  // 10000.0
        1000000000  // 100000.0
    };
    
    for (size_t i = 0; i < valid_prices.size(); ++i) {
#ifdef HAVE_STD_EXPECTED
        auto result = book.add_limit_order(i + 1, valid_prices[i], 10, Side::BUY);
        EXPECT_TRUE(result);
#else
        auto err = book.add_limit_order(i + 1, valid_prices[i], 10, Side::BUY);
        EXPECT_EQ(err, ErrorCode::SUCCESS);
#endif
    }
}

TEST_F(PriceValidationTest, PricePrecision) {
    // Test 4 decimal place precision
#ifdef HAVE_STD_EXPECTED
    auto result1 = book.add_limit_order(1, 10001, 10, Side::BUY);  // 1.0001
    EXPECT_TRUE(result1);
    auto result2 = book.add_limit_order(2, 10005, 10, Side::BUY);  // 1.0005
    EXPECT_TRUE(result2);
    auto result3 = book.add_limit_order(3, 10009, 10, Side::BUY);  // 1.0009
    EXPECT_TRUE(result3);
#else
    auto err1 = book.add_limit_order(1, 10001, 10, Side::BUY);
    EXPECT_EQ(err1, ErrorCode::SUCCESS);
    auto err2 = book.add_limit_order(2, 10005, 10, Side::BUY);
    EXPECT_EQ(err2, ErrorCode::SUCCESS);
    auto err3 = book.add_limit_order(3, 10009, 10, Side::BUY);
    EXPECT_EQ(err3, ErrorCode::SUCCESS);
#endif
}
