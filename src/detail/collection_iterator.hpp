#ifndef ITERATORS_DETAIL_RANGE_ITERATOR_HPP
#define ITERATORS_DETAIL_RANGE_ITERATOR_HPP

#include <type_traits>

#include "base.hpp"
#include "detail/empty_base.hpp"

namespace iter {
namespace detail {

template <typename Collection>
class CollectionIterator
    : public Iterator<CollectionIterator<Collection>,
                      std::remove_reference_t<decltype(*std::begin(
                          std::declval<Collection>()))>>,
      public std::conditional_t<
          detail::is_decrementable<decltype(std::begin(
              std::declval<Collection>()))>::value,
          DoubleEndedIterator<CollectionIterator<Collection>,
                              std::remove_reference_t<decltype(*std::begin(
                                  std::declval<Collection>()))>>,
          EmptyBase>,
      public std::conditional_t<
          detail::is_sized<std::remove_reference_t<Collection>>::value,
          ExactSizeIterator<CollectionIterator<Collection>,
                            std::remove_reference_t<decltype(*std::begin(
                                std::declval<Collection>()))>>,
          EmptyBase> {
    using IteratorType = decltype(std::begin(std::declval<Collection>()));

    template <typename T>
    using is_double_ended
        = detail::is_decrementable<decltype(std::begin(std::declval<T>()))>;

    template <typename T>
    using is_exact_size = detail::type_is_valid<decltype(std::distance(
        std::begin(std::declval<T>()), std::end(std::declval<T>())))>;

public:
    using value_type
        = std::remove_reference_t<decltype(*std::declval<IteratorType>())>;

    CollectionIterator(IteratorType current, IteratorType end)
        : current_{current}, end_{end} {}

    std::optional<value_type> next() {
        if (current_ == end_) {
            return std::nullopt;
        }

        return *(current_++);
    }

    // -------------------------- DoubleEndedIterator --------------------------

    template <typename T = Collection>
    auto next_back() -> std::enable_if_t<is_double_ended<Collection>::value,
                                         std::optional<value_type>> {
        if (current_ == end_) {
            return std::nullopt;
        }

        end_--;
        return *end_;
    }

    // --------------------------- ExactSizeIterator ---------------------------

    template <typename T = Collection>
    auto size_hint() const
        -> std::enable_if_t<
            is_exact_size<T>::value,
            std::pair<std::size_t, std::optional<std::size_t>>> {
        const auto n = std::distance(current_, end_);
        return {n, n};
    }

private:
    IteratorType current_;
    IteratorType end_;
};

}  // namespace detail
}  // namespace iter

#endif  // ITERATORS_DETAIL_RANGE_ITERATOR_HPP
