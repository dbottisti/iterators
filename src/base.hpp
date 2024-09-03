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

    std::size_t advance_by(const size_t n) {
        for (auto i = 0; i < n; ++i) {
            if (!self().next()) {
                return n - i;
            }
        }
        return 0;
    }

private:
    const Self& self() const { return *static_cast<const Self*>(this); }
    Self& self() { return *static_cast<Self*>(this); }
};

}  // namespace iter

#include "adapters/filter.hpp"
#include "adapters/map.hpp"

#endif  // ITERATORS_BASE_HPP
