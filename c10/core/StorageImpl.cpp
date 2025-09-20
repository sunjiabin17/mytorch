#include <c10/core/StorageImpl.h>

namespace c10 {
void StorageImpl::throw_data_ptr_access_error() const {
  TORCH_CHECK(false, "Cannot access data pointer of Storage that is invalid.");
}

} // namespace c10
