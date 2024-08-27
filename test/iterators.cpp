#include <array>
#include <cstdint>
#include <limits>
#include <optional>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "iterator.hpp"

using Catch::Matchers::Equals;

namespace Catch {

template <typename T>
struct StringMaker<std::optional<T>> {
    static auto convert(std::optional<T> const& o) -> std::string {
        ReusableStringStream rss;
        if (o) {
            rss << "Some(" << Detail::stringify(*o) << ')';
        } else {
            rss << "None";
        }
        return rss.str();
    }
};

template <typename T, typename U>
struct StringMaker<std::pair<T, U>> {
    static auto convert(std::pair<T, U> const& pair) -> std::string {
        ReusableStringStream rss;
        rss << "(" << Detail::stringify(pair.first) << ", "
            << Detail::stringify(pair.second) << ")";
        return rss.str();
    }
};

}  // namespace Catch

const auto checked_add
    = [](const auto acc, const auto x) -> std::optional<decltype(acc)> {
    using T = decltype(acc);
    if (acc >= 0) {
        if (std::numeric_limits<T>::max() - acc < x) {
            return std::nullopt;
        }
    } else {
        if (x < std::numeric_limits<T>::min() - acc) {
            return std::nullopt;
        }
    }
    return std::make_optional(x + acc);
};

// Roadmap:
// - Filter
//   - next_chunk
//   - as an STL-style iterators

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
    REQUIRE(iter::from(xs).fold(0, [](const auto acc, const auto x) {
        return acc + x;
    }) == 22);
}

TEST_CASE("collect into generic", "[collect]") {
    const auto xs = std::array<std::int32_t, 8>{1, 2, 2, 1, 5, 9, 0, 2};

    const auto collected = iter::from(xs).collect<std::vector>();
    static_assert(std::is_same_v<decltype(collected),
                                 const std::vector<const std::int32_t>>,
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

    const auto sum = iter::from(a).try_fold(std::int8_t{0}, checked_add);
    REQUIRE(sum == std::make_optional<std::int8_t>(6));
}

TEST_CASE("try_fold short-circuiting", "[try_fold]") {
    const std::array<std::int32_t, 6> a{10, 20, 30, 100, 40, 50};

    // This sum overflows when adding the 100 element
    const auto sum = iter::from(a).try_fold(std::int8_t{0}, checked_add);
}

template <typename T>
class ControlFlow {
public:
    ControlFlow(const T value)
        : value_{std::move(value)}, state_{State::CONTINUE} {}

    static ControlFlow Continue(const T value) {
        return ControlFlow{std::move(value), State::CONTINUE};
    }

    static ControlFlow Break(const T value) {
        return ControlFlow{std::move(value), State::BREAK};
    }

    operator bool() const { return state_ == State::CONTINUE; }

    T operator*() const { return value_; }

private:
    enum class State { CONTINUE, BREAK };

    ControlFlow(const T value, const State state)
        : value_{std::move(value)}, state_{state} {}

    T value_;
    State state_;
};

TEST_CASE("try_fold can be used with any 'try'-like type", "[try_fold]") {
    const auto checked_add
        = [](const auto acc, const auto x) -> ControlFlow<decltype(acc)> {
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
        const auto triangular
            = iter::from(a).try_fold(std::int8_t{0}, checked_add);
        REQUIRE(triangular == ControlFlow<std::int8_t>::Break(120));
    }

    SECTION("without overflow") {
        const auto triangular
            = iter::from(a).try_fold(std::int64_t{0}, checked_add);
        REQUIRE(triangular == ControlFlow<std::int64_t>::Continue(435));
    }
}

TEST_CASE("next_back", "[next_back]") {
    const std::array<std::int32_t, 6> a{1, 2, 3, 4, 5, 6};

    auto itr = iter::from(a);

    REQUIRE(itr.next() == std::make_optional(1));
    REQUIRE(itr.next_back() == std::make_optional(6));
    REQUIRE(itr.next_back() == std::make_optional(5));
    REQUIRE(itr.next() == std::make_optional(2));
    REQUIRE(itr.next() == std::make_optional(3));
    REQUIRE(itr.next() == std::make_optional(4));
    REQUIRE(itr.next() == std::nullopt);
    REQUIRE(itr.next_back() == std::nullopt);
}

TEST_CASE("next_back (owning)", "[next_back]") {
    auto itr = iter::from(std::array<std::int32_t, 6>{1, 2, 3, 4, 5, 6});

    REQUIRE(itr.next() == std::make_optional(1));
    REQUIRE(itr.next_back() == std::make_optional(6));
    REQUIRE(itr.next_back() == std::make_optional(5));
    REQUIRE(itr.next() == std::make_optional(2));
    REQUIRE(itr.next() == std::make_optional(3));
    REQUIRE(itr.next() == std::make_optional(4));
    REQUIRE(itr.next() == std::nullopt);
    REQUIRE(itr.next_back() == std::nullopt);
}

TEST_CASE("rfold", "[rfold]") {
    const std::array<std::int32_t, 3> a{1, 2, 3};
    const auto sum = iter::from(a).rfold(
        0, [](const auto acc, const auto x) { return acc + x; });

    REQUIRE(sum == 6);
}

TEST_CASE("rfold is right associative", "[rfold]") {
    const std::array<std::int32_t, 5> a{1, 2, 3, 4, 5};

    const auto result = iter::from(a).rfold(
        std::to_string(0),
        [](const auto acc, const auto x) { return acc + std::to_string(x); });

    REQUIRE(result == "054321");
}

TEST_CASE("try_rfold", "[try_rfold]") {
    const std::array<std::int8_t, 3> a{1, 2, 3};
    const auto sum = iter::from(a).try_rfold(std::int8_t{0}, checked_add);

    REQUIRE(sum == std::make_optional(6));
}

TEST_CASE("try_rfold short-circuiting", "[try_rfold]") {
    const std::array<std::int8_t, 6> a{10, 20, 30, 100, 40, 50};

    auto it = iter::from(a);
    const auto sum = it.try_rfold(std::int8_t{0}, checked_add);

    REQUIRE(sum == std::nullopt);

    REQUIRE(it.next_back() == std::make_optional(30));
}

TEST_CASE("map basic", "[map]") {
    const auto v = iter::from(std::array<std::int8_t, 3>{1, 2, 3})
                       .map([](const auto x) { return x + 1; })
                       .collect<std::vector<const std::int8_t>>();

    REQUIRE_THAT(v, Equals(std::vector<const std::int8_t>{2, 3, 4}));
}

TEST_CASE("map with state", "[map]") {
    std::uint32_t c = 0;
    const auto v = iter::from(std::array<char, 3>{'a', 'b', 'c'})
                       .map([&](const auto letter) {
                           c += 1;
                           return std::make_pair(letter, c);
                       })
                       .collect<std::vector>();

    REQUIRE_THAT(v, Equals(std::vector<std::pair<char, std::uint32_t>>{
                        {'a', 1}, {'b', 2}, {'c', 3}}));
}

TEST_CASE("map try_folds", "[map][try_fold]") {
    SECTION("Without overflow") {
        const std::array<std::int8_t, 3> a{1, 2, 3};
        REQUIRE(iter::from(a)
                    .map([](const auto x) { return x * 10; })
                    .try_fold(std::int8_t{0}, checked_add)
                == std::make_optional(60));
    }

    SECTION("With overflow") {
        const std::array<std::int8_t, 6> a{1, 2, 3, 10, 4, 5};
        REQUIRE(iter::from(a)
                    .map([](const auto x) { return x * 10; })
                    .try_fold(std::int8_t{0}, checked_add)
                == std::nullopt);
    }

    SECTION("continuing after overflow") {
        const std::array<std::int8_t, 40> a{
            0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
            14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
            28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
        };
        auto it = iter::from(a).map([](const auto x) { return x + 10; });
        REQUIRE(it.try_fold(std::int8_t{0}, checked_add) == std::nullopt);
        REQUIRE(it.next() == std::make_optional(20));
        REQUIRE(it.try_rfold(std::int8_t{0}, checked_add) == std::nullopt);
        REQUIRE(it.next_back() == std::make_optional(46));
    }
}

TEST_CASE("double-ended map", "[map][next_back]") {
    auto itr = iter::from(std::array<std::int32_t, 6>{1, 2, 3, 4, 5, 6})
                   .map([](const auto x) { return x * -1; });

    REQUIRE(itr.next() == std::make_optional(-1));
    REQUIRE(itr.next_back() == std::make_optional(-6));
    REQUIRE(itr.next_back() == std::make_optional(-5));
    REQUIRE(itr.next() == std::make_optional(-2));
    REQUIRE(itr.next() == std::make_optional(-3));
    REQUIRE(itr.next() == std::make_optional(-4));
    REQUIRE(itr.next() == std::nullopt);
    REQUIRE(itr.next_back() == std::nullopt);
}

TEST_CASE("filter count", "[filter][count]") {
    const std::array<std::int8_t, 9> xs{0, 1, 2, 3, 4, 5, 6, 7, 8};

    REQUIRE(
        iter::from(xs).filter([](const auto x) { return x % 2 == 0; }).count()
        == 5);
}

TEST_CASE("filter fold", "[filter][fold]") {
    const std::array<std::int8_t, 9> xs{0, 1, 2, 3, 4, 5, 6, 7, 8};
    const std::array<std::int8_t, 5> ys{0, 2, 4, 6, 8};

    SECTION("forward") {
        auto it
            = iter::from(xs).filter([](const auto x) { return x % 2 == 0; });
        const auto i = it.fold(0, [&ys](const auto i, const auto x) {
            REQUIRE(x == ys[i]);
            return i + 1;
        });
        REQUIRE(i == ys.size());
    }

    SECTION("backwards") {
        auto it
            = iter::from(xs).filter([](const auto x) { return x % 2 == 0; });
        const auto i = it.rfold(ys.size(), [&ys](const auto i, const auto x) {
            REQUIRE(x == ys[i - 1]);
            return i - 1;
        });
        REQUIRE(i == 0);
    }
}
