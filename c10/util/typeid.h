#pragma once

#include <c10/core/ScalarType.h>
#include <c10/util/Macros.h>

class C10_API TypeMeta final {
 public:
  TypeMeta() : type(c10::ScalarType::Undefined) {}

  TypeMeta(c10::ScalarType scalar_type) : type(scalar_type) {}
  c10::ScalarType type;
};
