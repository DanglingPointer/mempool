## C++ Memory pool

Something in-between a memory pool and an object pool. Lock-free, header only, primarily intended for C++17 but might
 also compile with C++14.

A pool is created from a sequence of fixed sizes in ascending order unless using `mem::PoolSuitableFor<...>` in which
 case you can simply dump all your types you're going to use with the pool as template arguments regardless of ordering
  or possible duplicates of sizes. See `pool_test.cpp` for examples.
  
When allocating, the pool finds the most suitable size for a given object type during compilation. Relies on `std::max_align_t`. All methods in `mem::Pool` must be called from the
   same thread, but a `mem::PoolPtr` obtained through `Pool::MakeUnique()` or a `std::shared_ptr` obtained through
    `Pool::MakeShared()` can be marshalled to any other thread and die wherever they want.
