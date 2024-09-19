#pragma once

#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>
#include <c10/util/Exception.h>
#include <c10/util/IntrusivePtr.h>
#include <c10/util/MaybeOwned.h>
#include <c10/util/TensorImpl.h>
#include <c10/util/UndefinedTensorImpl.h>

namespace at {

class TORCH_API TensorBase {
 protected:
  c10::intrusive_ptr<c10::TensorImpl> impl_;
};

} // namespace at
