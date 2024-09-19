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
  constexpr DispatchKeySet() = default;
};

} // namespace c10
