#pragma once

#include <c10/core/device.h>
#include <c10/core/device_type.h>
#include <c10/util/exception.h>
#include <c10/core/tensor_impl.h>
#include <c10/core/undefined_tensor_impl.h>
#include <c10/util/maybe_owned.h>
#include <c10/util/intrusive_ptr.h>

namespace at {

class TORCH_API TensorBase {

protected:
  c10::intrusive_ptr<c10::TensorImpl> impl_;

};





} // namespace at
