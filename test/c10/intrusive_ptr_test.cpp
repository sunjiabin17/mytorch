#include <c10/util/IntrusivePtr.h>
#include <c10/util/MaybeOwned.h>
#include <gtest/gtest.h>
#include <iostream>

class TestClass : public c10::intrusive_ptr_target {
 public:
  int a;
  TestClass() : a(0), c10::intrusive_ptr_target() {
    std::cout << "TestClass constructor" << std::endl;
  }
  TestClass(int a) : a(a), c10::intrusive_ptr_target() {
    std::cout << "TestClass constructor" << std::endl;
  }

  ~TestClass() override {
    std::cout << "line: " << __LINE__ << " TestClass destructor" << std::endl;
  }

  void release_resources() override {
    std::cout << "TestClass release_resources" << std::endl;
  }
};

class TestClassNull : public TestClass {
 public:
  TestClassNull() {
    std::cout << "line: " << __LINE__ << " TestClassNull constructor"
              << std::endl;
  }

  ~TestClassNull() override {
    std::cout << "line: " << __LINE__ << " TestClassNull destructor"
              << std::endl;
  }
  static TestClassNull* null() {
    static TestClassNull* instance = new TestClassNull();
    return instance;
  };
};

template <typename T, typename NullType>
void print_count(const c10::intrusive_ptr<T, NullType>& ptr) {
  if (ptr) {
    std::cout << "intrusive_refcount: " << ptr.ref_use_count() << std::endl;
    std::cout << "intrusive_weakcount: " << ptr.weak_use_count() << std::endl;
  } else {
    std::cout << "ptr is null" << std::endl;
  }
}

template <typename T, typename NullType>
void print_count(const c10::weak_intrusive_ptr<T, NullType>& ptr) {
  std::cout << "intrusive_weak_refcount: " << ptr.ref_use_count() << std::endl;
  std::cout << "intrusive_weak_weakcount: " << ptr.weak_use_count()
            << std::endl;
}

TEST(IntrusivePtrTEST, test1) {
  c10::intrusive_ptr<TestClass, TestClassNull> ptr =
      c10::make_intrusive<TestClass, TestClassNull>();
  print_count(ptr);
  c10::weak_intrusive_ptr<TestClass, TestClassNull> wptr(ptr);
  print_count(wptr);
}

TEST(IntrusivePtrTEST, test2) {
  auto ptr = c10::make_intrusive<TestClass, TestClassNull>();
  print_count(ptr);

  c10::weak_intrusive_ptr<TestClass, TestClassNull> wptr(ptr);
  print_count(wptr);
  // NOLINTNEXTLINE
  c10::weak_intrusive_ptr<TestClass, TestClassNull> wptr2 = wptr;
  print_count(wptr2);
}

TEST(IntrusivePtrTEST, test3) {
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

TEST(IntrusivePtrTEST, test_operator) {
  auto ptr1 = c10::make_intrusive<TestClass, TestClassNull>(12);
  auto ptr2 = c10::make_intrusive<TestClass, TestClassNull>(12);

  ASSERT_EQ(ptr1 != ptr2, true);
  // NOLINTNEXTLINE
  auto ptr11 = ptr1;
  ASSERT_EQ(ptr1 == ptr11, true);

  c10::weak_intrusive_ptr<TestClass, TestClassNull> wptr1(ptr1);
  c10::weak_intrusive_ptr<TestClass, TestClassNull> wptr11(ptr1);
  ASSERT_EQ(wptr1 == wptr1, true);
  ASSERT_EQ(wptr1 != wptr1, false);

  c10::weak_intrusive_ptr<TestClass, TestClassNull> wptr2(ptr2);
  ASSERT_EQ(wptr1 == wptr2, false);
  ASSERT_EQ(wptr1 != wptr2, true);
}

TEST(IntrusivePtrTEST, test_reclaim) {
  auto ptr1 = c10::make_intrusive<TestClass, TestClassNull>(12);
  auto* p = ptr1.release();
  auto pi = c10::intrusive_ptr<TestClass, TestClassNull>::reclaim(p);
  ASSERT_EQ(pi.ref_use_count(), 1);
  ASSERT_EQ(pi.weak_use_count(), 1);
  ASSERT_EQ(pi.defined(), true);
  ASSERT_EQ(p, pi.get());

  auto p2 = c10::intrusive_ptr<TestClass, TestClassNull>::reclaim_copy(p);
  ASSERT_EQ(p2.ref_use_count(), 2);
  ASSERT_EQ(p2.weak_use_count(), 1);
}

TEST(IntrusivePtrTEST, test_reclaim_weak) {
  auto ptr1 = c10::make_intrusive<TestClass, TestClassNull>(12);
  auto tmp = c10::weak_intrusive_ptr<TestClass, TestClassNull>(ptr1);
  auto* p = tmp.release();
  auto pw_weak = c10::weak_intrusive_ptr<TestClass, TestClassNull>::reclaim(p);
  ASSERT_EQ(pw_weak.ref_use_count(), 1);
  ASSERT_EQ(pw_weak.weak_use_count(), 2);
  ASSERT_EQ(pw_weak.unsafe_get_target(), p);

  auto pw_weak2 =
      c10::weak_intrusive_ptr<TestClass, TestClassNull>::reclaim_copy(p);
  ASSERT_EQ(pw_weak2.ref_use_count(), 1);
  ASSERT_EQ(pw_weak2.weak_use_count(), 3);
  ASSERT_EQ(pw_weak2.unsafe_get_target(), p);
}

TEST(IntrusivePtrTEST, test_maybe_owned) {
  auto ptr1 = c10::make_intrusive<TestClass>(12);
  print_count(ptr1);
  auto borrowed = c10::MaybeOwned<decltype(ptr1)>::borrowed(ptr1);
  print_count(*borrowed);
  ASSERT_EQ(borrowed->ref_use_count(), 1);
  ASSERT_EQ(borrowed->weak_use_count(), 1);

  auto ptr2 = c10::make_intrusive<TestClass>(122);
  print_count(ptr2);
  auto owned = c10::MaybeOwned<decltype(ptr2)>::owned(std::move(ptr2));
  print_count(*owned);
}
