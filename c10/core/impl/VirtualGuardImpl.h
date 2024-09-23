#pragma once

#include <c10/core/impl/DeviceGuardImplInterface.h>

namespace c10::impl {

class VirtualGuardImpl final : public DeviceGuardImplInterface {
 public:
  VirtualGuardImpl(DeviceType device_type)
      : impl_(getDeviceGuardImpl(device_type)) {}

  VirtualGuardImpl(const DeviceGuardImplInterface* impl) : impl_(impl) {}

  VirtualGuardImpl(const VirtualGuardImpl&) = default;
  VirtualGuardImpl& operator=(const VirtualGuardImpl&) = default;
  VirtualGuardImpl(VirtualGuardImpl&&) = default;
  VirtualGuardImpl& operator=(VirtualGuardImpl&&) = default;

  DeviceType type() const override {
    return impl_->type();
  }

  Device exchangeDevice(Device device) const override {
    return impl_->exchangeDevice(device);
  }

  Device getDevice() const override {
    return impl_->getDevice();
  }

  void setDevice(Device device) const override {
    impl_->setDevice(device);
  }

  void uncheckedSetDevice(Device device) const override {
    impl_->uncheckedSetDevice(device);
  }

  // [TODO] Stream

 private:
  const DeviceGuardImplInterface* impl_;
};

} // namespace c10::impl
