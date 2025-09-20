#include <c10/core/Storage.h>

namespace c10 {

bool isSharedStorageAlias(const Storage& storage0, const Storage& storage1) {
  // [TODO] refcounted_deleter
  // return storage0.data_ptr().get_context() == storage1.data_ptr().get_context();
  return false;
}

} // namespace c10
