#ifndef ITERATOR_SOURCES_RANGE_HPP
#define ITERATOR_SOURCES_RANGE_HPP

#include <cstdint>
#include <optional>

#include "base.hpp"

namespace iter {

template <typename T>
class Range : public Iterator<Range<T>, T> {
public:
    using value_type = T;

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

    std::pair<std::size_t, std::optional<std::size_t>> size_hint() const {
        return {std::numeric_limits<std::size_t>::max(), {}};
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
