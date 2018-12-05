#ifndef MAP_HPP
#define MAP_HPP

template <typename Function, typename BaseIterator>
class MapIterator
    : public Iterator<
          std::invoke_result_t<Function, typename BaseIterator::value_type>,
          MapIterator<Function, BaseIterator>> {
 public:
  static_assert(
      std::is_invocable_v<Function, typename BaseIterator::value_type>,
      "Function provided to map() cannot be called with values of the incoming "
      "type");
  using value_type =
      std::invoke_result_t<Function, typename BaseIterator::value_type>;

  MapIterator(BaseIterator base_iterator, Function function)
      : base_iterator_{std::move(base_iterator)},
        function_{std::move(function)} {}

  std::optional<value_type> next() {
    auto first = base_iterator_.next();
    if (first) {
      return function_(first.value());
    }
    return {};
  }

 private:
  BaseIterator base_iterator_;
  Function function_;
};

template <typename Function>
struct MapProxy {
  Function function;
};

template <typename Function>
MapProxy<Function> map(Function&& function) {
  return {std::forward<Function>(function)};
}

template <typename Function, typename BaseIterator>
MapIterator<std::remove_reference_t<Function>,
            std::remove_reference_t<BaseIterator>>
operator|(BaseIterator&& base_iterator, MapProxy<Function>&& map_proxy) {
  return {std::forward<BaseIterator>(base_iterator),
          std::move(map_proxy.function)};
}

#endif
