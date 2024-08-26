#ifndef ITERATORS_DETAIL_DOUBLE_ENDED_RANGE_ITERATOR_HPP
#define ITERATORS_DETAIL_DOUBLE_ENDED_RANGE_ITERATOR_HPP

#include "base.hpp"
#include "double_ended.hpp"

namespace iter {
namespace detail {

template <typename IteratorType>
class DoubleEndedRangeIterator
    : public Iterator<
          DoubleEndedRangeIterator<IteratorType>,
          std::remove_reference_t<decltype(*std::declval<IteratorType>())>>,
      public DoubleEndedIterator<
          DoubleEndedRangeIterator<IteratorType>,
          std::remove_reference_t<decltype(*std::declval<IteratorType>())>> {
public:
    using value_type
        = std::remove_reference_t<decltype(*std::declval<IteratorType>())>;

    DoubleEndedRangeIterator(IteratorType current, IteratorType end)
        : current_{current}, end_{end} {}

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
    IteratorType current_;
    IteratorType end_;
};

}  // namespace detail
}  // namespace iter

#endif  // ITERATORS_DETAIL_DOUBLE_ENDED_RANGE_ITERATOR_HPP
