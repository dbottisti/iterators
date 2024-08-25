#ifndef ITERATORS_DETAIL_OWNING_RANGE_ITERATOR_HPP
#define ITERATORS_DETAIL_OWNING_RANGE_ITERATOR_HPP

#include "base.hpp"

namespace iter {
namespace detail {

template <typename Collection>
class OwningRangeIterator
    : public Iterator<OwningRangeIterator<Collection>,
                      std::remove_reference_t<decltype(*std::begin(
                          std::declval<Collection>()))>> {
    using IteratorType = decltype(std::begin(std::declval<Collection>()));

public:
    using value_type
        = std::remove_reference_t<decltype(*std::declval<IteratorType>())>;

    OwningRangeIterator(Collection collection)
        : collection_{std::move(collection)},
          current_{std::begin(collection_)},
          end_{std::end(collection_)} {}

    std::optional<value_type> next() {
        if (current_ == end_) {
            return std::nullopt;
        }

        return *(current_++);
    }

private:
    Collection collection_;
    IteratorType current_;
    IteratorType end_;
};

}  // namespace detail
}  // namespace iter

#endif  // ITERATORS_DETAIL_OWNING_RANGE_ITERATOR_HPP
