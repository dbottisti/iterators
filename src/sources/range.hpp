#ifndef ITERATOR_SOURCES_RANGE_HPP
#define ITERATOR_SOURCES_RANGE_HPP

#include <optional>

#include "base.hpp"

namespace iter {

template <typename T>
class Range : public Iterator<Range<T>, T> {
public:
    Range(const T begin) : value_{std::move(begin)}, step_{1} {}
    Range(const T begin, const T end, const T step)
        : value_{std::move(begin)},
          end_{std::move(end)},
          step_{std::move(step)} {}

    std::optional<T> next() {
        if (latched_ || (end_ && value_ >= *end_)) {
            latched_ = true;
            return std::nullopt;
        }

        const auto next_value = value_;
        value_ += step_;
        return next_value;
    }

private:
    T value_;
    std::optional<T> end_;
    T step_;
    bool latched_ = false;
};

template <typename T>
Range<T> range(const T begin) {
    return Range<T>{begin};
}

template <typename T>
Range<T> range(const T begin, const T end, const T step = 1) {
    return Range<T>{begin, end, step};
}

}  // namespace iter

#endif  // ITERATOR_SOURCES_RANGE_HPP
