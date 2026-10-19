#include <gtest/gtest.h>

TEST(Canary, HeapOverflow) {
  int* p = new int[4]{};
  volatile int i = 4;
  // A read whose value is used: an out-of-bounds *store* right before delete[] is a dead store
  // that the optimizer can delete, and then ASan has nothing to catch.
  EXPECT_EQ(p[i], 0);  // out of bounds
  delete[] p;
}
