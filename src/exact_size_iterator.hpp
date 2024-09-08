#ifndef ITERATOR_EXACT_SIZE_ITERATOR_HPP
#define ITERATOR_EXACT_SIZE_ITERATOR_HPP

#include <type_traits>

#include "declarations.hpp"
#include "detail/traits.hpp"

namespace iter {

template <typename Self, typename T>
class ExactSizeIterator {
public:
    static constexpr bool is_exact_size_iterator = true;

    // Provided methods

    std::size_t size() const { return self().size_hint().first; }
    bool is_empty() const { return self().size() == 0; }

private:
    const Self& self() const { return *static_cast<const Self*>(this); }
    Self& self() { return *static_cast<Self*>(this); }
};

}  // namespace iter

#endif  // ITERATOR_EXACT_SIZE_ITERATOR_HPP
