#include <c10/core/GeneratorImpl.h>

namespace c10 {

GeneratorImpl::GeneratorImpl(Device device, DispatchKeySet key_set)
    : device_(device), key_set_(key_set) {}

c10::intrusive_ptr<GeneratorImpl> GeneratorImpl::clone() const {
  auto res = this->clone_impl();
  c10::raw::intrusive_ptr::incref(res);
  c10::raw::weak_intrusive_ptr::incref(res);
  return c10::intrusive_ptr<GeneratorImpl>::reclaim(res);
}

Device GeneratorImpl::device() const {
  return device_;
}

} // namespace c10
