#include <c10/core/impl/DeviceGuardImplInterface.h>

namespace c10::impl {

std::array<
    std::atomic<const DeviceGuardImplInterface*>,
    static_cast<size_t>(DeviceType::MAX_DEVICE_TYPES)>
    device_guard_impl_registry;

DeviceGuardImplRegistrar::DeviceGuardImplRegistrar(
    DeviceType device,
    const DeviceGuardImplInterface* impl) {
  device_guard_impl_registry[static_cast<size_t>(device)].store(impl);
}

} // namespace c10::impl
