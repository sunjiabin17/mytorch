#pragma once

#include <c10/core/device.h>
#include <c10/core/device_type.h>
#include <c10/util/exception.h>
#include <c10/util/intrusive_ptr.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace at {
class Tensor;
class TensorBase;
} // namespace at

namespace c10 {

namespace impl {
struct C10_API AutogradMetaFactory {
  // TODO
};

// SetAutogradMetaFactory
// GetAutogradMetaFactory

struct C10_API AutogradMetaFactoryRegisterer {
  // TODO
};
} // namespace impl

// BackendMeta
// ExtraMeta
// VariableVersion

struct C10_API TensorImpl : public c10::intrusive_ptr_target {
  TensorImpl() = delete;
  ~TensorImpl() override;

  enum ImplType { VIEW };
  
};

} // namespace c10
