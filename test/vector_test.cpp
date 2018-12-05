// System includes
#include <optional>
#include <type_traits>
#include <vector>

// Library includes
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Local includes
#include "chain.hpp"
#include "iterator.hpp"
#include "map.hpp"
#include "step_by.hpp"
#include "vector_iterator.hpp"
#include "zip.hpp"

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

class ThatVectorIteratorNext : public ::testing::Test {
 protected:
  VectorIterator<uint32_t> empty_vector_iter_ = iter(std::vector<uint32_t>{});
  VectorIterator<uint32_t> vector_iter_ =
      iter(std::vector<uint32_t>{1, 2, 3, 4});
};

TEST_F(ThatVectorIteratorNext, returnsNulloptIfEmpty) {
  EXPECT_THAT(empty_vector_iter_.next(), NullOpt<uint32_t>());
}

TEST_F(ThatVectorIteratorNext, returnsFirstValue) {
  EXPECT_THAT(vector_iter_.next(), Optional<uint32_t>(1));
}

TEST_F(ThatVectorIteratorNext, returnsMiddleValueIfAlreadyCalled) {
  vector_iter_.next();
  EXPECT_THAT(vector_iter_.next(), Optional<uint32_t>(2));
}

TEST_F(ThatVectorIteratorNext, returnsNulloptWhenConsumed) {
  vector_iter_.next();
  vector_iter_.next();
  vector_iter_.next();
  vector_iter_.next();
  EXPECT_THAT(vector_iter_.next(), NullOpt<uint32_t>());
}

using ThatVectorIteratorSizeHint = ThatVectorIteratorNext;

TEST_F(ThatVectorIteratorSizeHint, returns4And4For4Elements) {
  EXPECT_THAT(vector_iter_.size_hint(), Pair(4, Optional<size_t>(4)));
}

TEST_F(ThatVectorIteratorSizeHint, returns3And3AfterANext) {
  vector_iter_.next();
  EXPECT_THAT(vector_iter_.size_hint(), Pair(3, Optional<size_t>(3)));
}

using ThatVectorIteratorCount = ThatVectorIteratorNext;

TEST_F(ThatVectorIteratorCount, returns4Initially) {
  EXPECT_THAT(vector_iter_.count(), Eq(4));
}

TEST_F(ThatVectorIteratorCount, returns3AfterNext) {
  vector_iter_.next();
  EXPECT_THAT(vector_iter_.count(), Eq(3));
}

using ThatVectorIteratorLast = ThatVectorIteratorNext;

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

using ThatVectorIteratorNth = ThatVectorIteratorNext;

TEST_F(ThatVectorIteratorNth, returnsSecondItem) {
  EXPECT_THAT(vector_iter_.nth(1), Optional<uint32_t>(2));
}

TEST_F(ThatVectorIteratorNth, returnsNulloptIfNEqualToSize) {
  EXPECT_THAT(vector_iter_.nth(4), NullOpt<uint32_t>());
}

TEST_F(ThatVectorIteratorNth, returnsNulloptIfNGreaterThanSize) {
  EXPECT_THAT(vector_iter_.nth(5), NullOpt<uint32_t>());
}

TEST_F(ThatVectorIteratorNth, calledMultipleTimesDoesntRewind) {
  EXPECT_THAT(vector_iter_.nth(1), Optional<uint32_t>(2));
  EXPECT_THAT(vector_iter_.nth(1), Optional<uint32_t>(4));
  EXPECT_THAT(vector_iter_.nth(1), NullOpt<uint32_t>());
}

class ThatStepByIteratorNext : public ThatVectorIteratorNext {
 public:
  decltype(vector_iter_ | step_by(2)) step_by_iter_ = vector_iter_ | step_by(2);
};

TEST_F(ThatStepByIteratorNext, returnsFirstValue) {
  EXPECT_THAT(step_by_iter_.next(), Optional<uint32_t>(1));
}

TEST_F(ThatStepByIteratorNext, skipsValueOnRepeatedNext) {
  step_by_iter_.next();
  EXPECT_THAT(step_by_iter_.next(), Optional<uint32_t>(3));
}

TEST_F(ThatStepByIteratorNext, returnsNulloptWhenConsumed) {
  step_by_iter_.next();
  step_by_iter_.next();
  EXPECT_THAT(step_by_iter_.next(), NullOpt<uint32_t>());
}

class ThatChainIteratorNext : public ThatVectorIteratorNext {
 protected:
  VectorIterator<uint32_t> vector_iter_2 =
      iter(std::vector<uint32_t>{5, 6, 7, 8});
  decltype(vector_iter_ | chain(vector_iter_2)) chain_iter =
      vector_iter_ | chain(vector_iter_2);
};

TEST_F(ThatChainIteratorNext, returnsFirstItem) {
  EXPECT_THAT(chain_iter.next(), Optional<uint32_t>(1));
}

TEST_F(ThatChainIteratorNext,
       returnsFirstItemOfSecondSequenceWhenFirstConsumed) {
  chain_iter.next();
  chain_iter.next();
  chain_iter.next();
  chain_iter.next();
  EXPECT_THAT(chain_iter.next(), Optional<uint32_t>(5));
}

TEST_F(ThatChainIteratorNext, returnsNulloptWhenSecondConsumed) {
  chain_iter.next();
  chain_iter.next();
  chain_iter.next();
  chain_iter.next();
  chain_iter.next();
  chain_iter.next();
  chain_iter.next();
  chain_iter.next();
  EXPECT_THAT(chain_iter.next(), NullOpt<uint32_t>());
}

using ThatZipIteratorNext = ThatVectorIteratorNext;

TEST_F(ThatZipIteratorNext, returnsZippedValues) {
  using value_type = std::pair<uint32_t, uint32_t>;

  auto vector_iter_2 = iter(std::vector<uint32_t>{5, 6, 7, 8});
  auto zip_iter = vector_iter_ | zip(vector_iter_2);
  EXPECT_THAT(zip_iter.next(), Optional(std::make_pair(1u, 5u)));
}

TEST_F(ThatZipIteratorNext, returnsNoneIfFirstEnds) {
  using value_type = std::pair<uint32_t, uint32_t>;

  auto vector_iter_2 = iter(std::vector<uint32_t>{5, 6, 7, 8, 9});
  auto zip_iter = vector_iter_ | zip(vector_iter_2);
  zip_iter.next();
  zip_iter.next();
  zip_iter.next();
  zip_iter.next();
  EXPECT_THAT(zip_iter.next(), NullOpt<value_type>());
}

TEST_F(ThatZipIteratorNext, returnsNoneIfSecondEnds) {
  using value_type = std::pair<uint32_t, uint32_t>;

  auto vector_iter_2 = iter(std::vector<uint32_t>{5, 6, 7});
  auto zip_iter = vector_iter_ | zip(vector_iter_2);
  zip_iter.next();
  zip_iter.next();
  zip_iter.next();
  EXPECT_THAT(zip_iter.next(), NullOpt<value_type>());
}

using ThatMapIteratorNext = ThatVectorIteratorNext;

struct Squared {
  uint32_t value;

  bool operator==(const Squared& other) const { return value == other.value; }
};

TEST_F(ThatMapIteratorNext, returnsTransformedValue) {
  auto map_iter =
      vector_iter_ | map([](const auto in) { return Squared{in * in}; });
  EXPECT_THAT(map_iter.next(), Optional(Squared{1}));
  EXPECT_THAT(map_iter.next(), Optional(Squared{4}));
  EXPECT_THAT(map_iter.next(), Optional(Squared{9}));
  EXPECT_THAT(map_iter.next(), Optional(Squared{16}));
  EXPECT_THAT(map_iter.next(), NullOpt<Squared>());
}
