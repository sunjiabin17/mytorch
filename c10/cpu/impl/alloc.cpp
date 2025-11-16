#include <c10/cpu/impl/alloc.h>
#include <c10/util/Exception.h>
#include <cstdlib>
#include <new>

namespace c10 {

void* alloc_cpu(size_t nbytes) {
  if (nbytes == 0) {
    return nullptr;
  }

  TORCH_INTERNAL_ASSERT(nbytes >= 0);
  void* data = std::malloc(nbytes); //NOLINT(cppcoreguidelines-no-malloc)
  if (!data) {
    throw std::bad_alloc();
  }
  return data;
}

void free_cpu(void* data) {
  std::free(data); // NOLINT(cppcoreguidelines-no-malloc)
}

} // namespace c10
