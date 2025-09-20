#pragma once

#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>
#include <c10/core/StorageImpl.h>
#include <c10/util/Exception.h>
#include <c10/util/IntrusivePtr.h>
#include <c10/util/Macros.h>
#include <c10/util/MaybeOwned.h>

#include <utility>
#include "c10/core/Allocator.h"

namespace c10 {
struct Storage;

bool isSharedStorageAlias(const Storage& storage0, const Storage& storage1);

struct C10_API Storage {
 public:
  struct use_byte_size_t {};
  struct unsafe_borrow_t {
    explicit unsafe_borrow_t() = default;
  };

  Storage() = default;
  Storage(c10::intrusive_ptr<c10::StorageImpl> ptr)
      : storage_impl_(std::move(ptr)) {}

  Storage(
      use_byte_size_t,
      const size_t size_bytes,
      Allocator* allocator = nullptr,
      bool resizable = false)
      : storage_impl_(
            c10::make_intrusive<StorageImpl>(
                StorageImpl::use_byte_size_t{},
                size_bytes,
                allocator,
                resizable)) {}
  Storage(
      use_byte_size_t,
      const size_t size_bytes,
      DataPtr data_ptr,
      Allocator* allocator = nullptr,
      bool resizable = false)
      : storage_impl_(
            c10::make_intrusive<StorageImpl>(
                StorageImpl::use_byte_size_t{},
                size_bytes,
                std::move(data_ptr),
                allocator,
                resizable)) {}

 protected:
  explicit Storage(unsafe_borrow_t, const Storage& rhs)
      : storage_impl_(
            c10::intrusive_ptr<c10::StorageImpl>::reclaim(
                rhs.storage_impl_.get())) {}

 public:
  void set_nbytes(size_t size_bytes) {
    storage_impl_->set_nbytes(size_bytes);
  }

  bool resizable() const {
    return storage_impl_->resizable();
  }

  size_t nbytes() const {
    return storage_impl_->nbytes();
  }

  const void* data() const {
    return storage_impl_->data_ptr().get();
  }

  void* mutable_data() {
    return storage_impl_->mutable_data_ptr().get();
  }

  DataPtr& mutable_data_ptr() const {
    return storage_impl_->mutable_data_ptr();
  }

  const DataPtr& data_ptr() const {
    return storage_impl_->data_ptr();
  }

  DataPtr set_data_ptr(DataPtr&& data_ptr) const {
    return storage_impl_->set_data_ptr(std::move(data_ptr));
  }

  void set_data_ptr_noswap(DataPtr&& data_ptr) const {
    storage_impl_->set_data_ptr_noswap(std::move(data_ptr));
  }

  DeviceType device_type() const {
    return storage_impl_->device_type();
  }

  Allocator* allocator() const {
    return storage_impl_->allocator();
  }

  Device device() const {
    return storage_impl_->device();
  }

  StorageImpl* unsafeGetStorageImpl() const noexcept {
    return storage_impl_.get();
  }

  StorageImpl* unsafeReleaseStorageImpl() {
    return storage_impl_.release();
  }

  c10::weak_intrusive_ptr<StorageImpl> getWeakStorageImpl() const {
    return c10::weak_intrusive_ptr<StorageImpl>(storage_impl_);
  }

  operator bool() const {
    return static_cast<bool>(storage_impl_);
  }

  size_t use_count() const {
    return storage_impl_.ref_use_count();
  }

  inline bool unique() const {
    return storage_impl_.unique();
  }

  bool is_alias_of(const Storage& other) const {
    return storage_impl_ == other.storage_impl_ ||
        isSharedStorageAlias(*this, other);
  }

 protected:
  c10::intrusive_ptr<c10::StorageImpl> storage_impl_;
};

} // namespace c10
