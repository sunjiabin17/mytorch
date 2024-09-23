#pragma once

#define LIKELY(expr) (__builtin_expect(static_cast<bool>(expr), 1))
#define UNLIKELY(expr) (__builtin_expect(static_cast<bool>(expr), 0))

#define C10_VISIBILE __attribute__((visibility("default")))
#define C10_HIDDEN __attribute__((visibility("hidden")))

#define C10_API C10_VISIBILE

#define TORCH_API C10_VISIBILE

#define C10_ALWAYS_INLINE __attribute__((__always_inline__)) inline

#define C10_ANONYMOUS_VARIABLE(str) str##__COUNTER__
