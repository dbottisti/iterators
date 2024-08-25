#include <array>
#include <cstdint>
#include <limits>
#include <optional>

#include <catch2/catch_test_macros.hpp>

// Roadmap:
// - Collect
// - Filter
//   - rfold
//     - next_back (DoubleEndedIterator
//   - try_fold
//   - try_rfold
//     - next_back (DoubleEndedIterator)
//   - collect (into vector)
//   - next_chunk
//   - as an STL-style iterators
// - Map
//   - try_fold
//   - try_rfold
//     - next_back (DoubleEndedIterator)

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

namespace detail {

template <typename IteratorType>
class RangeIterator
    : public Iterator<RangeIterator<IteratorType>,
                      std::remove_reference_t<decltype(*std::declval<IteratorType>())>> {
public:
    using value_type = std::remove_reference_t<decltype(*std::declval<IteratorType>())>;

    RangeIterator(IteratorType current, IteratorType end) : current_{current}, end_{end} {}

    std::optional<value_type> next() {
        if (current_ == end_) {
            return std::nullopt;
        }

        return *(current_++);
    }

private:
    IteratorType current_;
    IteratorType end_;
};

template <typename Collection>
class OwningRangeIterator
    : public Iterator<OwningRangeIterator<Collection>,
                      std::remove_reference_t<decltype(*std::begin(std::declval<Collection>()))>> {
    using IteratorType = decltype(std::begin(std::declval<Collection>()));

public:
    using value_type = std::remove_reference_t<decltype(*std::declval<IteratorType>())>;

    OwningRangeIterator(Collection collection)
        : collection_{std::move(collection)},
          current_{std::begin(collection_)},
          end_{std::end(collection_)} {}

    std::optional<value_type> next() {
        if (current_ == end_) {
            return std::nullopt;
        }

        return *(current_++);
    }

private:
    Collection collection_;
    IteratorType current_;
    IteratorType end_;
};

}  // namespace detail

template <typename Collection>
auto from(Collection&& collection)
    -> std::enable_if_t<!std::is_rvalue_reference<Collection&&>::value,
                        detail::RangeIterator<decltype(std::begin(collection))>> {
    return {std::begin(collection), std::end(collection)};
}

template <typename Collection>
auto from(Collection&& collection)
    -> std::enable_if_t<std::is_rvalue_reference<Collection&&>::value,
                        detail::OwningRangeIterator<std::remove_reference_t<Collection>>> {
    return {std::move(collection)};
}

}  // namespace iter

TEST_CASE("construct from std::array reference", "[construct]") {
    std::array<std::uint32_t, 6> xs{1, 2, 3, 4, 5, 6};

    auto it = iter::from(xs);
    REQUIRE(it.next() == std::make_optional(1));
    REQUIRE(it.next() == std::make_optional(2));
    REQUIRE(it.next() == std::make_optional(3));
    REQUIRE(it.next() == std::make_optional(4));
    REQUIRE(it.next() == std::make_optional(5));
    REQUIRE(it.next() == std::make_optional(6));
    REQUIRE(it.next() == std::nullopt);
}

TEST_CASE("construct from std::array&&", "[construct]") {
    auto it = iter::from(std::array<std::uint32_t, 6>{1, 2, 3, 4, 5, 6});
    REQUIRE(it.next() == std::make_optional(1));
    REQUIRE(it.next() == std::make_optional(2));
    REQUIRE(it.next() == std::make_optional(3));
    REQUIRE(it.next() == std::make_optional(4));
    REQUIRE(it.next() == std::make_optional(5));
    REQUIRE(it.next() == std::make_optional(6));
    REQUIRE(it.next() == std::nullopt);
}

TEST_CASE("count", "[count]") {
    const auto xs = std::array<std::int32_t, 8>{1, 2, 2, 1, 5, 9, 0, 2};
    REQUIRE(iter::from(xs).count() == 8);
}

TEST_CASE("fold", "[fold]") {
    const auto xs = std::array<std::int32_t, 8>{1, 2, 2, 1, 5, 9, 0, 2};
    REQUIRE(iter::from(xs).fold(0, [](const auto acc, const auto x) { return acc + x; }) == 22);
}

TEST_CASE("collect into generic", "[collect]") {
    const auto xs = std::array<std::int32_t, 8>{1, 2, 2, 1, 5, 9, 0, 2};

    const auto collected = iter::from(xs).collect<std::vector>();
    static_assert(std::is_same_v<decltype(collected), const std::vector<const std::int32_t>>,
                  "collect does not return a vector");
    REQUIRE(collected[0] == xs[0]);
    REQUIRE(collected[1] == xs[1]);
    REQUIRE(collected[2] == xs[2]);
    REQUIRE(collected[3] == xs[3]);
    REQUIRE(collected[4] == xs[4]);
    REQUIRE(collected[5] == xs[5]);
    REQUIRE(collected[6] == xs[6]);
    REQUIRE(collected[7] == xs[7]);
}

TEST_CASE("try_fold nominal", "[try_fold]") {
    const std::array<std::int32_t, 3> a{1, 2, 3};

    const auto checked_add = [](const auto acc, const auto x) -> std::optional<decltype(acc)> {
        if (x > std::numeric_limits<decltype(acc)>::max() - acc) {
            return std::nullopt;
        }
        return std::make_optional(x + acc);
    };

    const auto sum = iter::from(a).try_fold(std::int8_t{0}, checked_add);
    REQUIRE(sum == std::make_optional<std::int8_t>(6));
}

TEST_CASE("try_fold short-circuiting", "[try_fold]") {
    const std::array<std::int32_t, 6> a{10, 20, 30, 100, 40, 50};

    const auto checked_add = [](const auto acc, const auto x) -> std::optional<decltype(acc)> {
        if (x > std::numeric_limits<decltype(acc)>::max() - acc) {
            return std::nullopt;
        }
        return std::make_optional(x + acc);
    };

    // This sum overflows when adding the 100 element
    const auto sum = iter::from(a).try_fold(std::int8_t{0}, checked_add);
}

template <typename T>
class ControlFlow {
public:
    ControlFlow(const T value) : value_{std::move(value)}, state_{State::CONTINUE} {}

    static ControlFlow Continue(const T value) {
        return ControlFlow{std::move(value), State::CONTINUE};
    }

    static ControlFlow Break(const T value) { return ControlFlow{std::move(value), State::BREAK}; }

    operator bool() const { return state_ == State::CONTINUE; }

    T operator*() const { return value_; }

private:
    enum class State { CONTINUE, BREAK };

    ControlFlow(const T value, const State state) : value_{std::move(value)}, state_{state} {}

    T value_;
    State state_;
};

#include <iostream>

TEST_CASE("try_fold can be used with any 'try'-like type", "[try_fold]") {
    const auto checked_add = [](const auto acc, const auto x) -> ControlFlow<decltype(acc)> {
        using T = decltype(acc);
        if (acc >= 0) {
            if (std::numeric_limits<T>::max() - acc < x) {
                return ControlFlow<T>::Break(acc);
            }
        } else {
            if (x < std::numeric_limits<T>::min() - acc) {
                return ControlFlow<T>::Break(acc);
            }
        }
        return ControlFlow<T>::Continue(x + acc);
    };

    const std::array<std::int8_t, 29> a{
        1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    };

    SECTION("with overflow") {
        const auto triangular = iter::from(a).try_fold(std::int8_t{0}, checked_add);
        REQUIRE(triangular == ControlFlow<std::int8_t>::Break(120));
    }

    SECTION("without overflow") {
        const auto triangular = iter::from(a).try_fold(std::int64_t{0}, checked_add);
        REQUIRE(triangular == ControlFlow<std::int64_t>::Continue(435));
    }
}
