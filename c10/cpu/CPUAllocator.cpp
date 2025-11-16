#include <c10/core/Allocator.h>
#include <c10/core/DeviceType.h>
#include <c10/cpu/CPUAllocator.h>
#include <c10/cpu/impl/alloc.h>
#include <c10/util/UniqueVoidPtr.h>

namespace c10 {

static void cpu_deleter(void* ptr) {
  c10::free_cpu(ptr);
}

struct C10_API CPUAllocator final : Allocator {
  CPUAllocator() = default;
  DataPtr allocate(size_t nbytes) override {
    void* data = nullptr;
    // [TODO] try catch here
    data = c10::alloc_cpu(nbytes);
    return {data, data, &cpu_deleter, Device{DeviceType::CPU}};
  }

  DeleterFnPtr raw_deleter() const override {
    return &cpu_deleter;
  }

  void copy_data(void* dest, const void* src, size_t count) const override {
    default_copy_data(dest, src, count);
  }
};

static CPUAllocator g_cpu_alloc;

Allocator* GetCPUAllocator() {
  return &g_cpu_alloc;
}

void SetCPUAllocator(c10::Allocator* alloc, uint8_t priority) {
  SetAllocator(DeviceType::CPU, alloc, priority);
}

namespace {
static AllocatorRegisterer<DeviceType ::CPU> g_allocator_d(&g_cpu_alloc);
}

} // namespace c10
