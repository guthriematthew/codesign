#include <gtest/gtest.h>

#include <climits>

TEST(Canary, SignedOverflow) {
  volatile int x = INT_MAX;
  int y = x + 1;  // signed overflow is UB
  EXPECT_NE(y, 0);
}
