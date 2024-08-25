#ifndef ITERATORS_BASE_HPP
#define ITERATORS_BASE_HPP

namespace iter {

template <typename Self, typename T>
class Iterator {
public:
    std::size_t count() {
        return self().fold(0, [](const auto acc, const auto) { return acc + 1; });
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
        Collection<T, Ts...> ret;
        while (true) {
            const auto maybe_x = self().next();
            if (maybe_x) {
                ret.push_back(*maybe_x);
            } else {
                return ret;
            }
        }
    }

private:
    const Self& self() const { return *static_cast<const Self*>(this); }
    Self& self() { return *static_cast<Self*>(this); }
};

}  // namespace iter

#endif  // ITERATORS_BASE_HPP
