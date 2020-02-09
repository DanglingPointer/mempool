## C++ Memory pool

Something in-between a memory pool and an object pool. Lock-free, header only, primarily intended for C++17 but might also compile using C++14.

A pool is created by specifying a sequence of fixed sizes in ascending order so the user must know in advance the max
 size of objects the pool is going to be used for. When allocating, it finds the most suitable size for a given
  object type during compilation. Relies on `std::max_align_t`. All methods in `mem::Pool` must be called from the
   same thread, but a `mem::PoolPtr` obtained through `Pool::MakeUnique()` or a `std::shared_ptr` obtained through
    `Pool::MakeShared()` can be marshalled to any other thread and die wherever they want.
