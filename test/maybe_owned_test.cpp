#include <c10/util/maybe_owned.h>
#include <gtest/gtest.h>

#include <memory>
#include <type_traits>
#include <utility>
#include <iostream>

class TestClass1 {
public:
  float x;
  TestClass1() : x(1.23) {
    std::cout << "TestClass1 constructor" << std::endl;
  }

  TestClass1(float x) : x(x) {
    std::cout << "TestClass1 constructor" << std::endl;
  }

  ~TestClass1() {
    std::cout << "TestClass1 destructor" << std::endl;
  }
};

// NOLINTBEGIN

TEST(MaybeOwnedTest, test1) {
  auto p = std::make_shared<TestClass1>();
  std::cout << p.use_count() << std::endl;
  auto maybe_owned = c10::MaybeOwned<decltype(p)>::borrowed(p);
  std::cout << maybe_owned->use_count() << std::endl;

  std::cout << (*maybe_owned)->x << std::endl;

  auto maybe_owned2 = maybe_owned;
  std::cout << maybe_owned2->use_count() << std::endl;

  maybe_owned.~MaybeOwned();
  std::cout << maybe_owned2->use_count() << std::endl;

  auto maybe_owned3 = c10::MaybeOwned<decltype(p)>::owned(std::in_place, p);
  std::cout << maybe_owned3->use_count() << std::endl;
  std::cout << (*maybe_owned3)->x << std::endl;
  ASSERT_EQ((*maybe_owned3)->x, static_cast<float>(1.23));

}

TEST(MaybeOwnedTest, test2) {
  auto p = std::make_shared<TestClass1>();
  std::cout << p.use_count() << std::endl;
  auto maybe_owned = c10::MaybeOwned<decltype(p)>::owned(std::move(p));
  std::cout << maybe_owned->use_count() << std::endl;

  std::cout << (*maybe_owned)->x << std::endl;

  auto maybe_owned_move = std::move(maybe_owned);
  std::cout << maybe_owned_move->use_count() << std::endl;

  auto maybe_owned_copy = maybe_owned_move;
  std::cout << maybe_owned_copy->use_count() << std::endl;

  maybe_owned_move.~MaybeOwned();
  std::cout << maybe_owned_copy->use_count() << std::endl;

}

TEST(MaybeOwnedTest, test3) {
  auto p = std::make_shared<TestClass1>();
  auto owned = c10::MaybeOwned<decltype(p)>::owned(std::move(p));

  auto p2 = std::make_shared<TestClass1>();
  auto borrowed = c10::MaybeOwned<decltype(p2)>::borrowed(p2);

  owned = borrowed;
  ASSERT_EQ(owned.unsafeIsBorrowed(), true);
  ASSERT_EQ(borrowed.unsafeIsBorrowed(), true);
}

TEST(MaybeOwnedTest, test5) {
  auto p = std::make_shared<TestClass1>();
  auto owned = c10::MaybeOwned<decltype(p)>::owned(std::move(p));

  auto p2 = std::make_shared<TestClass1>();
  auto borrowed = c10::MaybeOwned<decltype(p2)>::borrowed(p2);

  borrowed = owned;
  ASSERT_EQ(owned.unsafeIsBorrowed(), false);
  ASSERT_EQ(borrowed.unsafeIsBorrowed(), false);
}


TEST(MaybeOwnedTest, test6) {
  auto p = std::make_shared<TestClass1>();
  auto owned = c10::MaybeOwned<decltype(p)>::owned(std::move(p));

  auto p2 = std::make_shared<TestClass1>();
  auto borrowed = c10::MaybeOwned<decltype(p2)>::borrowed(p2);

  owned = std::move(borrowed);
  ASSERT_EQ(owned.unsafeIsBorrowed(), true);
}

TEST(MaybeOwnedTest, test7) {
  auto p = std::make_shared<TestClass1>();
  auto owned = c10::MaybeOwned<decltype(p)>::owned(std::move(p));

  auto p2 = std::make_shared<TestClass1>();
  auto borrowed = c10::MaybeOwned<decltype(p2)>::borrowed(p2);

  borrowed = std::move(owned);
  ASSERT_EQ(borrowed.unsafeIsBorrowed(), false);
}


// NOLINTEND
