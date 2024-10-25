#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>

#include <c10/util/ArrayRef.h>
#include <c10/util/Macros.h>

#define C10_SIZES_AND_STRIDES_MAX_SIZE 10

namespace c10::impl {

class C10_API SizesAndStrides {
 public:
  using sizes_iterator = int64_t*;
  using sizes_const_iterator = const int64_t*;
  using strides_iterator = int64_t*;
  using strides_const_iterator = const int64_t*;

  SizesAndStrides() {
    size_at_unchecked(0) = 0;
    stride_at_unchecked(0) = 1;
  }

  ~SizesAndStrides() = default;

  SizesAndStrides(const SizesAndStrides& rhs) : size_(rhs.size_) {
    copy_data(rhs);
  }

  SizesAndStrides& operator=(const SizesAndStrides& rhs) {
    if (this != &rhs) {
      size_ = rhs.size_;
      copy_data(rhs);
    }
    return *this;
  }

  SizesAndStrides(SizesAndStrides&& rhs) noexcept : size_(rhs.size_) {
    copy_data(rhs);
    rhs.size_ = 0;
  }

  SizesAndStrides& operator=(SizesAndStrides&& rhs) noexcept {
    if (this != &rhs) {
      size_ = rhs.size_;
      copy_data(rhs);
      rhs.size_ = 0;
    }
    return *this;
  }

  size_t size() const noexcept {
    return size_;
  }

  const int64_t* sizes_data() const noexcept {
    return &storage_[0];
  }

  int64_t* sizes_data() noexcept {
    return &storage_[0];
  }

  sizes_const_iterator sizes_begin() const noexcept {
    return sizes_data();
  }

  sizes_iterator sizes_begin() noexcept {
    return sizes_data();
  }

  sizes_const_iterator sizes_end() const noexcept {
    return sizes_data() + size();
  }

  sizes_iterator sizes_end() noexcept {
    return sizes_data() + size();
  }

  IntArrayRef sizes_arrayref() const noexcept {
    return IntArrayRef{sizes_data(), size()};
  }

  void set_sizes(IntArrayRef new_sizes) {
    resize(new_sizes.size());
    std::copy(new_sizes.begin(), new_sizes.end(), sizes_begin());
  }

  const int64_t* strides_data() const noexcept {
    return &storage_[C10_SIZES_AND_STRIDES_MAX_SIZE];
  }

  int64_t* strides_data() noexcept {
    return &storage_[C10_SIZES_AND_STRIDES_MAX_SIZE];
  }

  strides_const_iterator strides_begin() const noexcept {
    return strides_data();
  }

  strides_iterator strides_begin() noexcept {
    return strides_data();
  }

  strides_const_iterator strides_end() const noexcept {
    return strides_data() + size();
  }

  strides_iterator strides_end() noexcept {
    return strides_data() + size();
  }

  IntArrayRef strides_arrayref() const noexcept {
    return IntArrayRef{strides_data(), size()};
  }

  void set_strides(IntArrayRef new_strides) {
    TORCH_INTERNAL_ASSERT(new_strides.size() == size());
    std::copy(new_strides.begin(), new_strides.end(), strides_begin());
  }

  int64_t size_at(size_t idx) const noexcept {
    assert(idx < size());
    return sizes_data()[idx];
  }

  int64_t& size_at(size_t idx) noexcept {
    assert(idx < size());
    return sizes_data()[idx];
  }

  int64_t size_at_unchecked(size_t idx) const noexcept {
    return sizes_data()[idx];
  }

  int64_t& size_at_unchecked(size_t idx) noexcept {
    return sizes_data()[idx];
  }

  int64_t stride_at(size_t idx) const noexcept {
    assert(idx < size());
    return strides_data()[idx];
  }

  int64_t& stride_at(size_t idx) noexcept {
    assert(idx < size());
    return strides_data()[idx];
  }

  int64_t stride_at_unchecked(size_t idx) const noexcept {
    return strides_data()[idx];
  }

  int64_t& stride_at_unchecked(size_t idx) noexcept {
    return strides_data()[idx];
  }

  void resize(size_t new_size) {
    const auto old_size = size();
    if (new_size == old_size) {
      return;
    }
    assert(new_size <= C10_SIZES_AND_STRIDES_MAX_SIZE);
    if (old_size < new_size) {
      const auto bytes_to_zero = (new_size - old_size) * sizeof(storage_[0]);
      memset(&storage_[old_size], 0, bytes_to_zero);
      memset(
          &storage_[old_size + C10_SIZES_AND_STRIDES_MAX_SIZE],
          0,
          bytes_to_zero);
    }
    size_ = new_size;
  }

 private:
  void copy_data(const SizesAndStrides& rhs) {
    memcpy(storage_, rhs.storage_, sizeof(storage_));
  }

  size_t size_{1};

  // NOLINTNEXTLINE(*c-arrays*)
  int64_t storage_[C10_SIZES_AND_STRIDES_MAX_SIZE * 2]{};
};

} // namespace c10::impl
