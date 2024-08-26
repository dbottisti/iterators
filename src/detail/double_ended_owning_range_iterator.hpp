#ifndef ITERATORS_DETAIL_DOUBLE_ENDED_OWNING_RANGE_ITERATOR_HPP
#define ITERATORS_DETAIL_DOUBLE_ENDED_OWNING_RANGE_ITERATOR_HPP

#include "base.hpp"
#include "double_ended.hpp"

namespace iter {
namespace detail {

template <typename Collection>
class DoubleEndedOwningRangeIterator
    : public Iterator<DoubleEndedOwningRangeIterator<Collection>,
                      std::remove_reference_t<decltype(*std::begin(
                          std::declval<Collection>()))>>,
      public DoubleEndedIterator<DoubleEndedOwningRangeIterator<Collection>,
                                 std::remove_reference_t<decltype(*std::begin(
                                     std::declval<Collection>()))>> {
    using IteratorType = decltype(std::begin(std::declval<Collection>()));

public:
    using value_type = std::remove_reference_t<decltype(*std::begin(
        std::declval<Collection>()))>;

    DoubleEndedOwningRangeIterator(Collection collection)
        : collection_{std::move(collection)},
          current_{std::begin(collection_)},
          end_{std::end(collection_)} {}

    std::optional<value_type> next() {
        if (current_ == end_) {
            return std::nullopt;
        }

        return *(current_++);
    }

    std::optional<value_type> next_back() {
        if (current_ == end_) {
            return std::nullopt;
        }

        end_--;
        return *end_;
    }

private:
    Collection collection_;
    IteratorType current_;
    IteratorType end_;
};

}  // namespace detail
}  // namespace iter

#endif  // ITERATORS_DETAIL_DOUBLE_ENDED_OWNING_RANGE_ITERATOR_HPP
