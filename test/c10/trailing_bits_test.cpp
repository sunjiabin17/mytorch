#include <c10/util/BitsUtil.h>
#include <gtest/gtest.h>
#include <array>
#include <bitset>
#include <climits>
#include <iostream>
#include <string>

// 将整数转换为二进制字符串
template <typename T>
std::string toBinaryString(T n) {
  std::bitset<CHAR_BIT * sizeof(T)> binary(n); // 32位二进制表示
  return binary.to_string();
}

TEST(BitsTEST, test_uint32) {
  using T = uint32_t;
  constexpr auto bits_count = CHAR_BIT * sizeof(T);
  auto trailing_ones = c10::detail::maskTrailingOnes<T>(10);
  std::cout << toBinaryString(trailing_ones) << std::endl;
  ASSERT_EQ(trailing_ones, (1u << 10) - 1);

  auto trailing_zeros = c10::detail::maskTrailingZeros<T>(10);
  std::cout << toBinaryString(trailing_zeros) << std::endl;
  ASSERT_EQ(trailing_zeros, ~((1u << 10) - 1));

  auto leading_ones = c10::detail::maskLeadingOnes<T>(10);
  std::cout << toBinaryString(leading_ones) << std::endl;
  ASSERT_EQ(leading_ones, ~((1u << (bits_count - 10)) - 1));

  auto leading_zeros = c10::detail::maskLeadingZeros<T>(10);
  std::cout << toBinaryString(leading_zeros) << std::endl;
  ASSERT_EQ(leading_zeros, (1u << (bits_count - 10)) - 1);
}

TEST(BitsTEST, test_uint64) {
  using T = uint64_t;
  constexpr auto bits_count = CHAR_BIT * sizeof(T);
  auto trailing_ones = c10::detail::maskTrailingOnes<T>(10);
  std::cout << toBinaryString(trailing_ones) << std::endl;
  ASSERT_EQ(trailing_ones, (1ull << 10) - 1);

  auto trailing_zeros = c10::detail::maskTrailingZeros<T>(10);
  std::cout << toBinaryString(trailing_zeros) << std::endl;
  ASSERT_EQ(trailing_zeros, ~((1ull << 10) - 1));

  auto leading_ones = c10::detail::maskLeadingOnes<T>(10);
  std::cout << toBinaryString(leading_ones) << std::endl;
  ASSERT_EQ(leading_ones, ~((1ull << (bits_count - 10)) - 1));

  auto leading_zeros = c10::detail::maskLeadingZeros<T>(10);
  std::cout << toBinaryString(leading_zeros) << std::endl;
  ASSERT_EQ(leading_zeros, (1ull << (bits_count - 10)) - 1);
}
