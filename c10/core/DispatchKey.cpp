#include <c10/core/DispatchKey.h>

#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>
namespace c10 {

BackendComponent toBackendComponent(DeviceType device_type) {
  switch (device_type) {
    case DeviceType::CPU:
      return BackendComponent::CPUBit;
    case DeviceType::CUDA:
      return BackendComponent::CUDABit;
    default:
      return BackendComponent::InvalidBit;
  }
}

std::string toString(DispatchKey dispatch_key) {
  switch (dispatch_key) {
    case DispatchKey::Undefined:
      return "Undefined";
    case DispatchKey::Dense:
      return "Dense";
    case DispatchKey::BackendSelect:
      return "BackendSelect";
    case DispatchKey::ADInplaceOrView:
      return "ADInplaceOrView";
    case DispatchKey::AutogradFunctionality:
      return "AutogradFunctionality";
    case DispatchKey::AutocastCPU:
      return "AutocastCPU";
    case DispatchKey::AutocastCUDA:
      return "AutocastCUDA";
    // alias
    case DispatchKey::Autograd:
      return "Autograd";
    case DispatchKey::CompositeImplicitAutograd:
      return "CompositeImplicitAutograd";
    case DispatchKey::CompositeExplicitAutograd:
      return "CompositeExplicitAutograd";
    // runtime keys
    default:
      auto functionality_key = toFunctionalityKey(dispatch_key);
      auto backend = toBackendComponent(dispatch_key);
      return toString(functionality_key) + std::string(" ") + toString(backend);
  }
}

std::string toString(BackendComponent backend) {
  switch (backend) {
    case BackendComponent::CPUBit:
      return "CPUBit";
    case BackendComponent::CUDABit:
      return "CUDABit";
    case BackendComponent::MetaBit:
      return "MetaBit";
    default:
      return "InvalidBit";
  }
}

std::ostream& operator<<(std::ostream& stream, DispatchKey dispatch_key) {
  return stream << toString(dispatch_key);
}

std::ostream& operator<<(std::ostream& stream, BackendComponent backend) {
  return stream << toString(backend);
}

} // namespace c10
