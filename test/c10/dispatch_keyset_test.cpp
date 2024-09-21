#include <c10/core/DispatchKey.h>
#include <c10/core/DispatchKeySet.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

TEST(DispatchKeySetTEST, test_iterator) {
  auto ks = c10::DispatchKeySet(c10::DispatchKey::Dense) |
      c10::DispatchKeySet(c10::DispatchKeySet::RAW, c10::full_backend_mask);

  for (auto k : ks) {
    std::cout << k << std::endl;
  }
}
