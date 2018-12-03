#include <optional>
#include <type_traits>
#include <vector>

template <typename BaseIterator>
class StepByIterator;

template <typename T, typename Self>
class Iterator {
 public:
  using value_type = T;

  virtual std::optional<T> next() = 0;

  virtual std::pair<size_t, std::optional<size_t>> size_hint() const {
    return {0, std::nullopt};
  }

  virtual size_t count() {
    auto count = 0;
    while (next().has_value()) {
      ++count;
    }
    return count;
  }

  virtual std::optional<T> last() {
    std::optional<T> current_last;
    std::optional<T> current_next = next();
    while (current_next) {
      current_last = current_next;
      current_next = next();
    }
    return current_last;
  }

  virtual std::optional<T> nth(const size_t n) {
    std::optional<T> current_next = next();
    for (auto i = 0; i < n && current_next.has_value(); ++i) {
      current_next = next();
    }
    return current_next;
  }
};

//==============================================================================

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

//==============================================================================

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

//==============================================================================

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

//------------------------------------------------------------------------------

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::A;
using ::testing::AllOf;
using ::testing::Eq;
using ::testing::Matcher;
using ::testing::Not;
using ::testing::Pair;
using ::testing::Property;
using ::testing::Truly;

template <typename T>
Matcher<std::optional<T>> NullOpt() {
  return Property(&std::optional<T>::has_value, Eq(false));
}

template <typename T>
Matcher<std::optional<T>> Optional(const T& expected) {
  return AllOf(Not(NullOpt<T>()), Truly([&expected](const auto actual) {
                 return actual.value() == expected;
               }));
}

TEST(ThatIter, createsAVectorIterator) {
  EXPECT_THAT(iter(std::vector<uint32_t>{}), A<VectorIterator<uint32_t>>());
}

class ThatVectorIterator : public ::testing::Test {
 protected:
  VectorIterator<uint32_t> empty_vector_iter_ = iter(std::vector<uint32_t>{});
  VectorIterator<uint32_t> vector_iter_ =
      iter(std::vector<uint32_t>{1, 2, 3, 4});
};

TEST_F(ThatVectorIterator, nextReturnsNulloptIfEmpty) {
  EXPECT_THAT(empty_vector_iter_.next(), NullOpt<uint32_t>());
}

TEST_F(ThatVectorIterator, nextReturnsFirstValue) {
  EXPECT_THAT(vector_iter_.next(), Optional<uint32_t>(1));
}

TEST_F(ThatVectorIterator, repeatedNextReturnsAllValuesAndNullopt) {
  EXPECT_THAT(vector_iter_.next(), Optional<uint32_t>(1));
  EXPECT_THAT(vector_iter_.next(), Optional<uint32_t>(2));
  EXPECT_THAT(vector_iter_.next(), Optional<uint32_t>(3));
  EXPECT_THAT(vector_iter_.next(), Optional<uint32_t>(4));
  EXPECT_THAT(vector_iter_.next(), NullOpt<uint32_t>());
}

using ThatVectorIteratorSizeHint = ThatVectorIterator;

TEST_F(ThatVectorIteratorSizeHint, returns4And4For4Elements) {
  EXPECT_THAT(vector_iter_.size_hint(), Pair(4, Optional<size_t>(4)));
}

TEST_F(ThatVectorIteratorSizeHint, returns3And3AfterANext) {
  vector_iter_.next();
  EXPECT_THAT(vector_iter_.size_hint(), Pair(3, Optional<size_t>(3)));
}

using ThatVectorIteratorCount = ThatVectorIterator;

TEST_F(ThatVectorIteratorCount, returns4Initially) {
  EXPECT_THAT(vector_iter_.count(), Eq(4));
}

TEST_F(ThatVectorIteratorCount, returns3AfterNext) {
  vector_iter_.next();
  EXPECT_THAT(vector_iter_.count(), Eq(3));
}

using ThatVectorIteratorLast = ThatVectorIterator;

TEST_F(ThatVectorIteratorLast, returns4IfValuesRemain) {
  EXPECT_THAT(vector_iter_.last(), Optional<uint32_t>(4));
}

TEST_F(ThatVectorIteratorLast, returns4AfterANext) {
  vector_iter_.next();
  EXPECT_THAT(vector_iter_.last(), Optional<uint32_t>(4));
}

TEST_F(ThatVectorIteratorLast, returnsNulloptAfterConsumed) {
  vector_iter_.count();
  EXPECT_THAT(vector_iter_.last(), NullOpt<uint32_t>());
}

using ThatVectorIteratorNth = ThatVectorIterator;

TEST_F(ThatVectorIteratorNth, returnsSecondItem) {
  EXPECT_THAT(vector_iter_.nth(1), Optional<uint32_t>(2));
}

TEST_F(ThatVectorIteratorNth, returnsNulloptIfNGreaterOrEqualToSize) {
  EXPECT_THAT(vector_iter_.nth(4), NullOpt<uint32_t>());
  EXPECT_THAT(vector_iter_.nth(5), NullOpt<uint32_t>());
}

TEST_F(ThatVectorIteratorNth, calledMultipleTimesDoesntRewind) {
  EXPECT_THAT(vector_iter_.nth(1), Optional<uint32_t>(2));
  EXPECT_THAT(vector_iter_.nth(1), Optional<uint32_t>(4));
  EXPECT_THAT(vector_iter_.nth(1), NullOpt<uint32_t>());
}

class ThatStepByIteratorNext : public ThatVectorIterator {
 public:
  decltype(vector_iter_ | step_by(2)) step_by_iter_ = vector_iter_ | step_by(2);
};

TEST_F(ThatStepByIteratorNext, returnsFirstValue) {
  EXPECT_THAT(step_by_iter_.next(), Optional<uint32_t>(1));
}

TEST_F(ThatStepByIteratorNext, skipsValueOnRepeatedNext) {
  EXPECT_THAT(step_by_iter_.next(), Optional<uint32_t>(1));
  EXPECT_THAT(step_by_iter_.next(), Optional<uint32_t>(3));
  EXPECT_THAT(step_by_iter_.next(), NullOpt<uint32_t>());
}

using ThatChainIteratorNext = ThatVectorIterator;

TEST_F(ThatChainIteratorNext, returnsEntireSequence) {
  auto vector_iter_2 = iter(std::vector<uint32_t>{5, 6, 7, 8});
  auto chain_iter = vector_iter_ | chain(vector_iter_2);
  EXPECT_THAT(chain_iter.next(), Optional<uint32_t>(1));
  EXPECT_THAT(chain_iter.next(), Optional<uint32_t>(2));
  EXPECT_THAT(chain_iter.next(), Optional<uint32_t>(3));
  EXPECT_THAT(chain_iter.next(), Optional<uint32_t>(4));
  EXPECT_THAT(chain_iter.next(), Optional<uint32_t>(5));
  EXPECT_THAT(chain_iter.next(), Optional<uint32_t>(6));
  EXPECT_THAT(chain_iter.next(), Optional<uint32_t>(7));
  EXPECT_THAT(chain_iter.next(), Optional<uint32_t>(8));
  EXPECT_THAT(chain_iter.next(), NullOpt<uint32_t>());
}
