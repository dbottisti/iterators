#ifndef ITERATORS_DETAIL_RANGE_ITERATOR_HPP
#define ITERATORS_DETAIL_RANGE_ITERATOR_HPP

#include "base.hpp"

namespace iter {
namespace detail {

template <typename IteratorType>
class RangeIterator
    : public Iterator<
          RangeIterator<IteratorType>,
          std::remove_reference_t<decltype(*std::declval<IteratorType>())>> {
public:
    using value_type
        = std::remove_reference_t<decltype(*std::declval<IteratorType>())>;

    RangeIterator(IteratorType current, IteratorType end)
        : current_{current}, end_{end} {}

    std::optional<value_type> next() {
        if (current_ == end_) {
            return std::nullopt;
        }

        return *(current_++);
    }

private:
    IteratorType current_;
    IteratorType end_;
};

}  // namespace detail
}  // namespace iter

#endif  // ITERATORS_DETAIL_RANGE_ITERATOR_HPP
