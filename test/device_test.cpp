#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>
#include <gtest/gtest.h>

#include <iostream>
#include <string>

TEST(Device_TEST, test1) {
  c10::Device device(c10::DeviceType::CPU, 0);
  std::cout << device.str() << std::endl;

  c10::Device device2(c10::DeviceType::CUDA, 1);
  std::cout << device2.str() << std::endl;

  c10::Device device3("cpu");
  std::cout << device3.str() << std::endl;

  c10::Device device4("cuda:1");
  std::cout << device4.str() << std::endl;

  // error parsing
  // c10::Device device5("cuda:01");
  // std::cout << device5.str() << std::endl;
}
