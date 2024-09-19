#pragma once

#include <c10/util/exception.h>
#include <c10/util/maybe_owned.h>
#include <c10/util/macros.h>

#include <atomic>
#include <cassert>
#include <cstddef>
#include <type_traits>

namespace c10 {

namespace raw {
struct DontIncreaseRefcount {};
} // namespace raw

// NOLINTNEXTLINE(cppcoreguidelines-virtual-class-destructor)
class C10_API intrusive_ptr_target {
  mutable std::atomic<uint32_t> refcount_;
  mutable std::atomic<uint32_t> weakcount_;

  template <class TTarget, class NullType>
  friend class intrusive_ptr;

  template <class TTarget, class NullType>
  friend class weak_intrusive_ptr;

 protected:
  virtual ~intrusive_ptr_target() {
    assert(refcount_.load() == 0);
    assert(weakcount_.load() == 0 or weakcount_.load() == 1);
  }

  constexpr intrusive_ptr_target() : refcount_(0), weakcount_(0) {}

  intrusive_ptr_target(intrusive_ptr_target&& /*other*/) noexcept
      : intrusive_ptr_target() {}

  intrusive_ptr_target& operator=(intrusive_ptr_target&& /*other*/) noexcept {
    return *this;
  }

  intrusive_ptr_target(const intrusive_ptr_target& /*other*/)
      : intrusive_ptr_target() {}

  intrusive_ptr_target& operator=(const intrusive_ptr_target& /*other*/) {
    return *this;
  }

 private:
  // this is called when refcount reaches zero.
  virtual void release_resources() {}
};

namespace detail {
inline uint32_t atomic_refcount_increment(std::atomic<uint32_t>& refcount) {
  return refcount.fetch_add(1, std::memory_order_acq_rel) + 1;
}

inline uint32_t atomic_weakcount_increment(std::atomic<uint32_t>& weakcount) {
  return weakcount.fetch_add(1, std::memory_order_acq_rel) + 1;
}

inline uint32_t atomic_refcount_decrement(std::atomic<uint32_t>& refcount) {
  return refcount.fetch_sub(1, std::memory_order_acq_rel) - 1;
}

inline uint32_t atomic_weakcount_decrement(std::atomic<uint32_t>& weakcount) {
  return weakcount.fetch_sub(1, std::memory_order_acq_rel) - 1;
}

template <class TTarget, class ToNullType, class FromNullType>
TTarget* assign_ptr_(TTarget* rhs) {
  if (FromNullType::null() == rhs) {
    return ToNullType::null();
  } else {
    return rhs;
  }
}

template <class TTarget>
struct intrusive_default_null final {
  static constexpr TTarget* null() noexcept {
    return nullptr;
  }
};

} // namespace detail

template <
    class TTarget,
    class NullType = detail::intrusive_default_null<TTarget>>
class intrusive_ptr final {
 private:
  TTarget* target_;

  static_assert(
      std::is_base_of<intrusive_ptr_target, TTarget>::value,
      "intrusive_ptr can only be used with types derived from intrusive_ptr_target");

  template <class TTarget2, class NullType2>
  friend class intrusive_ptr;
  friend class intrusive_ptr_target;

  void retain_() {
    if (target_ != NullType::null()) {
      uint32_t new_refcount =
          detail::atomic_refcount_increment(target_->refcount_);
      TORCH_CHECK(
          new_refcount != 1,
          "intrusive_ptr: internal error: retain_() called on a target after it reached 0.");
    }
  }

  void reset_() {
    if (target_ != NullType::null() and
        detail::atomic_refcount_decrement(target_->refcount_) == 0) {
      bool should_delete =
          target_->weakcount_.load(std::memory_order_acquire) == 1;
      if (!should_delete) {
        // refcount is 0, weakcount > 1, so there is still other weak_ptrs, we
        // should release resources
        const_cast<std::remove_const_t<TTarget>*>(target_)->release_resources();
        should_delete =
            detail::atomic_weakcount_decrement(target_->weakcount_) == 0;
      }
      if (should_delete) {
        // there is no weak_ptrs, so we can safely delete the target
        delete target_;
      }
    }
  }

  explicit intrusive_ptr(TTarget* target)
      : intrusive_ptr(target, raw::DontIncreaseRefcount{}) {
    if (target != NullType::null()) {
      TORCH_CHECK(
          target_->refcount_ == 0 and target_->weakcount_ == 0,
          "intrusive_ptr: newly created target had non-zero refcount or weakcount");
      target_->refcount_.store(1, std::memory_order_relaxed);
      target_->weakcount_.store(1, std::memory_order_relaxed);
    }
  }

 public:
  intrusive_ptr() noexcept
      : intrusive_ptr(NullType::null(), raw::DontIncreaseRefcount{}) {}

  intrusive_ptr(std::nullptr_t) noexcept
      : intrusive_ptr(NullType::null(), raw::DontIncreaseRefcount{}) {}

  intrusive_ptr(TTarget* target, raw::DontIncreaseRefcount) noexcept
      : target_(target) {}

  ~intrusive_ptr() {
    reset_();
  }

  // copy constructor
  intrusive_ptr(const intrusive_ptr& rhs) : target_(rhs.target_) {
    retain_();
  }

  template <class From, class FromNullType>
  intrusive_ptr(const intrusive_ptr<From, FromNullType>& rhs)
      : target_(
            detail::assign_ptr_<TTarget, NullType, FromNullType>(rhs.target_)) {
    static_assert(
        std::is_convertible<From*, TTarget*>::value,
        "Type mismatch in intrusive_ptr copy constructor");
    retain_();
  }

  // copy assignment
  // NOLINTNEXTLINE(bugprone-unhandled-self-assignment)
  intrusive_ptr& operator=(const intrusive_ptr& rhs) & noexcept {
    // NOLINTNEXTLINE(*assign-operator, *assignment-signature)
    return operator=<TTarget, NullType>(rhs);
  }

  template <class From, class FromNullType>
  intrusive_ptr& operator=(
      const intrusive_ptr<From, FromNullType>& rhs) & noexcept {
    static_assert(
        std::is_convertible<From*, TTarget*>::value,
        "Type mismatch in intrusive_ptr copy assignment");
    intrusive_ptr tmp(rhs);
    swap(tmp);
    return *this;
  }

  // move constructor
  intrusive_ptr(intrusive_ptr&& rhs) noexcept : target_(rhs.target_) {
    rhs.target_ = NullType::null();
  }

  template <class From, class FromNullType>
  intrusive_ptr(intrusive_ptr<From, FromNullType>&& rhs) noexcept
      : target_(
            detail::assign_ptr_<TTarget, NullType, FromNullType>(rhs.target_)) {
    static_assert(
        std::is_convertible<From*, TTarget*>::value,
        "Type mismatch in intrusive_ptr move constructor");
    rhs.target_ = FromNullType::null();
  }

  // move assignment
  intrusive_ptr& operator=(intrusive_ptr&& rhs) & noexcept {
    // NOLINTNEXTLINE(*assign*)
    return operator=<TTarget, NullType>(std::move(rhs));
  }

  template <class From, class FromNullType>
  intrusive_ptr& operator=(intrusive_ptr<From, FromNullType>&& rhs) & noexcept {
    static_assert(
        std::is_convertible<From*, TTarget*>::value,
        "Type mismatch in intrusive_ptr move assignment");
    intrusive_ptr tmp(std::move(rhs));
    swap(tmp);
    return *this;
  }

  // operator bool
  explicit operator bool() const noexcept {
    return target_ != NullType::null();
  }

  // operator* and ->
  TTarget& operator*() const noexcept {
    return *target_;
  }

  TTarget* operator->() const noexcept {
    return target_;
  }

  TTarget* get() const noexcept {
    return target_;
  }

  void reset() {
    reset_();
    target_ = NullType::null();
  }

  void swap(intrusive_ptr& rhs) {
    std::swap(target_, rhs.target_);
  }

  bool defined() const noexcept {
    return target_ != NullType::null();
  }

  TTarget* release() noexcept {
    TTarget* result = target_;
    target_ = NullType::null();
    return result;
  }

  static intrusive_ptr reclaim(TTarget* owning_ptr) {
    // owning pointer must be one of the following:
    // 1. null
    // 2. refcount > 0
    TORCH_CHECK(
        owning_ptr == NullType::null() or
            owning_ptr->refcount_.load(std::memory_order_acquire) == 0 or
            owning_ptr->weakcount_.load(std::memory_order_acquire) > 0,
        "reclaim() received a invalid pointer that refcout > 0 and weakcount == 0");
    return intrusive_ptr(owning_ptr, raw::DontIncreaseRefcount{});
  }

  static intrusive_ptr reclaim_copy(TTarget* owning_ptr) {
    auto ret = reclaim(owning_ptr);
    ret.retain_();
    return ret;
  }

  template <class... Args>
  static intrusive_ptr make(Args&&... args) {
    return intrusive_ptr(new TTarget(std::forward<Args>(args)...));
  }

  uint32_t ref_use_count() const noexcept {
    if (target_ == NullType::null()) {
      return 0;
    } else {
      return target_->refcount_.load(std::memory_order_acquire);
    }
  }

  uint32_t weak_use_count() const noexcept {
    if (target_ == NullType::null()) {
      return 0;
    } else {
      return target_->weakcount_.load(std::memory_order_acquire);
    }
  }
};

template <
    class TTarget,
    class NullType = detail::intrusive_default_null<TTarget>,
    class... Args>
inline intrusive_ptr<TTarget, NullType> make_intrusive(Args&&... args) {
  return intrusive_ptr<TTarget, NullType>::make(std::forward<Args>(args)...);
}

template <class TTarget1, class NullType1, class TTarget2, class NullType2>
inline bool operator<(
    const intrusive_ptr<TTarget1, NullType1>& lhs,
    const intrusive_ptr<TTarget2, NullType2>& rhs) noexcept {
  return lhs.get() < rhs.get();
}

template <class TTarget1, class NullType1, class TTarget2, class NullType2>
inline bool operator==(
    const intrusive_ptr<TTarget1, NullType1>& lhs,
    const intrusive_ptr<TTarget2, NullType2>& rhs) noexcept {
  return lhs.get() == rhs.get();
}

template <class TTarget1, class NullType1>
inline bool operator==(
    const intrusive_ptr<TTarget1, NullType1>& lhs,
    std::nullptr_t) noexcept {
  return lhs.get() == nullptr;
}

template <class TTarget1, class NullType1>
inline bool operator==(
    std::nullptr_t,
    const intrusive_ptr<TTarget1, NullType1>& rhs) noexcept {
  return nullptr == rhs.get();
}

template <class TTarget1, class NullType1, class TTarget2, class NullType2>
inline bool operator!=(
    const intrusive_ptr<TTarget1, NullType1>& lhs,
    const intrusive_ptr<TTarget2, NullType2>& rhs) noexcept {
  return !operator==(lhs, rhs);
}

template <class TTarget1, class NullType1>
inline bool operator!=(
    const intrusive_ptr<TTarget1, NullType1>& lhs,
    std::nullptr_t) noexcept {
  return !operator==(lhs, lhs);
}

template <class TTarget1, class NullType1>
inline bool operator!=(
    std::nullptr_t,
    const intrusive_ptr<TTarget1, NullType1>& rhs) noexcept {
  return !operator==(rhs, rhs);
}

template <class TTarget1, class NullType1>
inline void swap(
    intrusive_ptr<TTarget1, NullType1>& lhs,
    intrusive_ptr<TTarget1, NullType1>& rhs) noexcept {
  lhs.swap(rhs);
}

template <typename T>
struct MaybeOwnedTraits<c10::intrusive_ptr<T>> {
  using owned_type = c10::intrusive_ptr<T>;
  using borrow_type = c10::intrusive_ptr<T>;

  static borrow_type createBorrow(const owned_type& from) {
    return borrow_type::reclaim(from.get());
  }

  static void assignBorrow(borrow_type& lhs, borrow_type rhs) {
    lhs.release();
    lhs = borrow_type::reclaim(rhs.get());
  }

  static void destroyBorrow(borrow_type& toDestroy) {
    toDestroy.release();
  }

  static const owned_type& referenceFromBorrow(const borrow_type& borrow) {
    return borrow;
  }

  static const owned_type* pointerFromBorrow(const borrow_type& borrow) {
    return &borrow;
  }

  static bool debugBorrowIsValid(const borrow_type& /*borrow*/) {
    return true;
  }
};

template <class TTarget, class NullType>
class weak_intrusive_ptr final {
 private:
  TTarget* target_;

  static_assert(
      std::is_base_of<intrusive_ptr_target, TTarget>::value,
      "weak_intrusive_ptr can only be used with types derived from intrusive_ptr_target");

  template <class TTarget2, class NullType2>
  friend class weak_intrusive_ptr;
  friend class intrusive_ptr_target;

  void retain_() {
    if (target_ != NullType::null()) {
      uint32_t new_weakcount =
          detail::atomic_weakcount_increment(target_->weakcount_);
      TORCH_CHECK(
          new_weakcount != 1,
          "weak_intrusive_ptr: internal error: retain_() called on a target after it reached 0.");
    }
  }

  void reset_() {
    if (target_ != NullType::null() and
        detail::atomic_weakcount_decrement(target_->weakcount_) == 0) {
      delete target_;
    }
  }

  explicit weak_intrusive_ptr(TTarget* target) : target_(target) {}

 public:
  explicit weak_intrusive_ptr(const intrusive_ptr<TTarget, NullType>& ptr)
      : weak_intrusive_ptr(ptr.get()) {
    retain_();
  }

  ~weak_intrusive_ptr() noexcept {
    reset_();
  }

  // copy constructor
  weak_intrusive_ptr(const weak_intrusive_ptr& rhs) : target_(rhs.target_) {
    retain_();
  }

  template <class From, class FromNullType>
  weak_intrusive_ptr(const weak_intrusive_ptr<From, FromNullType>& rhs)
      : target_(
            detail::assign_ptr_<TTarget, NullType, FromNullType>(rhs.target_)) {
    static_assert(
        std::is_convertible<From*, TTarget*>::value,
        "Type mismatch in weak_intrusive_ptr copy constructor");
    retain_();
  }

  // copy assignment
  weak_intrusive_ptr& operator=(const weak_intrusive_ptr& rhs) & noexcept {
    if (this == &rhs) {
      return *this;
    }
    // NOLINTNEXTLINE(*assign-operator, *assignment-signature)
    return operator=<TTarget, NullType>(rhs);
  }

  template <class From, class FromNullType>
  weak_intrusive_ptr& operator=(
      const weak_intrusive_ptr<From, FromNullType>& rhs) & noexcept {
    static_assert(
        std::is_convertible<From*, TTarget*>::value,
        "Type mismatch in weak_intrusive_ptr copy assignment");
    weak_intrusive_ptr tmp(rhs);
    swap(tmp);
    return *this;
  }

  // move constructor
  weak_intrusive_ptr(weak_intrusive_ptr&& rhs) noexcept : target_(rhs.target_) {
    rhs.target_ = NullType::null();
  }

  template <class From, class FromNullType>
  weak_intrusive_ptr(weak_intrusive_ptr<From, FromNullType>&& rhs) noexcept
      : target_(
            detail::assign_ptr_<TTarget, NullType, FromNullType>(rhs.target_)) {
    static_assert(
        std::is_convertible<From*, TTarget*>::value,
        "Type mismatch in weak_intrusive_ptr move constructor");
    rhs.target_ = FromNullType::null();
  }

  // move assignment
  weak_intrusive_ptr& operator=(weak_intrusive_ptr&& rhs) & noexcept {
    // NOLINTNEXTLINE(*assign*)
    return operator=<TTarget, NullType>(std::move(rhs));
  }

  template <class From, class FromNullType>
  weak_intrusive_ptr& operator=(
      weak_intrusive_ptr<From, FromNullType>&& rhs) & noexcept {
    static_assert(
        std::is_convertible<From*, TTarget*>::value,
        "Type mismatch in weak_intrusive_ptr move assignment");
    weak_intrusive_ptr tmp(std::move(rhs));
    swap(tmp);
    return *this;
  }

  void reset() noexcept {
    reset_();
  }

  void swap(weak_intrusive_ptr& rhs) noexcept {
    std::swap(target_, rhs.target_);
  }

  TTarget* unsafe_get_target() const noexcept {
    return target_;
  }

  uint32_t ref_use_count() const noexcept {
    if (target_ == NullType::null()) {
      return 0;
    } else {
      return target_->refcount_.load(std::memory_order_acquire);
    }
  }

  uint32_t weak_use_count() const noexcept {
    if (target_ == NullType::null()) {
      return 0;
    } else {
      return target_->weakcount_.load(std::memory_order_acquire);
    }
  }

  bool expire() const noexcept {
    return ref_use_count() == 0;
  }

  intrusive_ptr<TTarget, NullType> lock() const noexcept {
    if (expire()) {
      return intrusive_ptr<TTarget, NullType>();
    } else {
      auto refcount = target_->refcount_.load(std::memory_order_seq_cst);
      do {
        if (refcount == 0) {
          return intrusive_ptr<TTarget, NullType>();
        }
      } while (
          !target_->refcount_.compare_exchange_weak(refcount, refcount + 1));
      return intrusive_ptr<TTarget, NullType>(
          target_, raw::DontIncreaseRefcount{});
    }
  }

  TTarget* release() noexcept {
    TTarget* result = target_;
    target_ = NullType::null();
    return result;
  }

  static weak_intrusive_ptr reclaim(TTarget* owning_ptr) {
    // if refcount == 0, weakcount only must be >0
    // if refcount > 0, weakcount must be >1 for weak references to exist.
    // owning pointer must be one of the following:
    // 1. null
    // 2. weakcount > 1
    // 3. if refcount == 0, weakcount must be > 0
    // bad cases:
    // raw pointer(refcount == 0, weakcount == 0)
    // owning pointer without weak references (refcount == xx, weakcount == 1)
    TORCH_CHECK(
        owning_ptr == NullType::null() or owning_ptr->weakcount_.load() > 1 or
            owning_ptr->refcount_.load() == 0 and
                owning_ptr->weakcount_.load() > 0,
        "weak_intrusive_ptr: reclaim() received a invalid pointer");
    return weak_intrusive_ptr(owning_ptr);
  }

  static weak_intrusive_ptr reclaim_copy(TTarget* owning_ptr) {
    auto ret = reclaim(owning_ptr);
    ret.retain_();
    return ret;
  }

  template <class TTarget1, class NullType1, class TTarget2, class NullType2>
  friend bool operator<(
      const weak_intrusive_ptr<TTarget1, NullType1>& lhs,
      const weak_intrusive_ptr<TTarget2, NullType2>& rhs) noexcept;

  template <class TTarget1, class NullType1, class TTarget2, class NullType2>
  friend bool operator==(
      const weak_intrusive_ptr<TTarget1, NullType1>& lhs,
      const weak_intrusive_ptr<TTarget2, NullType2>& rhs) noexcept;
};

template <class TTarget1, class NullType1, class TTarget2, class NullType2>
inline bool operator<(
    const weak_intrusive_ptr<TTarget1, NullType1>& lhs,
    const weak_intrusive_ptr<TTarget2, NullType2>& rhs) noexcept {
  return lhs.target_ < rhs.target_;
}

template <class TTarget1, class NullType1, class TTarget2, class NullType2>
inline bool operator==(
    const weak_intrusive_ptr<TTarget1, NullType1>& lhs,
    const weak_intrusive_ptr<TTarget2, NullType2>& rhs) noexcept {
  return lhs.target_ == rhs.target_;
}

template <class TTarget1, class NullType1, class TTarget2, class NullType2>
inline bool operator!=(
    const weak_intrusive_ptr<TTarget1, NullType1>& lhs,
    const weak_intrusive_ptr<TTarget2, NullType2>& rhs) noexcept {
  return !operator==(lhs, rhs);
}

template <class TTarget1, class NullType1>
inline void swap(
    weak_intrusive_ptr<TTarget1, NullType1>& lhs,
    weak_intrusive_ptr<TTarget1, NullType1>& rhs) noexcept {
  lhs.swap(rhs);
}

} // namespace c10

namespace std {
template <class TTarget, class NullType>
struct hash<c10::intrusive_ptr<TTarget, NullType>> {
  size_t operator()(const c10::intrusive_ptr<TTarget, NullType>& ptr) const {
    return std::hash<TTarget*>()(ptr.get());
  }
};

template <class TTarget, class NullType>
struct hash<c10::weak_intrusive_ptr<TTarget, NullType>> {
  size_t operator()(
      const c10::weak_intrusive_ptr<TTarget, NullType>& ptr) const {
    return std::hash<TTarget*>()(ptr.unsafe_get_target());
  }
};
} // namespace std
