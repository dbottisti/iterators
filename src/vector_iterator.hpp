#ifndef VECTOR_ITERATOR_HPP
#define VECTOR_ITERATOR_HPP

#include "iterator.hpp"

template <typename T>
class VectorIterator : public Iterator<T, VectorIterator<T>> {
 public:
  explicit VectorIterator(std::vector<T>&& values)
      : values_{std::move(values)} {}

  std::optional<T> next() {
    if (itr_ == end_) {
      return std::nullopt;
    } else {
      return *itr_++;
    }
  }

  std::pair<size_t, std::optional<size_t>> size_hint() const {
    const auto num_elements = std::distance(itr_, end_);
    return {num_elements, num_elements};
  }

 private:
  std::vector<T> values_;
  typename std::vector<T>::const_iterator itr_ = values_.cbegin();
  typename std::vector<T>::const_iterator end_ = values_.cend();
};

template <typename T>
VectorIterator<T> iter(std::vector<T>&& v) {
  return VectorIterator<T>{std::move(v)};
}

#endif
