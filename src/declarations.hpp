#ifndef ITERATOR_DECLARATIONS_HPP
#define ITERATOR_DECLARATIONS_HPP

// Forward (and otherwise) declarations of necessary types

namespace iter {

template <typename Self, typename T>
class Iterator;

template <typename Function, typename BaseIterator>
class Map;

template <typename Predicate, typename BaseIterator>
class Filter;

template <typename BaseIterator>
class Take;

template <typename Self, typename T>
class DoubleEndedIterator;

template <typename Self, typename T>
class ExactSizeIterator;

}  // namespace iter

#endif  // ITERATOR_DECLARATIONS_HPP
