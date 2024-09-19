#include <c10/core/device_type.h>

namespace c10 {

std::string DeviceTypeName(DeviceType d, bool lower_case) {
  switch (d) {
    case DeviceType::CPU:
      return lower_case ? "cpu" : "CPU";
    case DeviceType::CUDA:
      return lower_case ? "cuda" : "CUDA";
    default:
      return lower_case ? "unknown" : "UNKNOWN";
  }
}

bool isValidDeviceType(DeviceType d) {
  switch (d) {
    case DeviceType::CPU:
    case DeviceType::CUDA:
      return true;
    default:
      return false;
  }
}

std::ostream& operator<<(std::ostream& stream, DeviceType type) {
  stream << DeviceTypeName(type);
  return stream;
}

} // namespace c10
