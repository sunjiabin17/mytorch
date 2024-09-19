#pragma once

#include <c10/util/Macros.h>
#include <c10/util/StringUtil.h>

#include <exception>
#include <string>

#define __func__ __FUNCTION__

namespace c10 {
class Error : public std::exception {
 private:
  std::string msg;

 public:
  Error(
      const char* func,
      const char* file,
      uint32_t line,
      const std::string& msg);

  const char* what() const noexcept override;
};

namespace detail {
[[noreturn]] void myCheckFail(
    const char* func,
    const char* file,
    uint32_t line,
    const std::string& msg);

} // namespace detail

} // namespace c10

#define CheckMsg(cond, type, ...) \
  (::c10::str("expected " #cond " to be true, but got false. ", ##__VA_ARGS__))

#define TORCH_CHECK(cond, ...)              \
  if (UNLIKELY(!(cond))) {                  \
    ::c10::detail::myCheckFail(             \
        __func__,                           \
        __FILE__,                           \
        static_cast<uint32_t>(__LINE__),    \
        CheckMsg(cond, "", ##__VA_ARGS__)); \
  }
