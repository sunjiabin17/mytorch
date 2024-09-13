#include <gtest/gtest.h>

int add(int a, int b) {
    return a + b;
}

TEST(AddTest, Positive) {
    EXPECT_EQ(2, add(1, 1));
    EXPECT_EQ(10, add(5, 5));
}
