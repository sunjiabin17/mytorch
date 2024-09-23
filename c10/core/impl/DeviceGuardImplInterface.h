#pragma once

#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>
#include <c10/util/Exception.h>
#include <c10/util/Macros.h>

#include <array>
#include <atomic>

namespace c10::impl {

struct C10_API DeviceGuardImplInterface {
  DeviceGuardImplInterface() = default;
  DeviceGuardImplInterface(const DeviceGuardImplInterface&) = default;
  DeviceGuardImplInterface& operator=(
      const DeviceGuardImplInterface&) noexcept = default;
  DeviceGuardImplInterface(DeviceGuardImplInterface&&) = default;
  DeviceGuardImplInterface& operator=(DeviceGuardImplInterface&&) noexcept =
      default;
  virtual ~DeviceGuardImplInterface() = default;

  virtual DeviceType type() const = 0;

  // set the current device, and return the previous device
  virtual Device exchangeDevice(Device device) const = 0;

  virtual Device getDevice() const = 0;

  virtual void setDevice(Device device) const = 0;

  virtual void uncheckedSetDevice(Device device) const = 0;

  // [TODO] Stream
};

extern C10_API std::array<
    std::atomic<const DeviceGuardImplInterface*>,
    static_cast<size_t>(DeviceType::MAX_DEVICE_TYPES)>
    device_guard_impl_registry;

class C10_API DeviceGuardImplRegistrar {
 public:
  DeviceGuardImplRegistrar(DeviceType, const DeviceGuardImplInterface*);
};

#define C10_REGISTER_GUARD_IMPL(device, DeviceGuardImpl)               \
  static ::c10::impl::DeviceGuardImplRegistrar C10_ANONYMOUS_VARIABLE( \
      g_##DeviceType)(::c10::DeviceType::device, new DeviceGuardImpl());

inline const DeviceGuardImplInterface* getDeviceGuardImpl(DeviceType device) {
  static_assert(sizeof(DeviceType) == 1, "DeviceType is not 8-bit");
  auto p = device_guard_impl_registry[static_cast<size_t>(device)].load();
  TORCH_CHECK(p, "Device type ", device, " is not registered.");
  return p;
}

inline bool hasDeviceGuardImpl(DeviceType device) {
  return device_guard_impl_registry[static_cast<size_t>(device)].load();
}

} // namespace c10::impl
