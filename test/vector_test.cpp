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
#include "step_by.hpp"
#include "vector_iterator.hpp"

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
