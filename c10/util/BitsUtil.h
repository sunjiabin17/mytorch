#pragma once

#include <cassert>
#include <climits>
#include <type_traits>

namespace c10::detail {

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

} // namespace c10::detail
