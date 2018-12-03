#ifndef CHAIN_HPP
#define CHAIN_HPP

#include "iterator.hpp"

template <typename FirstIterator, typename SecondIterator>
class ChainIterator {
  static_assert(std::is_same_v<typename FirstIterator::value_type,
                               typename SecondIterator::value_type>,
                "Only iterators producing the same types can be chained");

 public:
  using value_type = typename FirstIterator::value_type;

  ChainIterator(FirstIterator first_iterator, SecondIterator second_iterator)
      : first_iterator_{std::move(first_iterator)},
        second_iterator_{std::move(second_iterator)} {}

  std::optional<value_type> next() {
    if (!first_done_) {
      auto result = first_iterator_.next();
      if (result.has_value()) {
        return result;
      }
      first_done_ = true;
    }

    return second_iterator_.next();
  }

 private:
  FirstIterator first_iterator_;
  SecondIterator second_iterator_;
  bool first_done_ = false;
};

template <typename SecondIterator>
struct ChainProxy {
  SecondIterator second_iterator;
};

template <typename SecondIterator>
ChainProxy<SecondIterator> chain(SecondIterator&& second_iterator) {
  return {std::forward<SecondIterator>(second_iterator)};
}

template <typename LeftIterator, typename SecondIterator>
ChainIterator<std::remove_reference_t<LeftIterator>,
              std::remove_reference_t<SecondIterator>>
operator|(LeftIterator&& left_iterator,
          ChainProxy<SecondIterator>&& step_proxy) {
  return {std::forward<LeftIterator>(left_iterator),
          std::move(step_proxy.second_iterator)};
}

#endif
