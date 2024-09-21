#include <c10/core/DispatchKey.h>
#include <c10/core/DispatchKeySet.h>
#include <c10/util/BitsUtil.h>
#include <cassert>
#include <climits>

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

  uint64_t masked_functionality_bits =
      c10::detail::maskTrailingZeros<uint64_t>(next_functionality_) &
      *data_ptr_;
  uint64_t masked_backend_bits =
      c10::detail::maskTrailingZeros<uint64_t>(next_backend_) &
      full_backend_mask & *data_ptr_;

  uint64_t first_functionality_idx =
      c10::detail::findFirstSet(masked_functionality_bits);
  uint64_t first_backendcomponent_idx =
      c10::detail::findFirstSet(masked_backend_bits);

  if (first_functionality_idx == std::numeric_limits<uint64_t>::max() or
      next_functionality_ == iterator::end_iter_val) {
    // set up state to be same as end()
    next_functionality_ = iterator::end_iter_val;
    current_dispatchkey_idx_ = iterator::end_iter_key_val;
    next_backend_ = 0;
    current_backendcomponent_idx_ = iterator::end_iter_key_val;
    return *this;
  }

  // +1 is because of DispatchKey::Undefined and BackendComponent::InvalidBit
  auto new_next_functionality_idx = first_functionality_idx + 1;
  auto new_next_backend_idx = first_backendcomponent_idx + 1;

  auto next_dispatchkey_idx = new_next_functionality_idx - num_backends;

  if (isPerBackendFunctionalityKey(
          static_cast<DispatchKey>(next_dispatchkey_idx))) {
    if (first_backendcomponent_idx == std::numeric_limits<uint64_t>::max()) {
      // case 1: if the current backend is undefined, then there is no valid
      // backend instance of this functionality key so we can skip it.
      next_functionality_ = new_next_functionality_idx;
      ++(*this);
      return *this;
    }

    // we know what the current backend and functionality bits are
    current_dispatchkey_idx_ = next_dispatchkey_idx;
    current_backendcomponent_idx_ = new_next_backend_idx;

    // next, we neet to set up the masks for the next iteration
    uint64_t next_backendcomponent_bits =
        c10::detail::maskTrailingZeros<uint64_t>(
            first_backendcomponent_idx + 1) &
        full_backend_mask & *data_ptr_;
    uint64_t next_backendcomponent_idx =
        c10::detail::findFirstSet(next_backendcomponent_bits);
    if (next_backendcomponent_idx == std::numeric_limits<uint64_t>::max()) {
      // case 2: the current backend is valid, but there is not another backend
      // in the keyset. In this case, we need to bump the functionality mask and
      // reset the backend mask for the next increment
      next_functionality_ = new_next_functionality_idx;
      next_backend_ = 0;
    } else {
      // case 3: we have another backend to iterate over. We want to iterate
      // over the same functionality bit next time, but a different backend bit.
      next_backend_ = first_backendcomponent_idx + 1;
    }
  } else {
    TORCH_CHECK(next_backend_ == 0);
    current_dispatchkey_idx_ = next_dispatchkey_idx;
    next_functionality_ = new_next_functionality_idx;
  }
  return *this;
}

} // namespace c10
