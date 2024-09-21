#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>
#include <c10/core/DispatchKey.h>
#include <c10/util/BitsUtil.h>
#include <c10/util/Exception.h>
#include <c10/util/Macros.h>
#include <c10/util/MaybeOwned.h>

#include <array>
#include <initializer_list>
#include <iterator>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace c10 {

struct FunctionalityOffsetAndMask {
  FunctionalityOffsetAndMask() = default;
  FunctionalityOffsetAndMask(uint8_t offset, uint64_t mask)
      : offset(offset), mask(mask) {}

  uint16_t offset{};
  uint64_t mask{};
};

C10_API std::array<FunctionalityOffsetAndMask, num_functionality_keys>
initializeFunctionalityOffsetsAndMasks();

C10_ALWAYS_INLINE static const std::
    array<FunctionalityOffsetAndMask, num_functionality_keys>&
    offsetsAndMasks() {
  static auto offsets_and_masks = initializeFunctionalityOffsetsAndMasks();
  return offsets_and_masks;
}

class DispatchKeySet final {
 public:
  enum Full { FULL };
  enum FullAfter { FULL_AFTER };
  enum Raw { RAW };

  constexpr DispatchKeySet() = default;

  constexpr DispatchKeySet(Full)
      : repr_((1ULL << (num_backends + num_functionality_keys - 1)) - 1) {}

  constexpr DispatchKeySet(FullAfter, DispatchKey t)
      : repr_(
            (1ULL
             << (num_backends + static_cast<uint8_t>(toFunctionalityKey(t)) -
                 1)) -
            1) {}

  constexpr DispatchKeySet(Raw, uint64_t x) : repr_(x) {}

  constexpr explicit DispatchKeySet(BackendComponent k) {
    if (k == BackendComponent::InvalidBit) {
      repr_ = 0;
    } else {
      repr_ = 1ULL << (static_cast<uint8_t>(k) - 1);
    }
  }

  constexpr explicit DispatchKeySet(DispatchKey k) {
    // NOLINTNEXTLINE(bugprone-branch-clone)
    if (k == DispatchKey::Undefined) {
      repr_ = 0;
    } else if (k <= DispatchKey::EndOfFunctionlityKeys) {
      uint64_t functionality_val = 1ULL
          << (num_backends + static_cast<uint8_t>(k) - 1);
      repr_ = functionality_val;
    } else if (k <= DispatchKey::EndOfRuntimeBackendKeys) {
      auto functionality_k = toFunctionalityKey(k);
      uint64_t functionality_val = 1ULL
          << (num_backends + static_cast<uint8_t>(functionality_k) - 1);
      auto backend_k = toBackendComponent(k);
      uint64_t backend_val = 1ULL << (static_cast<uint8_t>(backend_k) - 1);
      repr_ = functionality_val + backend_val;
    } else {
      repr_ = 0;
    }
  }

  constexpr explicit DispatchKeySet(std::initializer_list<DispatchKey> ks)
      : repr_(keys_to_repr(ks)) {}

  constexpr explicit DispatchKeySet(std::initializer_list<BackendComponent> ks)
      : repr_(backend_bits_to_repr(ks)) {}

  constexpr uint64_t keys_to_repr(std::initializer_list<DispatchKey> ks) {
    uint64_t repr = 0;
    for (auto k : ks) {
      repr |= DispatchKeySet(k).repr_;
    }
    return repr;
  }

  constexpr uint64_t backend_bits_to_repr(
      std::initializer_list<BackendComponent> ks) {
    uint64_t repr = 0;
    for (auto k : ks) {
      repr |= DispatchKeySet(k).repr_;
    }
    return repr;
  }

  inline bool has(DispatchKey k) const {
    TORCH_CHECK(k != DispatchKey::Undefined, "Undefined key is not allowed.");
    return has_all(DispatchKeySet(k));
  }

  constexpr bool has_backend(BackendComponent k) const {
    return has_all(DispatchKeySet(k));
  }

  constexpr bool has_all(DispatchKeySet ks) const {
    return static_cast<bool>((repr_ & ks.repr_) == ks.repr_);
  }

  constexpr bool has_any(DispatchKeySet ks) const {
    TORCH_CHECK(
        ((ks.repr_ & full_backend_mask) == 0) ||
        ((ks &
          DispatchKeySet(
              {DispatchKey::Dense, DispatchKey::AutogradFunctionality})) == 0));
    return static_cast<bool>(repr_ & ks.repr_);
  }

  bool isSupersetOf(DispatchKeySet ks) const {
    return (repr_ & ks.repr_) == ks.repr_;
  }

  constexpr DispatchKeySet operator|(DispatchKeySet other) const {
    return DispatchKeySet(repr_ | other.repr_);
  }

  constexpr DispatchKeySet operator&(DispatchKeySet other) const {
    return DispatchKeySet(repr_ & other.repr_);
  }

  // compute the set difference self - other
  // but ONLY for the functionality keys
  constexpr DispatchKeySet operator-(DispatchKeySet other) const {
    return DispatchKeySet(repr_ & (full_backend_mask | ~other.repr_));
  }

  constexpr DispatchKeySet operator^(DispatchKeySet other) const {
    return DispatchKeySet(repr_ ^ other.repr_);
  }

  constexpr bool operator==(DispatchKeySet other) const {
    return repr_ == other.repr_;
  }

  constexpr bool operator!=(DispatchKeySet other) const {
    return repr_ != other.repr_;
  }

  [[nodiscard]] constexpr DispatchKeySet add(DispatchKey k) const {
    return *this | DispatchKeySet(k);
  }

  [[nodiscard]] constexpr DispatchKeySet add(DispatchKeySet ks) const {
    return *this | ks;
  }

  [[nodiscard]] constexpr DispatchKeySet remove(DispatchKey k) const {
    return DispatchKeySet(
        repr_ & ~(DispatchKeySet(k).repr_ & ~full_backend_mask));
  }

  [[nodiscard]] constexpr DispatchKeySet remove_backend(
      BackendComponent b) const {
    return DispatchKeySet(repr_ & ~(DispatchKeySet(b).repr_));
  }

  bool empty() const {
    return repr_ == 0;
  }

  uint64_t raw_repr() {
    return repr_;
  }

  uint8_t indexOfHighestBit() const {
    static_assert(
        std::is_same_v<uint64_t, decltype(repr_)>,
        "indexOfHighestBit only works on unsigned integral types");
    return 64 - detail::countLeadingZeros(repr_);
  }

  DispatchKey highestFunctionalityKey() const {
    auto functionality_idx = indexOfHighestBit();
    if (functionality_idx < num_backends) {
      return DispatchKey::Undefined;
    }
    return static_cast<DispatchKey>(functionality_idx - num_backends);
  }

  BackendComponent highestBackendKey() const {
    auto backend_idx =
        DispatchKeySet(repr_ & full_backend_mask).indexOfHighestBit();
    if (backend_idx == 0) {
      return BackendComponent::InvalidBit;
    }
    return static_cast<BackendComponent>(backend_idx);
  }

  int getDispatchTableIndexForDispatchKeySet() const {
    auto functionality_idx = static_cast<uint8_t>(highestFunctionalityKey());
    TORCH_CHECK(
        functionality_idx >= 0 and functionality_idx < num_functionality_keys,
        "functionality_idx must be non-negative");
    auto offset_and_mask = offsetsAndMasks()[functionality_idx];
    // Mask the functionality bits out first, then right-shift by 1.
    // right-shifting by 1 because everything is zero-indexed.
    // E.g. 000001 (CPU) should give us an offset of 0, 000010 (CUDA) should
    // give us an offset of 1, etc.
    auto backend_idx =
        DispatchKeySet((repr_ & offset_and_mask.mask) >> 1).indexOfHighestBit();
    return offset_and_mask.offset + backend_idx;
  }

  // returns the "index" of the highest priority backend in the keyset.
  // This is pretty similar to getBackendKey(), but:
  // - It's hotpath code (part of the runtime bitset calculation)
  // - I's returns an integer index, not an enum value
  // - Everything is shifted to the right by 1.
  //   BackendComponent::InvalidBit is technically the lowest enum value,
  //   but it isn't included in the runtime table. So CPUBit = 1, CUDABit = 2,
  //   etc.
  uint64_t getBackendIndex() const {
    return DispatchKeySet((repr_ & full_backend_mask) >> 1).indexOfHighestBit();
  }

 private:
  uint64_t repr_ = 0;
  constexpr DispatchKeySet(uint64_t repr) : repr_(repr) {}

 public:
  // iterator for DispatchKeySet
  class iterator {
   public:
    using self_type = iterator;
    using iterator_category = std::input_iterator_tag;
    using value_type = DispatchKey;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using pointer = value_type*;

    static const uint8_t end_iter_val = num_backends + num_functionality_keys;
    static const uint8_t end_iter_key_val = num_functionality_keys;

    explicit iterator(
        const uint64_t* data_ptr,
        uint8_t next_functionality = num_backends,
        uint8_t next_backend = 0)
        : data_ptr_(data_ptr),
          next_functionality_(next_functionality),
          next_backend_(next_backend),
          current_dispatchkey_idx_(end_iter_val),
          current_backendcomponent_idx_(end_iter_key_val) {
      TORCH_CHECK(
          next_functionality_ >= num_backends,
          "num_backends=",
          static_cast<uint32_t>(num_backends),
          " next_functionality_=",
          static_cast<uint32_t>(next_functionality_));

      ++(*this);
    }

    C10_API self_type& operator++();

    self_type operator++(int) {
      self_type i = *this;
      ++(*this);
      return i;
    }

    bool operator==(const self_type& rhs) const {
      return next_functionality_ == rhs.next_functionality_ and
          current_dispatchkey_idx_ == rhs.current_dispatchkey_idx_ and
          next_backend_ == rhs.next_backend_ and
          current_backendcomponent_idx_ == rhs.current_backendcomponent_idx_;
    }

    bool operator!=(const self_type& rhs) const {
      return next_functionality_ != rhs.next_functionality_ or
          current_dispatchkey_idx_ != rhs.current_dispatchkey_idx_ or
          next_backend_ != rhs.next_backend_ or
          current_backendcomponent_idx_ != rhs.current_backendcomponent_idx_;
    }

    DispatchKey operator*() const {
      auto functionality_key =
          static_cast<DispatchKey>(current_dispatchkey_idx_);
      if (isPerBackendFunctionalityKey(functionality_key)) {
        auto next_key = toRuntimePerBackendFunctionalityKey(
            functionality_key,
            static_cast<BackendComponent>(current_backendcomponent_idx_));
        TORCH_CHECK(
            toBackendComponent(next_key) ==
                static_cast<BackendComponent>(current_backendcomponent_idx_),
            "BackendComponent mismatch");
        return next_key;
      } else {
        return functionality_key;
      }
    }

   private:
    const uint64_t* data_ptr_;
    uint8_t next_functionality_;
    uint8_t next_backend_;
    uint8_t current_dispatchkey_idx_;
    uint8_t current_backendcomponent_idx_;
  };

 public:
  iterator begin() const {
    return iterator(&repr_);
  }

  iterator end() const {
    return iterator(&repr_, iterator::end_iter_val);
  }
};

} // namespace c10
