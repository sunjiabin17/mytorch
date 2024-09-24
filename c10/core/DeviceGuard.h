#pragma once

#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>
#include <c10/core/impl/DeviceGuardImplInterface.h>
#include <c10/core/impl/InlineDeviceGuard.h>
#include <c10/core/impl/VirtualGuardImpl.h>
#include <c10/util/Exception.h>
#include <c10/util/Macros.h>

namespace c10 {

class DeviceGuard {
 public:
  explicit DeviceGuard() = delete;

  explicit DeviceGuard(Device device) : guard_(device) {}

  explicit DeviceGuard(
      Device device,
      const impl::DeviceGuardImplInterface* impl)
      : guard_(device, impl) {}

  DeviceGuard(const DeviceGuard&) = delete;
  DeviceGuard& operator=(const DeviceGuard&) = delete;

  DeviceGuard(DeviceGuard&&) = delete;
  DeviceGuard& operator=(DeviceGuard&&) = delete;

  void reset_device(Device device) {
    guard_.reset_device(device);
  }

  void reset_device(Device device, const impl::DeviceGuardImplInterface* impl) {
    guard_.reset_device(device, impl);
  }

  void set_index(DeviceIndex index) {
    guard_.set_index(index);
  }

  Device original_device() const {
    return guard_.original_device();
  }

  Device current_device() const {
    return guard_.current_device();
  }

 private:
  impl::InlineDeviceGuard<impl::VirtualGuardImpl> guard_;
};

} // namespace c10
