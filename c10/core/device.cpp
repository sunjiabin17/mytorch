#include <c10/core/device.h>

#include <algorithm>
#include <array>
#include <string>
#include <vector>

namespace c10 {
namespace {
DeviceType parse_type(const std::string& device_name) {
  static const std::array<
      std::pair<const char*, DeviceType>,
      static_cast<size_t>(DeviceType::MAX_DEVICE_TYPES)>
      types = {{
          {"cpu", DeviceType::CPU},
          {"cuda", DeviceType::CUDA},
      }};
  auto device = std::find_if(
      types.begin(),
      types.end(),
      [&device_name](const std::pair<const char*, DeviceType>& p) {
        return p.first and p.first == device_name;
      });
  if (device != types.end()) {
    return device->second;
  }

  TORCH_CHECK(
      false,
      "Invalid device name: '",
      device_name,
      "'. Available device names are: cpu, cuda");
}

enum DeviceStringParsingState { START, INDEX_START, INDEX_REST, ERROR };
} // namespace

Device::Device(const std::string& device_string) : Device(DeviceType::CPU) {
  TORCH_CHECK(!device_string.empty(), "Device string is empty");

  std::string device_name, device_index_str;
  DeviceStringParsingState state = START;
  for (size_t i = 0;
       state != DeviceStringParsingState::ERROR && i < device_string.size();
       ++i) {
    const char ch = device_string.at(i);
    switch (state) {
      case DeviceStringParsingState::START:
        if (ch != ':') {
          if (isalpha(ch) or ch == '_') {
            device_name.push_back(ch);
          } else {
            state = DeviceStringParsingState::ERROR;
          }
        } else {
          state = DeviceStringParsingState::INDEX_START;
        }
        break;

      case DeviceStringParsingState::INDEX_START:
        if (isdigit(ch)) {
          device_index_str.push_back(ch);
          state = DeviceStringParsingState::INDEX_REST;
        } else {
          state = DeviceStringParsingState::ERROR;
        }
        break;

      case DeviceStringParsingState::INDEX_REST:
        if (device_index_str.at(0) == '0') {
          state = DeviceStringParsingState::ERROR;
          break;
        }
        if (isdigit(ch)) {
          device_index_str.push_back(ch);
        } else {
          state = DeviceStringParsingState::ERROR;
        }
        break;

      case DeviceStringParsingState::ERROR:
        break;
    }
  }

  const bool has_error = device_name.empty() or
      state == DeviceStringParsingState::ERROR or
      (state == DeviceStringParsingState::INDEX_START and
       device_index_str.empty());

  TORCH_CHECK(!has_error, "Invalid device string: '", device_string, "'");

  try {
    if (!device_index_str.empty()) {
      index_ = static_cast<DeviceIndex>(std::stoi(device_index_str));
    }
  } catch (const std::exception&) {
    TORCH_CHECK(
        false,
        "Invalid device index: '",
        device_index_str,
        "' in device string: '",
        device_string,
        "'");
  }

  type_ = parse_type(device_name);
  validate();
}

std::string Device::str() const {
  std::string device_str = DeviceTypeName(type(), /* lower_case */ true);
  if (has_index()) {
    device_str += ":" + std::to_string(index());
  }
  return device_str;
}

std::ostream& operator<<(std::ostream& stream, const Device& device) {
  stream << device.str();
  return stream;
}

} // namespace c10
