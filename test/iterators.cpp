#include <array>
#include <cstdint>
#include <optional>

#include <catch2/catch_test_macros.hpp>

// Roadmap:
// - Collect
// - Filter
//   - From Array
//   - count
//   - fold
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

namespace detail {

template <typename IteratorType>
class RangeIterator {
public:
    using Item = std::remove_reference_t<decltype(*std::declval<IteratorType>())>;

    RangeIterator(IteratorType current, IteratorType end) : current_{current}, end_{end} {}

    std::optional<Item> next() {
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
class OwningRangeIterator {
    using IteratorType = decltype(std::begin(std::declval<Collection>()));

public:
    using Item = std::remove_reference_t<decltype(*std::declval<IteratorType>())>;

    OwningRangeIterator(Collection collection)
        : collection_{std::move(collection)},
          current_{std::begin(collection_)},
          end_{std::end(collection_)} {}

    std::optional<Item> next() {
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
