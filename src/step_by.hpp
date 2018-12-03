#ifndef STEP_BY_HPP
#define STEP_BY_HPP

#include "iterator.hpp"

template <typename BaseIterator>
class StepByIterator {
 public:
  using value_type = typename BaseIterator::value_type;

  StepByIterator(BaseIterator base_iterator, size_t skip)
      : base_iterator_{std::move(base_iterator)}, skip_{skip} {}

  std::optional<value_type> next() {
    const auto result = base_iterator_.next();
    for (auto i = 1; i < skip_; ++i) {
      base_iterator_.next();
    }
    return result;
  }

 private:
  BaseIterator base_iterator_;
  size_t skip_;
};

struct StepByProxy {
  size_t step;
};

StepByProxy step_by(const size_t step) { return {step}; }

template <typename LeftIterator>
StepByIterator<std::remove_reference_t<LeftIterator>> operator|(
    LeftIterator&& left_iterator, const StepByProxy step_by_proxy) {
  return {std::forward<LeftIterator>(left_iterator), step_by_proxy.step};
}

#endif
