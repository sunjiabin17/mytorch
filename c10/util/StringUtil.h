#pragma once

#include <ostream>
#include <sstream>
#include <string>

namespace c10 {
namespace detail {
template <typename T>
struct canonicalize_str {
  using type = const T&;
};

template <size_t N>
// NOLINTNEXTLINE(*c-arrays*)
struct canonicalize_str<char[N]> {
  using type = const char*;
};

inline std::ostream& my_str(std::ostream& ss) {
  return ss;
}

template <typename T>
inline std::ostream& my_str(std::ostream& ss, const T& t) {
  ss << t;
  return ss;
}

template <typename T, typename... Args>
inline std::ostream& my_str(std::ostream& ss, const T& t, const Args&... args) {
  return my_str(my_str(ss, t), args...);
}

template <typename... Args>
struct str_wrapper final {
  static std::string call(const Args&... args) {
    std::ostringstream ss;
    my_str(ss, args...);
    return ss.str();
  }
};

template <>
struct str_wrapper<std::string> final {
  static std::string call(const std::string& str) {
    return str;
  }
};

template <>
struct str_wrapper<const char*> final {
  static std::string call(const char* str) {
    return str;
  }
};

} // namespace detail

template <typename... Args>
inline decltype(auto) str(const Args&... args) {
  return detail::str_wrapper<
      typename detail::canonicalize_str<Args>::type...>::call(args...);
}

} // namespace c10
