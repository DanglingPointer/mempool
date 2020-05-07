#ifndef POOLPTR_HPP
#define POOLPTR_HPP

#include <memory>
#include <type_traits>

namespace mem {
namespace internal {

template <typename T>
class Deleter
{
   using DeleterFn = void(*)(void *, void *);

   template <typename U>
   friend class Deleter;

public:
   Deleter(void * mempool, DeleterFn dealloc)
      : m_pool(mempool)
      , m_dealloc(dealloc)
   {}
   Deleter()
      : m_pool(nullptr)
      , m_dealloc([](auto, auto){})
   {}
   template <typename U, typename = std::enable_if_t<std::is_convertible_v<U *, T *>>>
   Deleter(const Deleter<U> & other)
      : m_pool(other.m_pool)
      , m_dealloc(other.m_dealloc)
   {}
   void operator()(T * obj) const
   {
      m_dealloc(m_pool, obj);
   }

private:
   void * m_pool;
   DeleterFn m_dealloc;
};

} // internal

template <typename T>
using PoolPtr = std::unique_ptr<T, internal::Deleter<T>>;

} // mem

#endif // POOLPTR_HPP