#pragma once

#include <c10/util/Macros.h>
#include <memory>
#include <utility>

namespace c10 {
using DeleterFnPtr = void (*)(void*);

namespace detail {
void deleteNothing(void*);

class UniqueVoidPtr {
 private:
  void* data_;
  std::unique_ptr<void, DeleterFnPtr> ctx_;

 public:
  UniqueVoidPtr() : data_(nullptr), ctx_(nullptr, &detail::deleteNothing) {}
  explicit UniqueVoidPtr(void* data)
      : data_(data), ctx_(nullptr, &detail::deleteNothing) {}
  UniqueVoidPtr(void* data, void* ctx, DeleterFnPtr deleter)
      : data_(data), ctx_(ctx, deleter) {}

  void* operator->() const {
    return data_;
  }

  void clear() {
    ctx_ = nullptr;
    data_ = nullptr;
  }

  void* get() const {
    return data_;
  }

  void* get_context() const {
    return ctx_.get();
  }

  void* release_context() {
    return ctx_.release();
  }

  std::unique_ptr<void, DeleterFnPtr>&& move_context() {
    return std::move(ctx_);
  }

  [[nodiscard]] bool compare_exchange_deleter(
      DeleterFnPtr expected_deleter,
      DeleterFnPtr new_deleter) {
    if (get_deleter() != expected_deleter) {
      return false;
    }
    ctx_ = std::unique_ptr<void, DeleterFnPtr>(ctx_.release(), new_deleter);
    return true;
  }

  template <typename T>
  T* cast_context(DeleterFnPtr expected_deleter) const {
    if (get_deleter() != expected_deleter) {
      return nullptr;
    }
    return static_cast<T*>(get_context());
  }
  
  operator bool() const {
    return data_ || ctx_;
  }

  DeleterFnPtr get_deleter() const {
    return ctx_.get_deleter();
  }
};

inline bool operator==(const UniqueVoidPtr& sp, std::nullptr_t) noexcept {
  return !sp;
}
inline bool operator==(std::nullptr_t, const UniqueVoidPtr& sp) noexcept {
  return !sp;
}
inline bool operator!=(const UniqueVoidPtr& sp, std::nullptr_t) noexcept {
  return sp;
}
inline bool operator!=(std::nullptr_t, const UniqueVoidPtr& sp) noexcept {
  return sp;
}
} // namespace detail
} // namespace c10
