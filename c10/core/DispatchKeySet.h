#include <c10/core/Device.h>
#include <c10/core/DeviceType.h>
#include <c10/core/DispatchKey.h>
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
        std::is_integral_v<decltype(repr_)> and
            std::is_unsigned_v<decltype(repr_)>,
        "indexOfHighestBit only works on unsigned integral types");
    return 64 - __builtin_clzll(repr_);
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

 private:
  uint64_t repr_ = 0;
  constexpr DispatchKeySet(uint64_t repr) : repr_(repr) {}
};

} // namespace c10
