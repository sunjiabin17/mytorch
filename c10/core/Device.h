#pragma once

#include <c10/core/DeviceType.h>
#include <c10/util/Exception.h>

#include <functional>
#include <ostream>
#include <string>

namespace c10 {

using DeviceIndex = int8_t;

struct Device final {
  /* implicit */ Device(DeviceType type, DeviceIndex index = -1)
      : type_(type), index_(index) {
    validate();
  }

  /* implicit */ Device(const std::string& device_string);

  bool operator==(const Device& rhs) const noexcept {
    return type_ == rhs.type_ && index_ == rhs.index_;
  }

  bool operator!=(const Device& rhs) const noexcept {
    return !(*this == rhs);
  }

  void set_index(DeviceIndex index) {
    index_ = index;
  }

  DeviceType type() const noexcept {
    return type_;
  }

  DeviceIndex index() const noexcept {
    return index_;
  }

  bool has_index() const noexcept {
    return index_ != -1;
  }

  bool is_cuda() const noexcept {
    return type_ == DeviceType::CUDA;
  }

  bool is_cpu() const noexcept {
    return type_ == DeviceType::CPU;
  }

  constexpr bool support_as_strided() const noexcept {
    return false; // [TODO]
  }

  std::string str() const;

 private:
  DeviceType type_;
  DeviceIndex index_;

  void validate() {
    TORCH_CHECK(
        index_ >= -1,
        "Device index must be -1 or non-negative, got ",
        static_cast<int>(index_));

    TORCH_CHECK(
        !is_cpu() or index_ <= 0,
        "CPU device index must be -1 or 0, got ",
        static_cast<int>(index_));
  }
};

C10_API std::ostream& operator<<(std::ostream& stream, const Device& device);

} // namespace c10

namespace std {
template <>
struct hash<c10::Device> {
  size_t operator()(const c10::Device& x) const {
    static_assert(sizeof(c10::DeviceType) == 1, "DeviceType is not 8-bit");
    static_assert(sizeof(c10::DeviceIndex) == 1, "DeviceIndex is not 8-bit");

    uint32_t bits = static_cast<uint32_t>(static_cast<uint8_t>(x.type()))
            << 16 |
        static_cast<uint32_t>(static_cast<uint8_t>(x.index()));
    return std::hash<uint32_t>()(bits);
  }
};

} // namespace std
