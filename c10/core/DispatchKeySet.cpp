#include <c10/core/DispatchKey.h>
#include <c10/core/DispatchKeySet.h>

namespace c10 {

std::array<FunctionalityOffsetAndMask, num_functionality_keys>
initializeFunctionalityOffsetsAndMasks() {
  std::array<FunctionalityOffsetAndMask, num_functionality_keys>
      offsets_and_masks;

  offsets_and_masks[0] = FunctionalityOffsetAndMask(0, 0);
  for (size_t functionality_idx = 1; functionality_idx < num_functionality_keys;
       ++functionality_idx) {
    auto pre_offset_and_mask = offsets_and_masks[functionality_idx - 1];
    auto k = static_cast<DispatchKey>(functionality_idx);

    auto offset = pre_offset_and_mask.offset +
        (pre_offset_and_mask.mask == 0 ? 1 : num_backends);
    auto mask = isPerBackendFunctionalityKey(k) ? full_backend_mask : 0;
    offsets_and_masks[functionality_idx] =
        FunctionalityOffsetAndMask(offset, mask);
  }
  TORCH_CHECK(
      offsets_and_masks[num_functionality_keys - 1].offset ==
          (num_runtime_entries - 1),
      "num_runtime_entries: ",
      num_runtime_entries,
      " last offset: ",
      offsets_and_masks[num_functionality_keys - 1].offset);
  return offsets_and_masks;
}



DispatchKeySet::iterator& DispatchKeySet::iterator::operator++() {
  TORCH_CHECK(next_functionality_ <= iterator::end_iter_val);
  TORCH_CHECK(next_backend_ <= num_backends);

  // TODO

  return *this;
}

} // namespace c10
