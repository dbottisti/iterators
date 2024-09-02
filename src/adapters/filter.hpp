#ifndef ITERATOR_ADAPTERS_FILTER_HPP
#define ITERATOR_ADAPTERS_FILTER_HPP

#include <utility>

#include "detail/traits.hpp"

namespace iter {

// Forward declarations
template <typename Self, typename T>
class Iterator;

// Filter

template <typename Predicate, typename BaseIterator, typename Enable>
class Filter
    : public Iterator<
          Filter<Predicate, BaseIterator>,
          std::invoke_result_t<Predicate, typename std::remove_reference_t<
                                              BaseIterator>::value_type>> {
    using BaseNoRef = std::remove_reference_t<BaseIterator>;
    using T = typename BaseNoRef::value_type;

public:
    static_assert(
        std::is_invocable_r_v<
            bool, Predicate,
            typename std::remove_reference_t<BaseIterator>::value_type>,
        "Predicate provided to filter() is incompatible with the value type of "
        "the iterator");
    using value_type = typename BaseNoRef::value_type;

    Filter(Predicate predicate, BaseNoRef&& base_iterator)
        : base_iterator_{std::move(base_iterator)},
          predicate_{std::move(predicate)} {}

    Filter(Predicate predicate, BaseNoRef& base_iterator)
        : base_iterator_{base_iterator}, predicate_{std::move(predicate)} {}

    std::optional<value_type> next() {
        while (true) {
            auto first = base_iterator_.next();
            if (!first) {
                return {};
            }
            if (predicate_(first.value())) {
                return first;
            }
        }
    }

    template <typename B, typename F>
    auto fold(const B init, F f) -> B {
        return base_iterator_.fold(init,
                                   [&f, this](const auto acc, const auto x) {
                                       return predicate_(x) ? f(acc, x) : acc;
                                   });
    }

    template <typename B, typename F>
    auto try_fold(const B init, F f) -> decltype(f(init, std::declval<T>())) {
        return base_iterator_.try_fold(
            init, [&f, this](const auto acc, const auto x) {
                return predicate_(x) ? f(acc, x) : acc;
            });
    }

private:
    BaseIterator base_iterator_;
    Predicate predicate_;
};

template <typename Predicate, typename BaseIterator>
class Filter<Predicate, BaseIterator,
             std::enable_if_t<detail::is_double_ended<
                 std::remove_reference_t<BaseIterator>>::value>>
    : public Iterator<
          Filter<Predicate, BaseIterator>,
          std::invoke_result_t<Predicate, typename std::remove_reference_t<
                                              BaseIterator>::value_type>>,
      public DoubleEndedIterator<
          Filter<Predicate, BaseIterator>,
          std::invoke_result_t<Predicate, typename std::remove_reference_t<
                                              BaseIterator>::value_type>> {
    using BaseNoRef = std::remove_reference_t<BaseIterator>;
    using T = typename BaseNoRef::value_type;

public:
    static_assert(
        std::is_invocable_r_v<
            bool, Predicate,
            typename std::remove_reference_t<BaseIterator>::value_type>,
        "Predicate provided to filter() is incompatible with the value type of "
        "the iterator");
    using value_type = typename BaseNoRef::value_type;

    Filter(Predicate predicate, BaseNoRef&& base_iterator)
        : base_iterator_{std::move(base_iterator)},
          predicate_{std::move(predicate)} {}

    Filter(Predicate predicate, BaseNoRef& base_iterator)
        : base_iterator_{base_iterator}, predicate_{std::move(predicate)} {}

    std::optional<value_type> next() {
        while (true) {
            auto first = base_iterator_.next();
            if (!first) {
                return {};
            }
            if (predicate_(first.value())) {
                return first;
            }
        }
    }

    std::optional<value_type> next_back() {
        while (true) {
            auto first = base_iterator_.next_back();
            if (!first) {
                return {};
            }
            if (predicate_(first.value())) {
                return first;
            }
        }
    }

    template <typename B, typename F>
    auto fold(const B init, F f) -> B {
        return base_iterator_.fold(init,
                                   [&f, this](const auto acc, const auto x) {
                                       return predicate_(x) ? f(acc, x) : acc;
                                   });
    }

    template <typename B, typename F>
    auto try_fold(const B init, F f) -> decltype(f(init, std::declval<T>())) {
        return base_iterator_.try_fold(
            init, [&f, this](const auto acc, const auto x) {
                return predicate_(x) ? f(acc, x) : acc;
            });
    }

    template <typename B, typename F>
    auto rfold(const B init, F f) -> B {
        return base_iterator_.rfold(init,
                                    [&f, this](const auto acc, const auto x) {
                                        return predicate_(x) ? f(acc, x) : acc;
                                    });
    }

    template <typename B, typename F>
    auto try_rfold(const B init, F f) -> decltype(f(init, std::declval<T>())) {
        return base_iterator_.try_rfold(
            init, [&f, this](const auto acc, const auto x) {
                return predicate_(x) ? f(acc, x) : acc;
            });
    }

private:
    BaseIterator base_iterator_;
    Predicate predicate_;
};

}  // namespace iter

#endif  // ITERATOR_ADAPTERS_FILTER_HPP
