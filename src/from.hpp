#ifndef ITERATORS_INTO_HPP
#define ITERATORS_INTO_HPP

#include "base.hpp"
#include "detail/collection_iterator.hpp"
#include "detail/owning_collection_iterator.hpp"
#include "detail/traits.hpp"

namespace iter {

template <typename Collection>
auto from(Collection&& collection)
    -> std::enable_if_t<
        !std::is_rvalue_reference<Collection&&>::value,
        detail::CollectionIterator<std::remove_reference_t<Collection>>> {
    return {std::begin(collection), std::end(collection)};
}

template <typename Collection>
auto from(Collection&& collection)
    -> std::enable_if_t<
        std::is_rvalue_reference<Collection&&>::value,
        detail::OwningCollectionIterator<std::remove_reference_t<Collection>>> {
    return {std::move(collection)};
}

}  // namespace iter

#endif  // ITERATORS_INTO_HPP
