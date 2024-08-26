#ifndef ITERATORS_DOUBLE_ENDED_HPP
#define ITERATORS_DOUBLE_ENDED_HPP

namespace iter {

template <typename Self, typename T>
class DoubleEndedIterator {
public:
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

private:
    const Self& self() const { return *static_cast<const Self*>(this); }
    Self& self() { return *static_cast<Self*>(this); }
};

}  // namespace iter

#endif  // ITERATORS_DOUBLE_ENDED_HPP
