#ifndef ITERATORS_DETAIL_OWNING_COLLECTION_ITERATOR_HPP
#define ITERATORS_DETAIL_OWNING_COLLECTION_ITERATOR_HPP

#include "base.hpp"
#include "detail/collection_iterator.hpp"

namespace iter {
namespace detail {

template <typename Collection>
class OwningCollectionIterator : public CollectionIterator<Collection> {
    using Super = CollectionIterator<Collection>;

public:
    OwningCollectionIterator(Collection collection)
        : collection_{std::move(collection)},
          Super{std::begin(collection_), std::end(collection_)} {}

private:
    Collection collection_;
};

}  // namespace detail
}  // namespace iter

#endif  // ITERATORS_DETAIL_OWNING_COLLECTION_ITERATOR_HPP
