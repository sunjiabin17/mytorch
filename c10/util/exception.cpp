#include <c10/util/exception.h>
#include <c10/util/string_util.h>
#include <string>


namespace c10 {

Error::Error(
  const char* func,
  const char* file,
  uint32_t line,
  const std::string& msg)
  : msg(c10::str("[", file, ":", line, "] ", func, " ", msg)) {
}

const char* Error::what() const noexcept {
  return msg.c_str();
}

namespace detail {

void myCheckFail(
  const char* func,
  const char* file,
  uint32_t line,
  const std::string& msg) {
  throw Error(func, file, line, msg);
}

} // namespace detail

} // namespace c10
