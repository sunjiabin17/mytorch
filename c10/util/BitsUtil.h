#pragma once

#include <cassert>
#include <climits>
#include <limits>
#include <type_traits>

namespace c10::detail {

enum ZeroBehavior {
  ZB_Undefined,
  ZB_Max,
  ZB_Width,
};

template <typename T>
T maskTrailingOnes(unsigned N) {
  static_assert(std::is_unsigned_v<T>, "Invalid type");
  const unsigned Bits = CHAR_BIT * sizeof(T);
  assert(N <= Bits and "Invalid number of bits");
  return N == 0 ? 0 : (T(-1) >> (Bits - N));
}

template <typename T>
T maskLeadingOnes(unsigned N) {
  return ~maskTrailingOnes<T>(CHAR_BIT * sizeof(T) - N);
}

template <typename T>
T maskTrailingZeros(unsigned N) {
  return maskLeadingOnes<T>(CHAR_BIT * sizeof(T) - N);
}

template <typename T>
T maskLeadingZeros(unsigned N) {
  return maskTrailingOnes<T>(CHAR_BIT * sizeof(T) - N);
}

template <typename T, std::size_t N>
struct CountTrailingZeros {
  // static std::size_t count(T val) {
  //   static_assert(false, "Unsupported type");
  //   return 0;
  // }
};

template <typename T>
struct CountTrailingZeros<T, 4> {
  static std::size_t count(T val) {
    return val == 0 ? 32 : __builtin_ctz(val);
  }
};

template <typename T>
struct CountTrailingZeros<T, 8> {
  static std::size_t count(T val) {
    return val == 0 ? 64 : __builtin_ctzll(val);
  }
};

template <typename T>
std::size_t countTrailingZeros(T val) {
  static_assert(
      std::is_integral_v<T> and std::is_unsigned_v<T>,
      "Only unsigned integral types are supported");
  return CountTrailingZeros<T, sizeof(T)>::count(val);
}

template <typename T>
T findFirstSet(T val, ZeroBehavior zb = ZB_Max) {
  if (val == 0 and zb == ZB_Max) {
    return std::numeric_limits<T>::max();
  }
  return countTrailingZeros(val);
}

} // namespace c10::detail
