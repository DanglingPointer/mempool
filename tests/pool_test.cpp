#include <gtest/gtest.h>
#include <utility>
#include "mempool.hpp"

namespace {

TEST(MempoolTest, mempool_shrinks_and_resizes_correctly)
{
   {
      mem::Pool<2, 8, 32, 64> pool(5);
      auto p = pool.Make<std::pair<double, double>>(35.0, 36.0);
      EXPECT_EQ(5 * 4, pool.GetBlockCount());
      EXPECT_EQ((2+8+32+64) * 5, pool.GetSize());

      EXPECT_EQ(35.0, p->first);
      EXPECT_EQ(36.0, p->second);

      pool.ShrinkToFit();
      EXPECT_EQ(1U, pool.GetBlockCount());
      EXPECT_EQ(32, pool.GetSize());

      p.Reset();
      pool.ShrinkToFit();
      EXPECT_EQ(0U, pool.GetBlockCount());
      EXPECT_EQ(0U, pool.GetSize());

      pool.Resize(6U);
      EXPECT_EQ(6U * 4, pool.GetBlockCount());
      EXPECT_EQ((2+8+32+64) * 6, pool.GetSize());
   }

   {
      mem::Pool<4, 16> pool(5);
      auto p = pool.Make<int32_t>(42);
      EXPECT_EQ(5U * 2, pool.GetBlockCount());
      EXPECT_EQ((4+16) * 5, pool.GetSize());

      EXPECT_EQ(42, *p);

      pool.ShrinkToFit();
      EXPECT_EQ(1U, pool.GetBlockCount());
      EXPECT_EQ(4U, pool.GetSize());

      p.Reset();
      pool.ShrinkToFit();
      EXPECT_EQ(0U, pool.GetBlockCount());
      EXPECT_EQ(0U, pool.GetSize());

      pool.Resize(6U);
      EXPECT_EQ(6U * 2, pool.GetBlockCount());
      EXPECT_EQ((4+16) * 6, pool.GetSize());
   }
}

TEST(MempoolTest, mempool_shrinks_correctly_with_shared_ptr)
{
   {
      mem::Pool<2, 8, 32, 64> pool(5);
      auto p1 = pool.MakeShared<float>(35.0f);
      auto p2 = p1;
      EXPECT_EQ(5 * 4, pool.GetBlockCount());
      EXPECT_EQ((2+8+32+64) * 5, pool.GetSize());
      EXPECT_EQ(35.0f, *p1);
      EXPECT_EQ(35.0f, *p2);

      pool.ShrinkToFit();
      EXPECT_EQ(1U, pool.GetBlockCount());
      EXPECT_EQ(8, pool.GetSize());

      p1.reset();
      pool.ShrinkToFit();
      EXPECT_EQ(1U, pool.GetBlockCount());
      EXPECT_EQ(8, pool.GetSize());

      p2.reset();
      pool.ShrinkToFit();
      EXPECT_EQ(0U, pool.GetBlockCount());
      EXPECT_EQ(0U, pool.GetSize());
   }

   {
      mem::Pool<4, 16> pool(5);
      auto p1 = pool.MakeShared<int32_t>(42);
      auto p2 = p1;
      EXPECT_EQ(5U * 2, pool.GetBlockCount());
      EXPECT_EQ((4+16) * 5, pool.GetSize());
      EXPECT_EQ(42, *p1);
      EXPECT_EQ(42, *p2);

      pool.ShrinkToFit();
      EXPECT_EQ(1U, pool.GetBlockCount());
      EXPECT_EQ(4U, pool.GetSize());

      p1.reset();
      pool.ShrinkToFit();
      EXPECT_EQ(1U, pool.GetBlockCount());
      EXPECT_EQ(4U, pool.GetSize());

      p2.reset();
      pool.ShrinkToFit();
      EXPECT_EQ(0U, pool.GetBlockCount());
      EXPECT_EQ(0U, pool.GetSize());
   }
}

TEST(MempoolTest, mempool_cleans_up_when_constructor_throws)
{
   struct MyException {};
   struct Thrower
   {
      Thrower() {
         throw MyException{};
      }
   };

   mem::Pool<2, 8, 32, 64> pool(5);
   mem::PoolPtr<Thrower> p;

   EXPECT_THROW({
      p = pool.Make<Thrower>();
   }, MyException);

   pool.ShrinkToFit();
   EXPECT_EQ(0U, pool.GetBlockCount());
   EXPECT_EQ(0U, pool.GetSize());
}

} // namespace