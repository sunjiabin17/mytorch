#pragma once

#include <c10/core/Allocator.h>
#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>
#include <c10/core/StorageImpl.h>
#include <c10/util/Exception.h>
#include <c10/util/IntrusivePtr.h>
#include <c10/util/Macros.h>
#include <c10/util/MaybeOwned.h>

#include <utility>

namespace c10 {
struct C10_API StorageImpl : public c10::intrusive_ptr_target {
 public:
  struct use_byte_size_t {};

  StorageImpl(
      use_byte_size_t,
      const size_t size_bytes,
      DataPtr data_ptr,
      Allocator* allocator,
      bool resizable)
      : data_ptr_(std::move(data_ptr)),
        size_bytes_(size_bytes),
        resizable_(resizable),
        allocator_(allocator) {
    if (resizable_) {
      TORCH_INTERNAL_ASSERT(
          allocator_, "For resizable storage, allocator must be provided");
    }
    refresh_has_data_ptr_check();
  }

  StorageImpl(
      use_byte_size_t,
      const size_t size_bytes,
      Allocator* allocator = nullptr,
      bool resizable = false)
      : StorageImpl(
            use_byte_size_t{},
            size_bytes,
            allocator->allocate(size_bytes),
            allocator,
            resizable) {}
  StorageImpl(const StorageImpl&) = delete;
  StorageImpl& operator=(const StorageImpl&) = delete;
  StorageImpl() = delete;
  StorageImpl(StorageImpl&&) = delete;
  StorageImpl& operator=(StorageImpl&&) = delete;
  ~StorageImpl() override = default;

  void reset() {
    data_ptr_.clear();
    size_bytes_ = 0;
  }

  void release_resources() override {
    data_ptr_.clear();
  }

  size_t nbytes() const {
    return size_bytes_;
  }

  void set_nbytes(size_t size_bytes) {
    size_bytes_ = size_bytes;
  }

  bool resizable() const {
    return resizable_;
  }

  const DataPtr& data_ptr() const {
    if (UNLIKELY(has_mutable_data_ptr_check_)) {
      if (throw_on_immutable_data_ptr_) {
        throw_data_ptr_access_error();
      }
    }
    return data_ptr_;
  }

  DataPtr& mutable_data_ptr() {
    if (UNLIKELY(has_mutable_data_ptr_check_)) {
      if (throw_on_mutable_data_ptr_) {
        throw_data_ptr_access_error();
      }
    }
    return data_ptr_;
  }

  DataPtr& _mutable_data_ptr_unsafe() {
    return data_ptr_;
  }

  DataPtr set_data_ptr(DataPtr&& data_ptr) {
  return set_data_ptr_no_materialize_cow(std::move(data_ptr));
  }

  void set_data_ptr_noswap(DataPtr&& data_ptr) {
    data_ptr_ = std::move(data_ptr);
    refresh_has_data_ptr_check();
  }

  // [TODO] cow
  DataPtr set_data_ptr_no_materialize_cow(DataPtr&& data_ptr) {
    DataPtr old_data_ptr(std::move(data_ptr_));
    data_ptr_ = std::move(data_ptr);
    refresh_has_data_ptr_check();
    return old_data_ptr;
  }


  void set_throw_on_mutable_data_ptr() {
    throw_on_mutable_data_ptr_ = true;
    refresh_has_data_ptr_check();
  }

  void set_throw_on_immutable_data_ptr() {
    throw_on_immutable_data_ptr_ = true;
    refresh_has_data_ptr_check();
  }

  [[noreturn]] void throw_data_ptr_access_error() const;

  Allocator* allocator() {
    return allocator_;
  }

  const Allocator* allocator() const {
    return allocator_;
  }

  DeviceType device_type() const {
    return data_ptr_.device().type();
  }

  void set_allocator(Allocator* allocator) {
    allocator_ = allocator;
  }

  Device device() const {
    return data_ptr_.device();
  }

  void set_resizable(bool resizable) {
    if (resizable) {
      TORCH_INTERNAL_ASSERT(
          allocator_, "For resizable storage, allocator must be provided");
    }
    resizable_ = resizable;
  }

 private:
  void refresh_has_data_ptr_check() {
    has_mutable_data_ptr_check_ =
        throw_on_mutable_data_ptr_ || throw_on_immutable_data_ptr_;
  }

  DataPtr data_ptr_;
  size_t size_bytes_;
  bool resizable_;

  bool has_mutable_data_ptr_check_ = false;
  bool throw_on_mutable_data_ptr_ = false;
  bool throw_on_immutable_data_ptr_ = false;
  Allocator* allocator_;
};

} // namespace c10
