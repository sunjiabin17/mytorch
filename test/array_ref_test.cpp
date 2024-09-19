#include <c10/util/ArrayRef.h>
#include <gtest/gtest.h>

#include <array>
#include <initializer_list>
#include <iostream>
#include <type_traits>
#include <vector>

TEST(ArrayRefTEST, test_ctor) {
  c10::IntArrayRef ref;
  ASSERT_EQ(ref.size(), 0);
  ASSERT_EQ(ref.data(), nullptr);

  c10::IntArrayRef ref1(1);
  ASSERT_EQ(ref1.size(), 1);
  ASSERT_EQ(*ref1.data(), 1);
}

TEST(ArrayRefTEST, test_ctor2) {
  std::vector<int64_t> vec{1, 2, 3, 4};
  c10::IntArrayRef ref(vec.data(), vec.size());

  ASSERT_EQ(ref.size(), 4);
  ASSERT_EQ(ref.data(), vec.data());
  ASSERT_EQ(ref.front(), 1);
  ASSERT_EQ(ref.back(), 4);
  ASSERT_EQ(ref[0], 1);
  ASSERT_EQ(ref[1], 2);
  ASSERT_EQ(ref.at(2), 3);
  ASSERT_EQ(ref.at(3), 4);

  // UB
  // vec.clear();
  // vec.shrink_to_fit();
  // std::cout << ref.size() << std::endl;
  // std::cout << ref[0] << std::endl;
  // std::cout << ref[1] << std::endl;
  // std::cout << ref[2] << std::endl;
  // std::cout << ref.at(3) << std::endl;
}

TEST(ArrayRefTEST, test_make_array_ref) {
  std::array<int64_t, 4> arr = {1, 2, 3, 4};
  c10::IntArrayRef ref = c10::makeArrayRef(arr);

  ASSERT_EQ(ref.size(), 4);
  ASSERT_EQ(ref.data(), arr.data());
  ASSERT_EQ(ref.front(), 1);
  ASSERT_EQ(ref.back(), 4);
  ASSERT_EQ(ref[0], 1);
  ASSERT_EQ(ref[1], 2);
  ASSERT_EQ(ref.at(2), 3);
  ASSERT_EQ(ref.at(3), 4);

  std::vector<int64_t> vec = {1, 2, 3, 4};
  c10::IntArrayRef ref2 =
      c10::makeArrayRef(vec.data(), vec.data() + vec.size());

  ASSERT_EQ(ref2.size(), 4);
  ASSERT_EQ(ref2.data(), vec.data());
  ASSERT_EQ(ref2.front(), 1);
  ASSERT_EQ(ref2.back(), 4);
  ASSERT_EQ(ref2[0], 1);
  ASSERT_EQ(ref2[1], 2);
  ASSERT_EQ(ref2.at(2), 3);
  ASSERT_EQ(ref2.at(3), 4);
}

TEST(ArrayRefTEST, test_forloop) {
  // NOLINTNEXTLINE(*c-arrays)
  int64_t arr[] = {1, 2, 3, 4};
  c10::IntArrayRef ref(arr);

  ASSERT_EQ(ref.size(), 4);
  int64_t i = 1;
  for (const auto& a : arr) {
    ASSERT_EQ(a, i++);
  }
}

TEST(ArrayRefTEST, test_eq) {
  // NOLINTNEXTLINE(*c-arrays)
  int64_t arr[] = {1, 2, 3, 4};
  std::vector<int64_t> vec = {1, 2, 3, 4};
  std::array<int64_t, 4> stdarr = {1, 2, 3, 4};
  std::initializer_list<int64_t> il = {1, 2, 3, 4};

  c10::IntArrayRef ref1(arr);
  c10::IntArrayRef ref2(vec);
  c10::IntArrayRef ref3(stdarr);
  c10::IntArrayRef ref4(il);

  ASSERT_EQ(ref1 == ref2, true);
  ASSERT_EQ(ref1 == ref3, true);
  ASSERT_EQ(ref1 == ref4, true);
  ASSERT_EQ(ref2 == ref3, true);
  ASSERT_EQ(ref2 == ref4, true);
  ASSERT_EQ(ref3 == ref4, true);

  ASSERT_EQ(ref1 == vec, true);
  ASSERT_EQ(ref3 == vec, true);
  ASSERT_EQ(vec == ref3, true);

  std::vector<int64_t> vec2 = {1, 2, 3, 5};
  ASSERT_EQ(ref1 == vec2, false);
  ASSERT_EQ(vec2 == ref4, false);
  ASSERT_EQ(ref2 != vec2, true);
}
