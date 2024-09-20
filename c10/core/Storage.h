#pragma once

#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>
#include <c10/core/StorageImpl.h>
#include <c10/util/Exception.h>
#include <c10/util/IntrusivePtr.h>
#include <c10/util/Macros.h>
#include <c10/util/MaybeOwned.h>

#include <utility>

namespace c10 {

struct C10_API Storage {
 public:
  Storage() = default;
  Storage(c10::intrusive_ptr<c10::StorageImpl> ptr)
      : storage_impl_(std::move(ptr)) {}
  
 protected:
  c10::intrusive_ptr<c10::StorageImpl> storage_impl_;

};

} // namespace c10
