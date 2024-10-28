#pragma once

#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>
#include <c10/core/SizesAndStrides.h>
#include <c10/core/Storage.h>
#include <c10/util/Exception.h>
#include <c10/util/IntrusivePtr.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace at {
class Tensor;
class TensorBase;
} // namespace at

namespace c10 {

struct C10_API AutogradMetaInterface {
  // TODO
};

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

 protected:
  Storage storage_;

 private:
  std::unique_ptr<c10::AutogradMetaInterface> autograd_meta_;

 protected:
  c10::impl::SizesAndStrides sizes_and_strides_;

  int64_t storage_offset_ = 0;

  int64_t numel_ = 1;

  // data_type_

  std::optional<c10::Device> device_opt_;

  bool is_contiguous_ : 1;

  bool is_channels_last_ : 1;

  bool is_channels_last_contiguous_ : 1;

  bool is_channels_last_3d_ : 1;

  bool is_channels_last_3d_contiguous_ : 1;

  bool is_non_overlapping_and_dense_ : 1;
};

} // namespace c10
