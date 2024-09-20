#pragma once

#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>
#include <c10/util/Macros.h>

#include <functional>
#include <memory>
#include <utility>

namespace c10 {
using DeleterType = void (*)(void*);

namespace detail {

C10_API void deleteNothing(void*);

} // namespace detail

class C10_API DataPtr {
 private:
  std::unique_ptr<void, DeleterType> data_ptr_;
  Device device_;

 public:
  DataPtr()
      : data_ptr_(nullptr, &detail::deleteNothing), device_(DeviceType::CPU) {}
  DataPtr(void* data, Device device)
      : data_ptr_(data, &detail::deleteNothing), device_(device) {}
  DataPtr(void* data, Device device, DeleterType deleter)
      : data_ptr_(data, deleter), device_(device) {}

  void* operator->() const {
    return data_ptr_.get();
  }

  void clear() {
    data_ptr_.reset();
  }

  void* get() const {
    return data_ptr_.get();
  }

  void* mutable_get() {
    return data_ptr_.get();
  }

  void* release() {
    return data_ptr_.release();
  }

  operator bool() const {
    return static_cast<bool>(data_ptr_);
  }

  DeleterType get_deleter() const {
    return data_ptr_.get_deleter();
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
