#include <array>
#include <cstdint>
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
        std::size_t ret = 0;
        while (self().next().has_value()) {
            ret++;
        }
        return ret;
    }

    template <typename B, typename F>
    B fold(const B init, F f) {
        auto accum = init;
        while (true) {
            const auto maybe_x = self().next();
            if (!maybe_x.has_value()) {
                return accum;
            }
            accum = f(accum, *maybe_x);
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
