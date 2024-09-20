#pragma once

#include <c10/core/DeviceType.h>

#include <functional>
#include <ostream>
#include <string>

namespace c10 {
#define C10_FORALL_BACKEND_COMPONENTS(_, extra) \
  _(CPU, extra)                                 \
  _(CUDA, extra)                                \
  _(Meta, extra)

#define C10_FORALL_FUNCTIONALITY_KEYS(_) \
  _(Dense, Dense)                        \
  _(AutogradFunctionality, Autograd)

enum class BackendComponent : uint8_t {
  InvalidBit = 0,
#define DEFINE_BACKEND_COMPONENT(n, _) n##Bit,
  C10_FORALL_BACKEND_COMPONENTS(DEFINE_BACKEND_COMPONENT, unused)
#undef DEFINE_BACKEND_COMPONENT

      EndOfBackendKeys = MetaBit,
};

enum class DispatchKey : uint16_t {
  Undefined = 0,
  CatchAll = Undefined,

  // Per-Backend Functionality Key
  Dense,

  BackendSelect,

  ADInplaceOrView,

  // Per-Backend Functionality Key
  AutogradFunctionality,

  AutocastCPU,
  AutocastCUDA,

  EndOfFunctionlityKeys,

#define DEFINE_PER_BACKEND_KEYS_FOR_BACKEND(n, prefix) prefix##n,

#define DEFINE_PER_BACKEND_KEYS(fullname, prefix)      \
  StartOf##fullname##Backends,                         \
      C10_FORALL_BACKEND_COMPONENTS(                   \
          DEFINE_PER_BACKEND_KEYS_FOR_BACKEND, prefix) \
          EndOf##fullname##Backends = prefix##Meta,

  C10_FORALL_FUNCTIONALITY_KEYS(DEFINE_PER_BACKEND_KEYS)

#undef DEFINE_PER_BACKEND_KEYS
#undef DEFINE_PER_BACKEND_KEYS_FOR_BACKEND

      EndOfRuntimeBackendKeys = EndOfAutogradFunctionalityBackends,

  // Alias Dispatch Keys
  Autograd,
  CompositeImplicitAutograd,
  CompositeExplicitAutograd,

  StartOfAliasKeys = Autograd,
  EndOfAliasKeys = CompositeExplicitAutograd,
};

constexpr bool isPerBackendFunctionalityKey(DispatchKey k) {
  if (k == DispatchKey::Dense or k == DispatchKey::AutogradFunctionality) {
    return true;
  } else {
    return false;
  }
}

constexpr uint8_t num_functionality_keys =
    static_cast<uint8_t>(DispatchKey::EndOfFunctionlityKeys);

constexpr uint8_t num_backends =
    static_cast<uint8_t>(BackendComponent::EndOfBackendKeys);

constexpr uint8_t numPerBackendFunctionalityKeys() {
  uint8_t count = 0;
  for (uint8_t k = 0; k <= num_functionality_keys; k++) {
    if (isPerBackendFunctionalityKey(static_cast<DispatchKey>(k))) {
      count++;
    }
  }
  return count;
}

constexpr uint16_t full_backend_mask =
    (static_cast<uint16_t>(1) << num_backends) - 1;

constexpr BackendComponent toBackendComponent(DispatchKey k) {
  if (k >= DispatchKey::StartOfDenseBackends and
      k <= DispatchKey::EndOfDenseBackends) {
    return static_cast<BackendComponent>(
        static_cast<uint8_t>(k) -
        static_cast<uint8_t>(DispatchKey::StartOfDenseBackends));
  } else if (
      k >= DispatchKey::StartOfAutogradFunctionalityBackends and
      k <= DispatchKey::EndOfAutogradFunctionalityBackends) {
    return static_cast<BackendComponent>(
        static_cast<uint8_t>(k) -
        static_cast<uint8_t>(
            DispatchKey::StartOfAutogradFunctionalityBackends));
  } else {
    return BackendComponent::InvalidBit;
  }
}

constexpr DispatchKey toFunctionalityKey(DispatchKey k) {
  if (k <= DispatchKey::EndOfFunctionlityKeys) {
    return k;
  } else if (k <= DispatchKey::EndOfDenseBackends) {
    return DispatchKey::Dense;
  } else if (k <= DispatchKey::EndOfAutogradFunctionalityBackends) {
    return DispatchKey::AutogradFunctionality;
  } else {
    return DispatchKey::Undefined;
  }
}

BackendComponent toBackendComponent(DeviceType device_type);

constexpr DispatchKey toRuntimePerBackendFunctionalityKey(
    DispatchKey functionality_k,
    BackendComponent backend_k) {
  if (functionality_k == DispatchKey::Dense) {
    return static_cast<DispatchKey>(
        static_cast<uint8_t>(DispatchKey::StartOfDenseBackends) +
        static_cast<uint8_t>(backend_k));
  } else if (functionality_k == DispatchKey::AutogradFunctionality) {
    return static_cast<DispatchKey>(
        static_cast<uint8_t>(
            DispatchKey::StartOfAutogradFunctionalityBackends) +
        static_cast<uint8_t>(backend_k));
  } else {
    return DispatchKey::Undefined;
  }
}

} // namespace c10


namespace std {
template <>
struct hash<c10::DispatchKey> {
  size_t operator()(c10::DispatchKey x) const {
    return static_cast<size_t>(x);
  }
};

} // namespace std
