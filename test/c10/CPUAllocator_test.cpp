#include <c10/core/Allocator.h>
#include <c10/core/DeviceType.h>
#include <c10/cpu/CPUAllocator.h>
#include <gtest/gtest.h>

TEST(CPUAllocator, get) {
  using namespace c10;
  auto allocator = GetAllocator(DeviceType::CPU);
  auto block = allocator->allocate(34);
  EXPECT_TRUE(block.device() == DeviceType::CPU);
  EXPECT_TRUE(allocator->is_simple_data_ptr(block));
  EXPECT_TRUE(block.get() == block.get_context());
}
