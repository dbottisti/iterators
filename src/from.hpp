#ifndef ITERATORS_INTO_HPP
#define ITERATORS_INTO_HPP

#include "base.hpp"
#include "detail/double_ended_owning_range_iterator.hpp"
#include "detail/double_ended_range_iterator.hpp"
#include "detail/owning_range_iterator.hpp"
#include "detail/range_iterator.hpp"
#include "detail/traits.hpp"

namespace iter {

template <typename Collection>
auto from(Collection&& collection)
    -> std::enable_if_t<
        !std::is_rvalue_reference<Collection&&>::value,
        std::conditional_t<
            detail::is_decrementable<decltype(std::begin(collection))>::value,
            detail::DoubleEndedRangeIterator<decltype(std::begin(collection))>,
            detail::RangeIterator<decltype(std::begin(collection))>>> {
    return {std::begin(collection), std::end(collection)};
}

template <typename Collection>
auto from(Collection&& collection)
    -> std::enable_if_t<
        std::is_rvalue_reference<Collection&&>::value,
        std::conditional_t<
            detail::is_decrementable<decltype(std::begin(collection))>::value,
            detail::DoubleEndedOwningRangeIterator<
                std::remove_reference_t<Collection>>,
            detail::OwningRangeIterator<std::remove_reference_t<Collection>>>> {
    return {std::move(collection)};
}

}  // namespace iter

#endif  // ITERATORS_INTO_HPP
