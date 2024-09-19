#pragma once

#include <functional>
#include <ostream>
#include <string>

namespace c10 {
#define C10_FORALL_BACKEND_COMPONENTS(_, extra) \
  _(CPU, extra)                                 \
  _(CUDA, extra)                                \
  _(Meta, extra)

#define C10_FORALL_FUNCTIONALITY_KEYS(_) \
  _(Dense, )                             \
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

  Dense,

  ADInplaceOrView,

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

};

} // namespace c10
