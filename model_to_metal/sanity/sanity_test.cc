#include <gtest/gtest.h>

#include <span>
#include <vector>

// Confirms the toolchain compiles C++20 and gtest links.
TEST(Sanity, Cxx20Available) {
  static_assert(__cplusplus >= 202002L);
  std::vector<int> v{1, 2, 3};
  std::span<const int> s{v};
  EXPECT_EQ(s.size(), 3u);
}
