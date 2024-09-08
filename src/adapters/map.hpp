#ifndef ITERATORS_ADAPTERS_MAP_HPP
#define ITERATORS_ADAPTERS_MAP_HPP

#include <type_traits>

#include "declarations.hpp"
#include "detail/empty_base.hpp"
#include "detail/traits.hpp"

namespace iter {

template <typename Function, typename BaseIterator>
class Map
    : public Iterator<
          Map<Function, BaseIterator>,
          std::invoke_result_t<Function, typename std::remove_reference_t<
                                             BaseIterator>::value_type>>,
      public std::conditional_t<
          detail::is_double_ended<std::remove_reference_t<BaseIterator>>::value,
          DoubleEndedIterator<
              Map<Function, BaseIterator>,
              std::invoke_result_t<Function, typename std::remove_reference_t<
                                                 BaseIterator>::value_type>>,
          detail::EmptyBase> {
    using BaseNoRef = std::remove_reference_t<BaseIterator>;
    using T = typename BaseNoRef::value_type;

    template <typename U>
    using is_double_ended = detail::is_double_ended<std::remove_reference_t<U>>;

    template <typename U>
    using is_exact_size = detail::is_exact_size<std::remove_reference_t<U>>;

public:
    static_assert(
        std::is_invocable_v<Function, typename std::remove_reference_t<
                                          BaseIterator>::value_type>,
        "Function provided to map() is incompatible with the value type of the "
        "iterator");
    using value_type
        = std::invoke_result_t<Function, typename BaseNoRef::value_type>;

    Map(Function function, BaseNoRef&& base_iterator)
        : base_iterator_{std::move(base_iterator)},
          function_{std::move(function)} {}

    Map(Function function, BaseNoRef& base_iterator)
        : base_iterator_{base_iterator}, function_{std::move(function)} {}

    // -------------------------------- Iterator -------------------------------

    std::optional<value_type> next() {
        auto first = base_iterator_.next();
        if (first) {
            return function_(first.value());
        }
        return {};
    }

    template <typename B, typename F>
    auto fold(const B init, F f) -> B {
        return base_iterator_.fold(init,
                                   [&f, this](const auto acc, const auto x) {
                                       return f(acc, function_(x));
                                   });
    }

    template <typename B, typename F>
    auto try_fold(const B init, F f) -> decltype(f(init, std::declval<T>())) {
        return base_iterator_.try_fold(
            init, [&f, this](const auto acc, const auto x) {
                return f(acc, function_(x));
            });
    }

    std::pair<std::size_t, std::optional<std::size_t>> size_hint() const {
        return base_iterator_.size_hint();
    }

    // -------------------------- DoubleEndedIterator --------------------------

    template <typename U = BaseIterator>
    auto next_back() -> std::enable_if_t<is_double_ended<U>::value,
                                         std::optional<value_type>> {
        auto first = base_iterator_.next_back();
        if (first) {
            return function_(first.value());
        }
        return {};
    }

    template <typename B, typename F, typename U = BaseIterator>
    auto rfold(const B init,
               F f) -> std::enable_if_t<is_double_ended<U>::value, B> {
        return base_iterator_.rfold(init,
                                    [&f, this](const auto acc, const auto x) {
                                        return f(acc, function_(x));
                                    });
    }

    template <typename B, typename F, typename U = BaseIterator>
    auto try_rfold(const B init, F f)
        -> std::enable_if_t<is_double_ended<U>::value,
                            decltype(f(init, std::declval<T>()))> {
        return base_iterator_.try_rfold(
            init, [&f, this](const auto acc, const auto x) {
                return f(acc, function_(x));
            });
    }

private:
    BaseIterator base_iterator_;
    Function function_;
};

}  // namespace iter

#endif  //  ITERATORS_ADAPTERS_MAP_HPP
