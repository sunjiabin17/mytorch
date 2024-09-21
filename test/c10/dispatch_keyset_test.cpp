#include <c10/core/DispatchKey.h>
#include <c10/core/DispatchKeySet.h>
#include <gtest/gtest.h>
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

TEST(DispatchKeySetTEST, test_iterator) {
  auto ks = c10::DispatchKeySet(c10::DispatchKey::Dense) |
      c10::DispatchKeySet(c10::DispatchKeySet::RAW, c10::full_backend_mask);

  for (auto k : ks) {
    std::cout << k << std::endl;
  }
}

TEST(DispatchKeySetTEST, test1) {
  auto ks1 = c10::DispatchKeySet(c10::DispatchKey::Dense);
  auto ks2 = ks1.keys_to_repr({c10::DispatchKey::AutogradFunctionality});
  auto rhs1 = c10::DispatchKeySet({c10::DispatchKey::AutogradFunctionality});
  ASSERT_EQ(ks2, rhs1.raw_repr());

  auto ks3 = ks1.backend_bits_to_repr({c10::BackendComponent::CUDABit});
  auto rhs2 = c10::DispatchKeySet(c10::BackendComponent::CUDABit);
  ASSERT_EQ(ks3, rhs2.raw_repr());

  auto ks4 = c10::DispatchKeySet(
      {c10::DispatchKey::Dense, c10::DispatchKey::AutogradFunctionality});
  ks4 = ks4 | c10::DispatchKeySet(c10::BackendComponent::CUDABit);
  ASSERT_TRUE(ks4.has(c10::DispatchKey::Dense));
  ASSERT_FALSE(ks4.has_backend(c10::BackendComponent::CPUBit));
  ASSERT_TRUE(ks4.has_backend(c10::BackendComponent::CUDABit));
  ASSERT_TRUE(ks4.has_all(c10::DispatchKeySet(
      {c10::DispatchKey::Dense, c10::DispatchKey::AutogradFunctionality})));
  ASSERT_TRUE(ks4.has_any(c10::DispatchKeySet(
      {c10::DispatchKey::Dense, c10::DispatchKey::BackendSelect})));
  ASSERT_FALSE(ks4.empty());
}

TEST(DispatchKeySetTEST, test2) {
  auto ks1 = c10::DispatchKeySet(
      {c10::DispatchKey::Dense, c10::DispatchKey::AutogradCPU});
  ks1 = ks1.add(c10::DispatchKey::BackendSelect);
  ks1 = ks1 |
      c10::DispatchKeySet(
            {c10::BackendComponent::CUDABit, c10::BackendComponent::CPUBit});
  // std::cout << toBinaryString(ks1.raw_repr()) << std::endl;
  auto idx = ks1.indexOfHighestBit();
  ASSERT_EQ(
      idx,
      static_cast<decltype(idx)>(c10::DispatchKey::AutogradFunctionality) +
          c10::num_backends);

  auto k = ks1.highestFunctionalityKey();
  auto b = ks1.highestBackendKey();
  ASSERT_EQ(k, c10::DispatchKey::AutogradFunctionality);
  ASSERT_EQ(b, c10::BackendComponent::CUDABit);
}

TEST(DispatchKeySetTEST, test3) {
  auto ks = c10::DispatchKeySet(c10::DispatchKey::Dense);
  ks = ks | c10::DispatchKeySet(c10::BackendComponent::CPUBit);
  std::cout << ks.getDispatchTableIndexForDispatchKeySet() << std::endl;

  auto offsets_and_masks = c10::offsetsAndMasks();
  for (auto idx = 0; idx < c10::num_functionality_keys; ++idx) {
    std::cout << "[" << static_cast<c10::DispatchKey>(idx)
              << "] offset: " << offsets_and_masks[idx].offset
              << " mask: " << offsets_and_masks[idx].mask << std::endl;
  }
}
