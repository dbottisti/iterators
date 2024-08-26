#ifndef ITERATORS_DETAIL_DOUBLE_ENDED_OWNING_RANGE_ITERATOR_HPP
#define ITERATORS_DETAIL_DOUBLE_ENDED_OWNING_RANGE_ITERATOR_HPP

#include "base.hpp"
#include "detail/double_ended_range_iterator.hpp"

namespace iter {
namespace detail {

template <typename Collection>
class DoubleEndedOwningRangeIterator
    : public DoubleEndedRangeIterator<decltype(std::begin(
          std::declval<Collection>()))> {
    using Super = DoubleEndedRangeIterator<decltype(std::begin(
        std::declval<Collection>()))>;

public:
    DoubleEndedOwningRangeIterator(Collection collection)
        : collection_{std::move(collection)},
          Super{std::begin(collection_), std::end(collection_)} {}

private:
    Collection collection_;
};

}  // namespace detail
}  // namespace iter

#endif  // ITERATORS_DETAIL_DOUBLE_ENDED_OWNING_RANGE_ITERATOR_HPP
