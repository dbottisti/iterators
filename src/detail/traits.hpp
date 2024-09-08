#ifndef ITERATORS_DETAIL_TRAITS_HPP
#define ITERATORS_DETAIL_TRAITS_HPP

#include <type_traits>

namespace iter {
namespace detail {

//------------------------------------------------------------------------------

template <typename... Ts>
struct make_void {
    typedef void type;
};

template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

//------------------------------------------------------------------------------

template <typename U, typename = decltype(std::declval<U&>()--)>
static std::true_type is_decrementable_test(const U&&);

static std::false_type is_decrementable_test(...);

template <typename Iterator>
struct is_decrementable
    : decltype(is_decrementable_test(std::declval<Iterator>())){};

//------------------------------------------------------------------------------

template <typename Iterator, typename Enable = void>
struct is_double_ended : std::false_type {};

template <typename Iterator>
struct is_double_ended<
    Iterator, std::enable_if_t<Iterator::is_double_ended_iterator == true>>
    : std::true_type {};

//------------------------------------------------------------------------------

template <typename Iterator, typename Enable = void>
struct is_exact_size : std::false_type {};

template <typename Iterator>
struct is_exact_size<Iterator,
                     std::enable_if_t<Iterator::is_exact_size_iterator == true>>
    : std::true_type {};

//------------------------------------------------------------------------------

template <typename U, typename = decltype(std::declval<U&>().size())>
static std::true_type is_sized_test(const U&&);

static std::false_type is_sized_test(...);

template <typename T>
struct is_sized : decltype(is_sized_test(std::declval<T>())){};

//------------------------------------------------------------------------------

template <typename T, typename Enable = void>
struct type_is_valid : std::false_type {};

template <typename T>
struct type_is_valid<T, void_t<T>> : std::true_type {};

//------------------------------------------------------------------------------

template <typename U,
          typename = decltype(std::declval<U&>() - std::declval<U&>())>
static std::true_type has_operator_minus_test(const U&&);

static std::false_type has_operator_minus_test(...);

template <typename Iterator>
struct has_operator_minus
    : decltype(has_operator_minus_test(std::declval<Iterator>())){};

}  // namespace detail
}  // namespace iter

#endif  // ITERATORS_DETAIL_TRAITS_HPP
