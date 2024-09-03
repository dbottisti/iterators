#ifndef ITERATORS_DOUBLE_ENDED_HPP
#define ITERATORS_DOUBLE_ENDED_HPP

namespace iter {

template <typename Self, typename T>
class DoubleEndedIterator {
public:
    static constexpr bool is_double_ended_iterator = true;

    template <typename B, typename F>
    B rfold(const B init, F f) {
        auto accum = init;
        while (true) {
            const auto maybe_x = self().next_back();
            if (!maybe_x) {
                return accum;
            }
            accum = f(accum, *maybe_x);
        }
    }

    template <typename B, typename F>
    auto try_rfold(const B init, F f) -> decltype(f(init, std::declval<T>())) {
        auto accum = init;
        while (true) {
            const auto maybe_x = self().next_back();
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

    std::size_t advance_back_by(const std::size_t n) {
        for (auto i = 0; i < n; ++i) {
            if (!self().next_back()) {
                return n - i;
            }
        }
        return 0;
    }

    std::optional<T> nth_back(const std::size_t n) {
        if (self().advance_back_by(n) == 0) {
            return self().next_back();
        }
        return std::nullopt;
    }

private:
    const Self& self() const { return *static_cast<const Self*>(this); }
    Self& self() { return *static_cast<Self*>(this); }
};

}  // namespace iter

#endif  // ITERATORS_DOUBLE_ENDED_HPP
