#pragma once

#include <cassert>

#define LIKELY(expr) (__builtin_expect(static_cast<bool>(expr), 1))
#define UNLIKELY(expr) (__builtin_expect(static_cast<bool>(expr), 0))


#define C10_EXPORT __attribute__((visibility("default")))
#define C10_IMPORT __attribute__((visibility("default")))
#define C10_HIDDEN __attribute__((visibility("hidden")))


#define C10_API C10_EXPORT
