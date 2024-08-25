#ifndef ITERATORS_INTO_HPP
#define ITERATORS_INTO_HPP

#include "base.hpp"

namespace iter {

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

#endif  // ITERATORS_INTO_HPP
