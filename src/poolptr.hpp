#ifndef POOLPTR_HPP
#define POOLPTR_HPP

#include <memory>

namespace mem {
namespace internal {

template <typename T>
class Deleter
{
   using DeleterFn = void(*)(void *, T *);

public:
   Deleter(void * mempool, DeleterFn dealloc)
      : m_pool(mempool)
      , m_dealloc(dealloc)
   {}
   Deleter()
      : m_pool(nullptr)
      , m_dealloc([](auto, auto){})
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