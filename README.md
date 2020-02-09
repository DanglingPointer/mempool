## C++ Memory pool

Header-only memory pool in C++17. 

Created by specifying a sequence of fixed sizes in ascending order. When allocating, finds the most suitable size for
 a given object type during compilation. Relies on `std::max_align_t`. All methods in `mem::Pool` must be called from
  the same thread, but a `mem::PoolPtr` obtained through `Pool::Make()` or a `std::shared_ptr` obtained through
   `Pool::MakeShared()` can be
   marshalled to any
   other thread and
   die wherever they want.
