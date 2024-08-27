#ifndef ITERATORS_BASE_HPP
#define ITERATORS_BASE_HPP

#include "detail/empty_base.hpp"
#include "detail/traits.hpp"
#include "double_ended.hpp"

namespace iter {

// ---------------------------- Forward declarations ---------------------------

template <typename Function, typename BaseIterator, typename Enable = void>
class Map;

template <typename Predicate, typename BaseIterator, typename Enable = void>
class Filter;

// ---------------------------------- Iterator ---------------------------------

template <typename Self, typename T>
class Iterator {
public:
    static constexpr bool is_iterator = true;

    std::size_t count() {
        return self().fold(std::size_t{0},
                           [](const auto acc, const auto) { return acc + 1; });
    }

    template <typename B, typename F>
    B fold(const B init, F f) {
        auto accum = init;
        while (true) {
            const auto maybe_x = self().next();
            if (!maybe_x) {
                return accum;
            }
            accum = f(accum, *maybe_x);
        }
    }

    template <typename B, typename F>
    auto try_fold(const B init, F f) -> decltype(f(init, std::declval<T>())) {
        auto accum = init;
        while (true) {
            const auto maybe_x = self().next();
            if (!maybe_x) {
                return accum;
            }

            const auto maybe_accum = f(accum, *maybe_x);
            if (!maybe_accum) {
                return maybe_accum;
            }
            accum = *maybe_accum;
        }
    }

    template <template <typename...> typename Collection, typename... Ts>
    Collection<T, Ts...> collect() {
        return collect<Collection<T, Ts...>>();
    }

    template <typename Collection>
    Collection collect() {
        Collection ret;
        while (true) {
            const auto maybe_x = self().next();
            if (maybe_x) {
                ret.push_back(*maybe_x);
            } else {
                return ret;
            }
        }
    }

    template <typename F>
    Map<F, Self> map(F&& f) && {
        return Map<F, Self>{std::forward<F>(f), self()};
    }

    template <typename F>
    Map<F, Self&> map(F&& f) & {
        return Map<F, Self&>{std::forward<F>(f), self()};
    }

    template <typename F>
    Filter<F, Self> filter(F&& f) && {
        return Filter<F, Self>{std::forward<F>(f), self()};
    }

    template <typename F>
    Filter<F, Self&> filter(F&& f) & {
        return Filter<F, Self&>{std::forward<F>(f), self()};
    }

private:
    const Self& self() const { return *static_cast<const Self*>(this); }
    Self& self() { return *static_cast<Self*>(this); }
};

// ------------------------------------ Map ------------------------------------

template <typename Function, typename BaseIterator, typename Enable>
class Map : public Iterator<
                Map<Function, BaseIterator>,
                std::invoke_result_t<Function, typename std::remove_reference_t<
                                                   BaseIterator>::value_type>> {
    using BaseNoRef = std::remove_reference_t<BaseIterator>;
    using T = typename BaseNoRef::value_type;

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

private:
    BaseIterator base_iterator_;
    Function function_;
};

template <typename Function, typename BaseIterator>
class Map<Function, BaseIterator,
          std::enable_if_t<detail::is_double_ended<
              std::remove_reference_t<BaseIterator>>::value>>
    : public Iterator<
          Map<Function, BaseIterator>,
          std::invoke_result_t<Function, typename std::remove_reference_t<
                                             BaseIterator>::value_type>>,
      public DoubleEndedIterator<
          Map<Function, BaseIterator>,
          std::invoke_result_t<Function, typename std::remove_reference_t<
                                             BaseIterator>::value_type>> {
    using BaseNoRef = std::remove_reference_t<BaseIterator>;
    using T = typename BaseNoRef::value_type;

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

    std::optional<value_type> next() {
        auto first = base_iterator_.next();
        if (first) {
            return function_(first.value());
        }
        return {};
    }

    std::optional<value_type> next_back() {
        auto first = base_iterator_.next_back();
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

    template <typename B, typename F>
    auto rfold(const B init, F f) -> B {
        return base_iterator_.rfold(init,
                                    [&f, this](const auto acc, const auto x) {
                                        return f(acc, function_(x));
                                    });
    }

    template <typename B, typename F>
    auto try_rfold(const B init, F f) -> decltype(f(init, std::declval<T>())) {
        return base_iterator_.try_rfold(
            init, [&f, this](const auto acc, const auto x) {
                return f(acc, function_(x));
            });
    }

private:
    BaseIterator base_iterator_;
    Function function_;
};

// ---------------------------------- Filter ----------------------------------

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

#endif  // ITERATORS_BASE_HPP
