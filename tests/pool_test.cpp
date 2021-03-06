#include <gtest/gtest.h>
#include <utility>
#include "poolbuilder.hpp"
#include "mempool.hpp"

namespace {

TEST(MempoolTest, mempool_shrinks_and_resizes_correctly)
{
   {
      mem::Pool<2, 8, 32, 64> pool(5);
      auto p = pool.MakeUnique<std::pair<double, double>>(35.0, 36.0);
      EXPECT_EQ(5 * 4, pool.GetBlockCount());
      EXPECT_EQ((2+8+32+64) * 5, pool.GetSize());

      EXPECT_EQ(35.0, p->first);
      EXPECT_EQ(36.0, p->second);

      pool.ShrinkToFit();
      EXPECT_EQ(1U, pool.GetBlockCount());
      EXPECT_EQ(32, pool.GetSize());

      p.reset();
      pool.ShrinkToFit();
      EXPECT_EQ(0U, pool.GetBlockCount());
      EXPECT_EQ(0U, pool.GetSize());

      pool.Resize(6U);
      EXPECT_EQ(6U * 4, pool.GetBlockCount());
      EXPECT_EQ((2+8+32+64) * 6, pool.GetSize());
   }

   {
      mem::Pool<4, 16> pool(5);
      auto p = pool.MakeUnique<int32_t>(42);
      EXPECT_EQ(5U * 2, pool.GetBlockCount());
      EXPECT_EQ((4+16) * 5, pool.GetSize());

      EXPECT_EQ(42, *p);

      pool.ShrinkToFit();
      EXPECT_EQ(1U, pool.GetBlockCount());
      EXPECT_EQ(4U, pool.GetSize());

      p.reset();
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
      p = pool.MakeUnique<Thrower>();
   }, MyException);

   pool.ShrinkToFit();
   EXPECT_EQ(0U, pool.GetBlockCount());
   EXPECT_EQ(0U, pool.GetSize());
}

TEST(MempoolTest, mempool_grows_when_necessary_and_calls_constructors_and_destructors_correcly)
{
   size_t constructed_count = 0U;
   size_t destructed_count = 0U;

   struct Counter
   {
      Counter(size_t & constructed_count, size_t & destructed_count)
         : constructed_count(&constructed_count), destructed_count(&destructed_count)
      {
         (*this->constructed_count)++;
      }
      ~Counter()
      {
         (*this->destructed_count)++;
      }

      size_t * const constructed_count;
      size_t * const destructed_count;
   };

   mem::Pool<sizeof(Counter)> pool(1);
   EXPECT_EQ(1U, pool.GetBlockCount());
   EXPECT_EQ(sizeof(Counter), pool.GetSize());

   auto p1 = pool.MakeUnique<Counter>(constructed_count, destructed_count);
   EXPECT_EQ(1U, constructed_count);
   EXPECT_EQ(0U, destructed_count);
   EXPECT_EQ(1U, pool.GetBlockCount());
   EXPECT_EQ(sizeof(Counter), pool.GetSize());

   auto p2 = pool.MakeUnique<Counter>(constructed_count, destructed_count);
   EXPECT_EQ(2U, constructed_count);
   EXPECT_EQ(0U, destructed_count);
   EXPECT_EQ(2U, pool.GetBlockCount());
   EXPECT_EQ(2 * sizeof(Counter), pool.GetSize());

   auto p3 = pool.MakeUnique<Counter>(constructed_count, destructed_count);
   EXPECT_EQ(3U, constructed_count);
   EXPECT_EQ(0U, destructed_count);
   EXPECT_EQ(3U, pool.GetBlockCount());
   EXPECT_EQ(3 * sizeof(Counter), pool.GetSize());

   p1.reset();
   EXPECT_EQ(1U, destructed_count);
   EXPECT_EQ(3U, constructed_count);

   auto p4 = pool.MakeUnique<Counter>(constructed_count, destructed_count);
   EXPECT_EQ(4U, constructed_count);
   EXPECT_EQ(1U, destructed_count);
   EXPECT_EQ(3U, pool.GetBlockCount());
   EXPECT_EQ(3 * sizeof(Counter), pool.GetSize());

   p2.reset();
   p3.reset();
   pool.ShrinkToFit();
   EXPECT_EQ(4U, constructed_count);
   EXPECT_EQ(3U, destructed_count);
   EXPECT_EQ(1U, pool.GetBlockCount());
   EXPECT_EQ(sizeof(Counter), pool.GetSize());
}

TEST(MempoolTest, poolbuilder_eliminates_duplicates_and_sorts)
{
   using Pool = mem::PoolSuitableFor<
      uint64_t,
      int32_t,
      int64_t,
      char,
      float,
      uint16_t>;

   Pool p(1);
   EXPECT_EQ(4U, p.GetBlockCount());
   EXPECT_EQ(1U+2U+4U+8U, p.GetSize());
}

} // namespace