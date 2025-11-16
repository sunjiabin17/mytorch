#pragma once

#include <c10/core/Allocator.h>


namespace c10 {

C10_API c10::Allocator* GetCPUAllocator();
C10_API void SetCPUAllocator(c10::Allocator* alloc, uint8_t priority = 0);

} // namespace c10
