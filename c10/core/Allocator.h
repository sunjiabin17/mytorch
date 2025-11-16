#pragma once

#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>
#include <c10/util/Macros.h>
#include <c10/util/UniqueVoidPtr.h>

#include <functional>
#include <memory>
#include <utility>

namespace c10 {

class C10_API DataPtr {
 private:
  c10::detail::UniqueVoidPtr data_ptr_;
  Device device_;

 public:
  DataPtr() : device_(DeviceType::CPU) {}
  DataPtr(void* data, Device device) : data_ptr_(data), device_(device) {}
  DataPtr(void* data, void* ctx, DeleterFnPtr deleter,  Device device)
      : data_ptr_(data, ctx, deleter), device_(device) {}

  void* operator->() const {
    return data_ptr_.get();
  }

  void clear() {
    data_ptr_.clear();
  }

  void* get() const {
    return data_ptr_.get();
  }

  void* mutable_get() {
    return data_ptr_.get();
  }

  void* get_context() const {
    return data_ptr_.get_context();
  }

  void* release_context() {
    return data_ptr_.release_context();
  }

  std::unique_ptr<void, DeleterFnPtr>&& move_context() {
    return data_ptr_.move_context();
  }

  operator bool() const {
    return static_cast<bool>(data_ptr_);
  }

  DeleterFnPtr get_deleter() const {
    return data_ptr_.get_deleter();
  }

  [[nodiscard]] bool compare_exchange_deleter(
      DeleterFnPtr expected_deleter,
      DeleterFnPtr new_deleter) {
    return data_ptr_.compare_exchange_deleter(expected_deleter, new_deleter);
  }

  template <typename T>
  T* cast_context(DeleterFnPtr expected_deleter) const {
    return data_ptr_.cast_context<T>(expected_deleter);
  }

  Device device() const {
    return device_;
  }

  void unsafe_set_device(Device device) {
    device_ = device;
  }
};

inline bool operator==(const DataPtr& dp, std::nullptr_t) noexcept {
  return !dp;
}

inline bool operator==(std::nullptr_t, const DataPtr& dp) noexcept {
  return !dp;
}

inline bool operator!=(const DataPtr& dp, std::nullptr_t) noexcept {
  return dp;
}

inline bool operator!=(std::nullptr_t, const DataPtr& dp) noexcept {
  return dp;
}

struct C10_API Allocator {
  virtual ~Allocator() = default;

  virtual DataPtr allocate(size_t n) = 0;

  DataPtr clone(const void* data, size_t n);

  virtual bool is_simple_data_ptr(const DataPtr& data_ptr) const;

  virtual DeleterFnPtr raw_deleter() const {
    return nullptr;
  }

  void* raw_allocate(size_t n) {
    auto dptr = allocate(n);
    TORCH_INTERNAL_ASSERT(dptr.get() == dptr.get_context());
    return dptr.release_context();
  }

  void raw_deallocate(void* ptr) {
    auto d = raw_deleter();
    TORCH_CHECK(d, "Allocator does not support raw deallocation");
    d(ptr);
  }

  virtual void copy_data(void* dest, const void* src, size_t count) const = 0;

 protected:
  void default_copy_data(void* dest, const void* src, size_t count) const;
};

C10_API void SetAllocator(DeviceType t, Allocator* alloc, uint8_t priority = 0);
C10_API Allocator* GetAllocator(const DeviceType& t);

template <DeviceType t>
struct AllocatorRegisterer {
  explicit AllocatorRegisterer(Allocator* alloc) {
    SetAllocator(t, alloc);
  }
};

#define REGISTER_ALLOCATOR(t, f)                  \
  namespace {                                     \
  static AllocatorRegisterer<t> g_allocator_d(f); \
  }

} // namespace c10
