#include <gtest/gtest.h>

#include <thread>

TEST(Canary, DataRace) {
  int counter = 0;
  std::thread a([&] { for (int i = 0; i < 100000; ++i) ++counter; });
  std::thread b([&] { for (int i = 0; i < 100000; ++i) ++counter; });
  a.join();
  b.join();
  EXPECT_GT(counter, 0);
}
