#pragma once

#include <c10/util/Exception.h>
#include <c10/util/Macros.h>

#include <array>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <ostream>
#include <type_traits>
#include <vector>

namespace c10 {
template <typename T>
// represent a constant reference to an array (o or more elements consecutively
// in memory)
// this class does not own the underlying data, it is expected to be used in
// situations where the data resides in some other buffer, whose lifetime
// extends past that of the ArrayRef. For this reason, it is not in general safe
// to store an ArrayRef.
class ArrayRef final {
 public:
  using iterator = const T*;
  using const_iterator = const T*;
  using size_type = size_t;
  using value_type = T;

  using reverse_iterator = std::reverse_iterator<iterator>;

 private:
  const T* data_;
  size_type length_;

 public:
  constexpr ArrayRef() : data_(nullptr), length_(0) {}

  constexpr ArrayRef(const T& oneValue) : data_(&oneValue), length_(1) {}

  constexpr ArrayRef(const T* data, size_t length)
      : data_(data), length_(length) {
    TORCH_CHECK(
        data_ != nullptr or length_ == 0,
        "created ArrayRef with nullptr and non-zero length");
  }

  constexpr ArrayRef(const T* begin, const T* end)
      : data_(begin), length_(end - begin) {
    TORCH_CHECK(
        data_ != nullptr or length_ == 0,
        "created ArrayRef with nullptr and non-zero length");
  }

  template <typename A>
  ArrayRef(const std::vector<T, A>& vec)
      : data_(vec.data()), length_(vec.size()) {
    static_assert(
        !std::is_same<T, bool>::value, "ArrayRef<bool> is not supported");
  }

  template <size_t N>
  constexpr ArrayRef(const std::array<T, N>& arr)
      : data_(arr.data()), length_(N) {}

  template <size_t N>
  // NOLINTNEXTLINE(*c-arrays*)
  constexpr ArrayRef(const T (&arr)[N]) : data_(arr), length_(N) {}

  constexpr ArrayRef(const std::initializer_list<T>& vec)
      : data_(
            std::begin(vec) == std::end(vec) ? static_cast<T*>(nullptr)
                                             : std::begin(vec)),
        length_(vec.size()) {}

  constexpr iterator begin() const {
    return data_;
  }

  constexpr iterator end() const {
    return data_ + length_;
  }

  constexpr const_iterator cbegin() const {
    return data_;
  }

  constexpr const_iterator cend() const {
    return data_ + length_;
  }

  constexpr reverse_iterator rbegin() const {
    return reverse_iterator(end());
  }

  constexpr reverse_iterator rend() const {
    return reverse_iterator(begin());
  }

  constexpr bool empty() const {
    return length_ == 0;
  }

  constexpr const T* data() const {
    return data_;
  }

  constexpr size_t size() const {
    return length_;
  }

  constexpr const T& front() const {
    TORCH_CHECK(
        !empty(), "ArrayRef: attempt to access front() of empty ArrayRef");
    return data_[0];
  }

  constexpr const T& back() const {
    TORCH_CHECK(
        !empty(), "ArrayRef: attempt to access back() of empty ArrayRef");
    return data_[length_ - 1];
  }

  constexpr bool equals(ArrayRef rhs) const {
    return length_ == rhs.length_ and std::equal(begin(), end(), rhs.begin());
  }

  // slice(n, m) - take M elements of the array starting at element N
  constexpr ArrayRef<T> slice(size_t N, size_t M) const {
    TORCH_CHECK(N + M <= size(), "ArrayRef: slice(n, m) with n + m > size()");
    return ArrayRef<T>(data_ + N, M);
  }

  // size(n) - chop off the first N elements of the array
  constexpr ArrayRef<T> slice(size_t N) const {
    TORCH_CHECK(N <= size(), "ArrayRef: slice(n) with n > size()");
    return slice(N, size() - N);
  }

  constexpr const T& operator[](size_t index) const {
    return data_[index];
  }

  constexpr const T& at(size_t index) const {
    TORCH_CHECK(index < size(), "ArrayRef: index out of range");
    return data_[index];
  }

  template <typename U>
  std::enable_if_t<std::is_same_v<T, U>, ArrayRef<T>>& operator=(
      // NOLINTNEXTLINE(performance-noexcept-move-constructor)
      U&&) = delete;

  template <typename U>
  std::enable_if_t<std::is_same_v<T, U>, ArrayRef<T>>& operator=(
      std::initializer_list<U>) = delete;

  std::vector<T> vec() const {
    return std::vector<T>(begin(), end());
  }
};

template <typename T>
std::ostream& operator<<(std::ostream& out, ArrayRef<T> ref) {
  int i = 0;
  out << '[';
  for (const auto& e : ref) {
    if (i++ > 0)
      out << ", ";
    out << e;
  }
  out << ']';
  return out;
}

template <typename T>
ArrayRef<T> makeArrayRef(const T& OneElt) {
  return OneElt;
}

template <typename T>
ArrayRef<T> makeArrayRef(const T* data, size_t length) {
  return ArrayRef<T>(data, length);
}

template <typename T>
ArrayRef<T> makeArrayRef(const T* begin, const T* end) {
  return ArrayRef<T>(begin, end);
}

template <typename T>
ArrayRef<T> makeArrayRef(const std::vector<T>& vec) {
  return ArrayRef<T>(vec);
}

template <typename T, size_t N>
ArrayRef<T> makeArrayRef(const std::array<T, N>& arr) {
  return ArrayRef<T>(arr);
}

template <typename T>
ArrayRef<T> makeArrayRef(const ArrayRef<T>& ref) {
  return ref;
}

template <typename T>
ArrayRef<T> makeArrayRef(ArrayRef<T>& ref) {
  return ref;
}

template <typename T, size_t N>
// NOLINTNEXTLINE(*c-arrays*)
ArrayRef<T> makeArrayRef(const T (&arr)[N]) {
  return ArrayRef<T>(arr);
}

template <typename T>
bool operator==(c10::ArrayRef<T> lhs, c10::ArrayRef<T> rhs) {
  return lhs.equals(rhs);
}

template <typename T>
bool operator!=(c10::ArrayRef<T> lhs, c10::ArrayRef<T> rhs) {
  return !lhs.equals(rhs);
}

template <typename T>
bool operator==(const std::vector<T>& lhs, c10::ArrayRef<T> rhs) {
  return c10::ArrayRef<T>(lhs).equals(rhs);
}

template <typename T>
bool operator!=(const std::vector<T>& lhs, c10::ArrayRef<T> rhs) {
  return !c10::ArrayRef<T>(lhs).equals(rhs);
}

template <typename T>
bool operator==(c10::ArrayRef<T> lhs, const std::vector<T>& rhs) {
  return lhs.equals(c10::ArrayRef<T>(rhs));
}

template <typename T>
bool operator!=(c10::ArrayRef<T> lhs, const std::vector<T>& rhs) {
  return !lhs.equals(c10::ArrayRef<T>(rhs));
}

using IntArrayRef = ArrayRef<int64_t>;
} // namespace c10
