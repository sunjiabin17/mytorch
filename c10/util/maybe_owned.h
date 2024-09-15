#pragma once

#include <c10/util/macros.h>
#include <c10/util/exception.h>

#include <utility>
#include <type_traits>
#include <memory>

namespace c10 {

template <typename T>
struct MaybeOwnedTraitsImpl {
  using owned_type = T;
  using borrow_type = const T*;

  static borrow_type createBorrow(const owned_type& from) {
    return &from;
  }

  static void assignBorrow(borrow_type& lhs, borrow_type rhs) {
    lhs = rhs;
  }

  static void destroyBorrow(borrow_type& /*toDestory*/) {}

  static const owned_type& referenceFromBorrow(const borrow_type& borrow) {
    return *borrow;
  }

  static const owned_type* pointerFromBorrow(const borrow_type& borrow) {
    return borrow;
  }

    static bool debugBorrowIsValid(const borrow_type& borrow) {
      return borrow != nullptr;
  }

};

template <typename T>
struct MaybeOwnedTraits;

template <typename T>
struct MaybeOwnedTraits<std::shared_ptr<T>>
    : public MaybeOwnedTraitsImpl<std::shared_ptr<T>> {};

template <typename T>
class MaybeOwned final {
  using borrow_type = typename MaybeOwnedTraits<T>::borrow_type;
  using owned_type = typename MaybeOwnedTraits<T>::owned_type;

  bool isBorrowed_;

  union {
    owned_type own_;
    borrow_type borrow_;
  };

  explicit MaybeOwned(const owned_type& t)
      : isBorrowed_(true), borrow_(MaybeOwnedTraits<T>::createBorrow(t)) {}

  explicit MaybeOwned(T&& t) noexcept(std::is_nothrow_move_constructible_v<T>)
      : isBorrowed_(false), own_(std::move(t)) {}

  template <typename... Args>
  explicit MaybeOwned(std::in_place_t, Args&&... args)
      : isBorrowed_(false), own_(std::forward<Args>(args)...) {}

public:
  explicit MaybeOwned() : isBorrowed_(true), borrow_() {}

  // copy constructor
  MaybeOwned(const MaybeOwned& rhs) : isBorrowed_(rhs.isBorrowed_) {
    if (LIKELY(rhs.isBorrowed_)) {
      MaybeOwnedTraits<T>::assignBorrow(borrow_, rhs.borrow_);
    } else {
      new (&own_) T(rhs.own_);
    }
  }

  // copy assignment
  MaybeOwned& operator=(const MaybeOwned& rhs) {
    if (this == &rhs) {
      return *this;
    }
    if (UNLIKELY(!isBorrowed_)) { // if this is owned
      if (rhs.isBorrowed_) {      // if rhs is borrowed
        own_.~T();                // destroy the owned object
        MaybeOwnedTraits<T>::assignBorrow(borrow_, rhs.borrow_);
        isBorrowed_ = true;
      } else {                    // if rhs is owned
        own_ = rhs.own_;          // copy the owned object to this
      }
    } else {                          // if this is borrowed
      if (LIKELY(rhs.isBorrowed_)) {  // if rhs is borrowed
        MaybeOwnedTraits<T>::assignBorrow(borrow_, rhs.borrow_);
      } else {                        // if rhs is owned
        MaybeOwnedTraits<T>::destroyBorrow(borrow_);
        new (&own_) T(rhs.own_);
        isBorrowed_ = false;
      }
    }
    TORCH_CHECK(isBorrowed_ == rhs.isBorrowed_);
    return *this;
  }

  // move constructor
  MaybeOwned(MaybeOwned&& rhs) noexcept(
      std::is_nothrow_move_constructible_v<T> and
      std::is_nothrow_move_assignable_v<borrow_type>)
      : isBorrowed_(rhs.isBorrowed_) {
    if (LIKELY(rhs.isBorrowed_)) {
      MaybeOwnedTraits<T>::assignBorrow(borrow_, rhs.borrow_);
    } else {
      new (&own_) T(std::move(rhs.own_));
    }
  }

  // move assignment
  MaybeOwned& operator=(MaybeOwned&& rhs) noexcept(
      std::is_nothrow_move_assignable_v<T> and
      std::is_nothrow_move_assignable_v<borrow_type> and
      std::is_nothrow_move_constructible_v<T> and
      std::is_nothrow_destructible_v<T> and
      std::is_nothrow_destructible_v<borrow_type>) {
    if (this == &rhs) {
      return *this;
    }
    if (UNLIKELY(!isBorrowed_)) {       // if this is owned
      if (rhs.isBorrowed_) {            // if rhs is borrowed
        own_.~T();                      // destroy the owned object
        MaybeOwnedTraits<T>::assignBorrow(borrow_, rhs.borrow_);
        isBorrowed_ = true;
      } else {                          // if rhs is owned
        own_ = std::move(rhs.own_);     // move the owned object to this
      }
    } else {                            // if this is borrowed
      if(LIKELY(rhs.isBorrowed_)) {     // if rhs is borrowed
        MaybeOwnedTraits<T>::assignBorrow(borrow_, rhs.borrow_);
      } else {                          // if rhs is owned
        MaybeOwnedTraits<T>::destroyBorrow(borrow_);
        new (&own_) T(std::move(rhs.own_));
        isBorrowed_ = false;
      }
    }
    return *this;
  }

  static MaybeOwned borrowed(const T& t) {
    return MaybeOwned(t);
  }

  static MaybeOwned owned(T&& t) noexcept(
      std::is_nothrow_move_assignable_v<T>) {
    return MaybeOwned(std::move(t));
  }

  template <class... Args>
  static MaybeOwned owned(std::in_place_t, Args&&... args) {
    return MaybeOwned(std::in_place, std::forward<Args>(args)...);
  }

  ~MaybeOwned() noexcept(
      std::is_nothrow_destructible_v<T> and
      std::is_nothrow_destructible_v<borrow_type>) {
    if (UNLIKELY(!isBorrowed_)) {
      own_.~T();
    } else {
      MaybeOwnedTraits<T>::destroyBorrow(borrow_);
    }
  }

  bool unsafeIsBorrowed() const {
    return isBorrowed_;
  }

  const T& operator*() const & {
    return LIKELY(isBorrowed_)
        ? MaybeOwnedTraits<T>::referenceFromBorrow(borrow_)
        : own_;
  }

  const T* operator->() const & {
    return LIKELY(isBorrowed_)
        ? MaybeOwnedTraits<T>::pointerFromBorrow(borrow_)
        : &own_;
  }

  T operator*() && {
    if (isBorrowed_) {
      return MaybeOwnedTraits<T>::referenceFromBorrow(borrow_);
    } else {
      return std::move(own_);
    }
  }

};


} // namespace c10
