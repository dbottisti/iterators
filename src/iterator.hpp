#ifndef ITERATORS_ITERATOR_HPP
#define ITERATORS_ITERATOR_HPP

#include "base.hpp"
#include "double_ended.hpp"
#include "from.hpp"

template <typename T, typename Self>
class Iterator {
public:
    using value_type = T;

    virtual std::optional<T> next() = 0;

    virtual std::pair<size_t, std::optional<size_t>> size_hint() const {
        return {0, std::nullopt};
    }

    virtual size_t count() {
        auto count = 0;
        while (next().has_value()) {
            ++count;
        }
        return count;
    }

    virtual std::optional<T> last() {
        std::optional<T> current_last;
        std::optional<T> current_next = next();
        while (current_next) {
            current_last = current_next;
            current_next = next();
        }
        return current_last;
    }

    virtual std::optional<T> nth(const size_t n) {
        std::optional<T> current_next = next();
        for (auto i = 0; i < n && current_next.has_value(); ++i) {
            current_next = next();
        }
        return current_next;
    }
};

#endif  // ITERATORS_ITERATOR_HPP
