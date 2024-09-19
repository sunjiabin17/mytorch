#pragma once

#include <c10/util/macros.h>

#include <functional>
#include <iostream>
#include <string>

namespace c10 {

enum class DeviceType : uint8_t {
  CPU = 0,
  CUDA = 1,
  MAX_DEVICE_TYPES = 2,
};

constexpr DeviceType kCPU = DeviceType::CPU;
constexpr DeviceType kCUDA = DeviceType::CUDA;

C10_API std::string DeviceTypeName(DeviceType d, bool lower_case = false);

C10_API bool isValidDeviceType(DeviceType d);

C10_API std::ostream& operator<<(std::ostream& stream, DeviceType type);

} // namespace c10

namespace std {
template <>
struct hash<c10::DeviceType> {
  size_t operator()(const c10::DeviceType& x) const {
    return std::hash<uint8_t>()(static_cast<uint8_t>(x));
  }
};
} // namespace std
