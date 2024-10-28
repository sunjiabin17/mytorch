#pragma once

#include <c10/util/Exception.h>
#include <c10/util/Half.h>

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <type_traits>

namespace c10 {

#define AT_FORALL_SCALAR_TYPES(_) \
  _(uint8_t, Byte)                \
  _(int8_t, Char)                 \
  _(int16_t, Short)               \
  _(int, Int)                     \
  _(int64_t, Long)                \
  _(c10::Half, Half)              \
  _(float, Float)                 \
  _(double, Double)               \
  _(bool, Bool)                   \
  _(uint16_t, UInt16)             \
  _(uint32_t, UInt32)             \
  _(uint64_t, UInt64)

enum class ScalarType : int8_t {
#define DEFINE_ENUM(_1, n) n,
  AT_FORALL_SCALAR_TYPES(DEFINE_ENUM)
#undef DEFINE_ENUM
      Undefined,
  NumOptions
};

constexpr uint16_t NumScalarTypes =
    static_cast<uint16_t>(ScalarType::NumOptions);

namespace impl {
template <c10::ScalarType>
struct ScalarTypeToCPPType;

#define SPECIALIZE_ScalarTypeToCPPType(cpp_type, scalar_type) \
  template <>                                                 \
  struct ScalarTypeToCPPType<c10::ScalarType::scalar_type> {  \
    using type = cpp_type;                                    \
    static type t;                                            \
  };

AT_FORALL_SCALAR_TYPES(SPECIALIZE_ScalarTypeToCPPType)

#undef SPECIALIZE_ScalarTypeToCPPType

template <c10::ScalarType N>
using ScalarTypeToCPPType_t = typename ScalarTypeToCPPType<N>::type;

} // namespace impl

template <typename T>
struct CPPTypeToScalarType;

#define SPECIALIZE_CPPTypeToScalarType(cpp_type, scalar_type)                  \
  template <>                                                                  \
  struct CPPTypeToScalarType<cpp_type>                                         \
      : std::                                                                  \
            integral_constant<c10::ScalarType, c10::ScalarType::scalar_type> { \
  };

AT_FORALL_SCALAR_TYPES(SPECIALIZE_CPPTypeToScalarType)

#undef SPECIALIZE_CPPTypeToScalarType

inline const char* toString(ScalarType t) {
#define DEFINE_CASE(_, name) \
  case ScalarType::name:     \
    return #name;

  switch (t) {
    AT_FORALL_SCALAR_TYPES(DEFINE_CASE)
    default:
      return "UNKNOWN";
  }
#undef DEFINE_CASE
}

inline size_t elementSize(ScalarType t) {
#define DEFINE_ELEMENT_SIZE_CASE(ctype, name) \
  case ScalarType::name:                      \
    return sizeof(ctype);

  switch (t) {
    AT_FORALL_SCALAR_TYPES(DEFINE_ELEMENT_SIZE_CASE)
    default:
      TORCH_CHECK(false, "Unknown scalar type");
  }
#undef DEFINE_ELEMENT_SIZE_CASE
}

inline bool isIntegralType(ScalarType t, bool includeBool = false) {
  bool is_integral =
      (t == ScalarType::Byte || t == ScalarType::Char || t == ScalarType::Int ||
       t == ScalarType::Long || t == ScalarType::Short ||
       t == ScalarType::UInt16 || t == ScalarType::UInt32 ||
       t == ScalarType::UInt64);
  return is_integral || (includeBool && t == ScalarType::Bool);
}

inline bool isReducedFloatingType(ScalarType t) {
  return t == ScalarType::Half;
}

inline bool isFloatingType(ScalarType t) {
  return t == ScalarType::Float || t == ScalarType::Double ||
      isReducedFloatingType(t);
}

inline std::ostream& operator<<(std::ostream& stream, ScalarType scalar_type) {
  return stream << toString(scalar_type);
}

} // namespace c10
