#include <gtest/gtest.h>
#include "lob/Common.h"
#include <limits>

using namespace lob;

TEST(FixedPointTest, PriceToFixedConversion) {
    // Test basic conversions
    EXPECT_EQ(price_to_fixed(0.0), 0);
    EXPECT_EQ(price_to_fixed(1.0), 10000);
    EXPECT_EQ(price_to_fixed(100.0), 1000000);
    EXPECT_EQ(price_to_fixed(0.0001), 1);
    EXPECT_EQ(price_to_fixed(0.0005), 5);
    EXPECT_EQ(price_to_fixed(0.9999), 9999);
}

TEST(FixedPointTest, PriceToDoubleConversion) {
    // Test reverse conversions
    EXPECT_DOUBLE_EQ(price_to_double(0), 0.0);
    EXPECT_DOUBLE_EQ(price_to_double(10000), 1.0);
    EXPECT_DOUBLE_EQ(price_to_double(1000000), 100.0);
    EXPECT_DOUBLE_EQ(price_to_double(1), 0.0001);
    EXPECT_DOUBLE_EQ(price_to_double(9999), 0.9999);
}

TEST(FixedPointTest, RoundTripConversion) {
    // Test round-trip conversion accuracy
    double original = 123.4567;
    Price fixed = price_to_fixed(original);
    double converted = price_to_double(fixed);
    EXPECT_NEAR(original, converted, 0.0001);
}

TEST(FixedPointTest, PriceToStringFormatting) {
    // Test string formatting
    EXPECT_EQ(price_to_string(10000), "1.0000");
    EXPECT_EQ(price_to_string(1000000), "100.0000");
    EXPECT_EQ(price_to_string(1), "0.0001");
    EXPECT_EQ(price_to_string(12345), "1.2345");
}

TEST(FixedPointTest, BoundaryValues) {
    // Test minimum and maximum price values
    EXPECT_EQ(price_to_double(MIN_PRICE), 0.0001);
    EXPECT_EQ(price_to_double(MAX_PRICE), 1000000.0);
    
    // Test conversion at boundaries
    EXPECT_EQ(price_to_fixed(0.0001), MIN_PRICE);
    // Note: MAX_PRICE is 10000000000 (1000000.0 in fixed-point)
    // Test with a value that fits in int64_t range
    EXPECT_EQ(price_to_fixed(100000.0), 1000000000);
}

TEST(FixedPointTest, NegativePrices) {
    // Test negative price handling
    EXPECT_EQ(price_to_fixed(-1.0), -10000);
    EXPECT_EQ(price_to_fixed(-100.0), -1000000);
    EXPECT_EQ(price_to_double(-10000), -1.0);
}

TEST(FixedPointTest, FractionalPrices) {
    // Test various fractional values
    EXPECT_EQ(price_to_fixed(0.5), 5000);
    EXPECT_EQ(price_to_fixed(0.25), 2500);
    EXPECT_EQ(price_to_fixed(0.125), 1250);
    EXPECT_EQ(price_to_fixed(0.0625), 625);
}

TEST(FixedPointTest, LargePrices) {
    // Test large price values
    EXPECT_EQ(price_to_fixed(10000.0), 100000000);
    EXPECT_EQ(price_to_fixed(100000.0), 1000000000);
    EXPECT_DOUBLE_EQ(price_to_double(1000000000), 100000.0);
}

TEST(FixedPointTest, PrecisionLimits) {
    // Test precision limits (4 decimal places)
    EXPECT_EQ(price_to_fixed(0.00001), 0); // Below precision, rounds to 0
    EXPECT_EQ(price_to_fixed(0.00005), 1); // Rounds up (0.5 rounds away from zero)
    EXPECT_EQ(price_to_fixed(0.0001), 1);  // At precision limit
    EXPECT_EQ(price_to_fixed(0.00015), 1); // Rounds down
    EXPECT_EQ(price_to_fixed(0.00016), 2); // Rounds up
}
