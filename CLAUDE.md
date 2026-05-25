# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

The project uses CMake with a pre-configured `build/` directory. To build from source:

```bash
# Configure (only needed once or after CMakeLists changes)
cmake -B build -S . -DDEBUG=ON

# Build everything
cmake --build build

# Build and run all tests
cmake --build build && ctest --test-dir build
```

To run a single test binary:

```bash
./build/test/<test_name>
# e.g. ./build/test/intrusive_ptr_test
```

Test binaries are named after their source file (e.g. `intrusive_ptr_test.cpp` → `intrusive_ptr_test`).

## Architecture

This is a from-scratch reimplementation of PyTorch's core tensor infrastructure, mirroring PyTorch's own `c10/` and `aten/` library structure.

### Layer overview

**`c10/`** — The low-level runtime library (compiled as `libc10.so`):
- `c10/util/` — Foundational utilities: `IntrusivePtr` (ref-counted smart pointers), `UniqueVoidPtr` (type-erased owning pointers), `Exception` (TORCH_CHECK/TORCH_INTERNAL_ASSERT macros), `ArrayRef`, `Half`, `MaybeOwned`
- `c10/core/` — Core tensor abstractions: `Allocator`/`DataPtr`, `StorageImpl`, `Storage`, `TensorImpl`, `Device`/`DeviceType`, `DispatchKey`/`DispatchKeySet`, `SizesAndStrides`
- `c10/cpu/` — CPU-specific allocator (`CPUAllocator`) backed by `c10/cpu/impl/alloc`
- `c10/core/impl/` — Device guard infrastructure (`DeviceGuardImplInterface`, `InlineDeviceGuard`, `VirtualGuardImpl`)

**`aten/`** — ATen tensor frontend (will eventually expose the user-facing `Tensor` type):
- `aten/src/ATen/core/TensorBase.h` — `at::TensorBase` holds an `intrusive_ptr<c10::TensorImpl>`
- `aten/src/ATen/core/TensorBody.h` — Full `at::Tensor` (WIP)

**`test/c10/`** — GoogleTest unit tests, one file per component.

### Key design patterns

- **Reference counting**: `StorageImpl` and `TensorImpl` both inherit `c10::intrusive_ptr_target`. Use `c10::intrusive_ptr<T>` / `c10::make_intrusive<T>()` — never raw `new`.
- **Allocator registry**: `SetAllocator(DeviceType, Allocator*)` / `GetAllocator(DeviceType)`. The `REGISTER_ALLOCATOR(t, f)` macro registers at static-init time. CPU allocator is auto-registered.
- **DataPtr**: Wraps a `UniqueVoidPtr` (data + typed deleter) plus a `Device`. `StorageImpl` owns a `DataPtr`; `Allocator::allocate()` returns one.
- **Dispatch keys**: `DispatchKey` (uint16) and `DispatchKeySet` (bitset) express which backends/functionalities a tensor participates in. Backend components (CPU/CUDA/Meta) compose with functionality keys (Dense, AutogradFunctionality) via macros in `DispatchKey.h`.
- **`C10_API` / `TORCH_API`**: Both expand to `__attribute__((visibility("default")))` — use on any symbol that must be exported from `libc10.so`.
