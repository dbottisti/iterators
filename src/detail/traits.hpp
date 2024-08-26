#ifndef ITERATORS_DETAIL_TRAITS_HPP
#define ITERATORS_DETAIL_TRAITS_HPP

#include <utility>

namespace iter {
namespace detail {

template <typename U, typename = decltype(std::declval<U&>()--)>
static std::true_type is_decrementable_test(const U&&);

static std::false_type is_decrementable_test(...);

template <typename Iterator>
struct is_decrementable
    : decltype(is_decrementable_test(std::declval<Iterator>())){};

}  // namespace detail
}  // namespace iter

#endif  // ITERATORS_DETAIL_TRAITS_HPP
