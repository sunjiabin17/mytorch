#pragma once

#include <c10/core/Device.h>
#include <c10/core/DispatchKeySet.h>
#include <c10/core/TensorImpl.h>
#include <c10/util/IntrusivePtr.h>

namespace c10 {

constexpr int64_t default_rng_seed = 67280421310721;

struct GeneratorImpl : public c10::intrusive_ptr_target {
  GeneratorImpl(Device device, DispatchKeySet key_set);

  // Delete all copy and move assignment in favor of clone()
  // method
  GeneratorImpl(const GeneratorImpl& other) = delete;
  GeneratorImpl(GeneratorImpl&& other) = delete;
  GeneratorImpl& operator=(const GeneratorImpl& other) = delete;
  GeneratorImpl& operator=(GeneratorImpl&& other) = delete;

  ~GeneratorImpl() override = default;
  c10::intrusive_ptr<GeneratorImpl> clone() const;

  virtual void set_current_seed(uint64_t seed) = 0;
  virtual void set_offset(uint64_t offset) = 0;
  virtual uint64_t current_seed() const = 0;
  virtual uint64_t get_offset() const = 0;
  virtual uint64_t seed() = 0;
  virtual void set_state(const TensorImpl& new_state) = 0;
  virtual c10::intrusive_ptr<TensorImpl> get_state() const = 0;
  Device device() const;

  std::mutex mutex_;

  DispatchKeySet key_set() const {
    return key_set_;
  }

 protected:
  Device device_;
  DispatchKeySet key_set_;
  virtual GeneratorImpl* clone_impl() const = 0;
};

} // namespace c10
