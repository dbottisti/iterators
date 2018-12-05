#ifndef ZIP_HPP
#define ZIP_HPP

#include "iterator.hpp"

template <typename FirstIterator, typename SecondIterator>
class ZipIterator
    : public Iterator<std::pair<typename FirstIterator::value_type,
                                typename SecondIterator::value_type>,
                      ZipIterator<FirstIterator, SecondIterator>> {
 public:
  using value_type = std::pair<typename FirstIterator::value_type,
                               typename SecondIterator::value_type>;

  ZipIterator(FirstIterator first_iterator, SecondIterator second_iterator)
      : first_iterator_{std::move(first_iterator)},
        second_iterator_{std::move(second_iterator)} {}

  std::optional<value_type> next() {
    auto first = first_iterator_.next();
    if (first.has_value()) {
      auto second = second_iterator_.next();
      if (second.has_value()) {
        return std::make_pair(first.value(), second.value());
      }
    }
    return std::nullopt;
  }

 private:
  FirstIterator first_iterator_;
  SecondIterator second_iterator_;
};

template <typename SecondIterator>
struct ZipProxy {
  SecondIterator second_iterator;
};

template <typename SecondIterator>
ZipProxy<SecondIterator> zip(SecondIterator&& second_iterator) {
  return {std::forward<SecondIterator>(second_iterator)};
}

template <typename FirstIterator, typename SecondIterator>
ZipIterator<std::remove_reference_t<FirstIterator>,
            std::remove_reference_t<SecondIterator>>
operator|(FirstIterator&& first_iterator,
          ZipProxy<SecondIterator>&& zip_proxy) {
  return {std::forward<FirstIterator>(first_iterator),
          std::move(zip_proxy.second_iterator)};
}

#endif
