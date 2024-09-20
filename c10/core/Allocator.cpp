#include <c10/core/Allocator.h>
#include <cstring>

namespace c10 {
namespace detail {

void deleteNothing(void*) {}

} // namespace detail

DataPtr Allocator::clone(const void* data, size_t n) {
  DataPtr new_data = allocate(n);
  copy_data(new_data.mutable_get(), data, n);
  return new_data;
}

void Allocator::default_copy_data(void* dest, const void* src, size_t count)
    const {
  std::memcpy(dest, src, count);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables,modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
C10_API c10::Allocator*
    allocator_array[static_cast<int>(c10::DeviceType::MAX_DEVICE_TYPES)];
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables,modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
C10_API uint8_t
    allocator_priority[static_cast<int>(c10::DeviceType::MAX_DEVICE_TYPES)] = {
        0};

void SetAllocator(c10::DeviceType t, c10::Allocator* alloc, uint8_t priority) {
  if (priority >= allocator_priority[static_cast<int>(t)]) {
    allocator_array[static_cast<int>(t)] = alloc;
    allocator_priority[static_cast<int>(t)] = priority;
  }
}

c10::Allocator* GetAllocator(const c10::DeviceType& t) {
  auto* alloc = allocator_array[static_cast<int>(t)];
  TORCH_CHECK(alloc, "Allocator for ", t, " is not set");
  return alloc;
}

} // namespace c10
