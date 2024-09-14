#include <c10/util/string_util.h>
#include <gtest/gtest.h>
#include <string>
#include <iostream>

TEST(Str_Append_TEST, test1) {
  std::string str{"hello"};
  const char* cstr = "world";

  auto res = c10::str("[", __FILE__, ":", __LINE__, "]\t", __FUNCTION__, " str: ", str, " cstr: ", cstr, " num: ", 20, " float: ", 12.321);
  std::cout << res << std::endl;
}
