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

} // namespace c10
