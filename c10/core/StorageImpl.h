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
struct C10_API StorageImpl : public c10::intrusive_ptr_target {
 public:
};

} // namespace c10
