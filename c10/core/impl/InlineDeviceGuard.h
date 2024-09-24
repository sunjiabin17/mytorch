#pragma once

#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>
#include <c10/core/impl/DeviceGuardImplInterface.h>
#include <c10/core/impl/VirtualGuardImpl.h>
#include <c10/util/Exception.h>
#include <c10/util/Macros.h>

namespace c10::impl {
template <typename T>
class InlineDeviceGuard {
 public:
  explicit InlineDeviceGuard() = delete;

  explicit InlineDeviceGuard(Device device)
      : impl_(device.type()),
        original_device_(
            device.index() == -1 ? impl_.getDevice()
                                 : impl_.exchangeDevice(device)),
        current_device_(device.index() == -1 ? original_device_ : device) {}

  template <
      typename U = T,
      typename = typename std::enable_if_t<std::is_same_v<U, VirtualGuardImpl>>>
  explicit InlineDeviceGuard(
      Device device,
      const DeviceGuardImplInterface* impl)
      : impl_(
            VirtualGuardImpl(impl ? impl : getDeviceGuardImpl(device.type()))),
        original_device_(
            device.index() == -1 ? impl_.getDevice()
                                 : impl_.exchangeDevice(device)),
        current_device_(device.index() == -1 ? original_device_ : device) {}

  InlineDeviceGuard(const InlineDeviceGuard&) = delete;
  InlineDeviceGuard& operator=(const InlineDeviceGuard&) = delete;

  InlineDeviceGuard(InlineDeviceGuard&&) = delete;
  InlineDeviceGuard& operator=(InlineDeviceGuard&&) = delete;

  ~InlineDeviceGuard() {
    impl_.uncheckedSetDevice(original_device_);
  }

  template <
      typename U = T,
      typename std::enable_if_t<std::is_same_v<U, VirtualGuardImpl>, int> = 0>
  void reset_device(
      Device device,
      const DeviceGuardImplInterface* impl = nullptr) {
    if (device.index() == -1) {
      return;
    }
    if (device.type() == original_device_.type()) {
      TORCH_CHECK(
          impl == nullptr or impl->type() == device.type(),
          "DeviceGuardImpl type mismatch");
      impl_.setDevice(device);
      current_device_ = device;
    } else {
      impl_.setDevice(original_device_);
      impl_ = !impl ? VirtualGuardImpl(device.type()) : VirtualGuardImpl(impl);
      original_device_ = impl_.exchangeDevice(device);
      current_device_ = device;
    }
  }

  void set_index(DeviceIndex index) {
    reset_device(Device(original_device_.type(), index));
  }

  Device original_device() const {
    return original_device_;
  }

  Device current_device() const {
    return current_device_;
  }

 protected:
  T impl_;

 private:
  Device original_device_;
  Device current_device_;
};

} // namespace c10::impl
