#ifndef POOLBUILDER_HPP
#define POOLBUILDER_HPP

#include <utility>
#include <type_traits>
#include "mempool.hpp"

namespace mem {
namespace internal {

template <size_t... Ss>
using Sequence = std::integer_sequence<size_t, Ss...>;

/// --- Contains ---
template <typename IntegerSeq, size_t V>
struct Contains;

template <size_t V>
struct Contains<Sequence<>, V>
{
   static constexpr bool result = false;
};
template <size_t V, size_t... Ss>
struct Contains<Sequence<V, Ss...>, V>
{
   static constexpr bool result = true;
};
template <size_t V, size_t... Ss, size_t S>
struct Contains<Sequence<S, Ss...>, V>
{
   static constexpr bool result = Contains<Sequence<Ss...>, V>::result;
};
template <typename IntegerSeq, size_t V>
inline constexpr bool Contains_v = Contains<IntegerSeq, V>::result;


/// --- Combine ---
template <typename IntegerSeq1, typename IntegerSeq2>
struct Combine;

template <size_t... Ss1, size_t... Ss2>
struct Combine<Sequence<Ss1...>, Sequence<Ss2...>>
{
   using Result = Sequence<Ss1..., Ss2...>;
};
template <typename IntegerSeq1, typename IntegerSeq2>
using Combine_t = typename Combine<IntegerSeq1, IntegerSeq2>::Result;


/// --- EliminateDuplicates ---
template <typename IntegerSeq>
struct EliminateDuplicates;

template <>
struct EliminateDuplicates<Sequence<>>
{
   using Result = Sequence<>;
};
template <size_t... Ss, size_t S>
struct EliminateDuplicates<Sequence<S, Ss...>>
{
   static constexpr bool duplicated = Contains_v<Sequence<Ss...>, S>;

   using Result = std::conditional_t<
      duplicated,
      typename EliminateDuplicates<Sequence<Ss...>>::Result,
      Combine_t<
         Sequence<S>,
         typename EliminateDuplicates<Sequence<Ss...>>::Result>>;
};
template <typename IntegerSeq>
using EliminateDuplicates_t = typename EliminateDuplicates<IntegerSeq>::Result;


/// --- InsertOrdered ---
template <typename IntegerSeq, size_t V>
struct InsertOrdered;

template <size_t V>
struct InsertOrdered<Sequence<>, V>
{
   using Result = Sequence<V>;
};
template <size_t... Ss, size_t S, size_t V>
struct InsertOrdered<Sequence<S, Ss...>, V>
{
   using Result = std::conditional_t<
      V <= S,
      Sequence<V, S, Ss...>,
      Combine_t<
         Sequence<S>,
         typename InsertOrdered<Sequence<Ss...>, V>::Result>>;
};
template <typename IntegerSeq, size_t V>
using InsertOrdered_t = typename InsertOrdered<IntegerSeq, V>::Result;


/// --- Sort ---
template <typename IntegerSeq>
struct InsertionSort;

template <size_t S>
struct InsertionSort<Sequence<S>>
{
   using Result = Sequence<S>;
};
template <size_t S, size_t... Ss>
struct InsertionSort<Sequence<S, Ss...>>
{
   using OrderedSubseq = typename InsertionSort<Sequence<Ss...>>::Result;
   using Result = InsertOrdered_t<OrderedSubseq, S>;
};
template <typename IntegerSeq>
using Sorted_t = typename InsertionSort<IntegerSeq>::Result;


/// --- PoolFromSeq ---
template <typename IntegerSeq>
struct PoolFromSeq;

template <size_t... Ss>
struct PoolFromSeq<Sequence<Ss...>>
{
   using Result = mem::Pool<Ss...>;
};
template <typename IntegerSeq>
using PoolFromSeq_t = typename PoolFromSeq<IntegerSeq>::Result;

} // internal

template <typename... TArgs>
class PoolBuilder
{
   static_assert((... && std::is_standard_layout_v<TArgs>), "All types must have standard layout");
   using RawSequence = internal::Sequence<sizeof(TArgs)...>;
   using FilteredSequence = internal::EliminateDuplicates_t<RawSequence>;
   using SortedSequence = internal::Sorted_t<FilteredSequence>;

public:
   using Type = internal::PoolFromSeq_t<SortedSequence>;

};

template <typename... TArgs>
using PoolSuitableFor = typename PoolBuilder<TArgs...>::Type;

} // mem

#endif
