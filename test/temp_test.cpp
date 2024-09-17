#include <gtest/gtest.h>
#include <type_traits>
#include <iostream>
#include <vector>

template <class T>
class ArrayRef {
public:
  explicit ArrayRef() {
    std::cout << "ArrayRef()" << std::endl;
  }

  explicit ArrayRef(int) {
    std::cout << "ArrayRef(int)" << std::endl;
  }

  template <class U>
  std::enable_if_t<std::is_same<T, U>::value, ArrayRef>& operator=(
      U&& t) = delete;

  template <class U>
  std::enable_if_t<std::is_same<T, U>::value, ArrayRef>& operator=(
      std::initializer_list<U>) = delete;

  // ArrayRef& operator=(ArrayRef&&) = delete;
};

TEST(TmpTEST, test1) {
  ArrayRef<float> a, b;
  a = b;
  a = 1;
  a = {};
}
