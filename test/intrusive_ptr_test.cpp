#include <c10/util/intrusive_ptr.h>

#include <gtest/gtest.h>
#include <iostream>

class TestClass : public c10::intrusive_ptr_target {
public:
  TestClass() : c10::intrusive_ptr_target() {
    std::cout << "line: " << __LINE__<< " TestClass constructor" << std::endl;
  }

  ~TestClass() override {
    std::cout << "line: " << __LINE__<< " TestClass destructor" << std::endl;
  }

  void release_resources() override {
    std::cout << "TestClass release_resources" << std::endl;
  }
};

class TestClassNull : public TestClass {
public:
  TestClassNull() {
    std::cout << "line: " << __LINE__<< " TestClassNull constructor" << std::endl;
  }

  ~TestClassNull() override {
    std::cout << "line: " << __LINE__<< " TestClassNull destructor" << std::endl;
  }
  static TestClassNull* null() {
    static TestClassNull* instance = new TestClassNull();
    return instance;
  };
};

template <typename T, typename NullType>
void print_count(c10::intrusive_ptr<T, NullType>& ptr) {
  if (ptr) {
    std::cout << "intrusive_refcount: " << ptr.ref_use_count() << std::endl;
    std::cout << "intrusive_weakcount: " << ptr.weak_use_count() << std::endl;
  } else {
    std::cout << "ptr is null" << std::endl;
  }
}

template <typename T, typename NullType>
void print_count(c10::weak_intrusive_ptr<T, NullType>& ptr) {
  std::cout << "intrusive_weak_refcount: " << ptr.ref_use_count() << std::endl;
  std::cout << "intrusive_weak_weakcount: " << ptr.weak_use_count() << std::endl;
}

TEST(IntrusivePtr_TEST, test1) {
  c10::intrusive_ptr<TestClass, TestClassNull> ptr = c10::make_intrusive<TestClass, TestClassNull>();
  print_count(ptr);
  c10::weak_intrusive_ptr<TestClass, TestClassNull> wptr(ptr);
  print_count(wptr);

}

TEST(IntrusivePtr_TEST, test2) {
  auto ptr = c10::make_intrusive<TestClass, TestClassNull>();
  print_count(ptr);

  c10::weak_intrusive_ptr<TestClass, TestClassNull> wptr(ptr);
  print_count(wptr);
  // NOLINTNEXTLINE
  c10::weak_intrusive_ptr<TestClass, TestClassNull> wptr2 = wptr;
  print_count(wptr2);
}

TEST(IntrusivePtr_TEST, test3) {
  auto ptr = c10::make_intrusive<TestClass, TestClassNull>();
  print_count(ptr);

  c10::weak_intrusive_ptr<TestClass, TestClassNull> wptr(ptr);
  print_count(wptr);
  // NOLINTNEXTLINE
  c10::weak_intrusive_ptr<TestClass, TestClassNull> wptr2 = wptr;
  print_count(wptr2);

  auto ptr2 = wptr.lock();
  ASSERT_EQ(ptr2.defined(), true);

  ptr.reset();
  print_count(ptr);

  auto ptr3 = wptr.lock();
  ASSERT_EQ(ptr3.defined(), true);

  ptr2.reset();
  ptr3.reset();
  auto ptr4 = wptr.lock();
  ASSERT_EQ(ptr4.defined(), false);
}
