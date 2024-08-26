#ifndef ITERATORS_DETAIL_OWNING_RANGE_ITERATOR_HPP
#define ITERATORS_DETAIL_OWNING_RANGE_ITERATOR_HPP

#include "base.hpp"
#include "detail/range_iterator.hpp"

namespace iter {
namespace detail {

template <typename Collection>
class OwningRangeIterator
    : public RangeIterator<decltype(std::begin(std::declval<Collection>()))> {
    using Super
        = RangeIterator<decltype(std::begin(std::declval<Collection>()))>;

public:
    OwningRangeIterator(Collection collection)
        : collection_{std::move(collection)},
          Super{std::begin(collection_), std::end(collection_)} {}

private:
    Collection collection_;
};

}  // namespace detail
}  // namespace iter

#endif  // ITERATORS_DETAIL_OWNING_RANGE_ITERATOR_HPP
