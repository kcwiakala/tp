#include <gtest/gtest.h>

#include <ThreadPool.hpp>

namespace izi {

TEST(ThreadPool, push)
{
  ThreadPool pool(5);

  auto fut = pool.enqueue([]() {
    std::cout << "Running task" << std::endl;
    return 3+3;
  });

  EXPECT_EQ(fut.get(), 6);
}

} // namespace izi