# Lab 7, variant 3: Interpolation. Exception handling and testing

C++20 project: static library `InterpolationLib`, demo program, antipattern program and Google Test executable.

## Structure

| Path | Purpose |
|------|---------|
| `include/InterpolationExceptions.h` | exception hierarchy |
| `include/Interpolation.h`, `src/Interpolation.cpp` | algorithms (Lagrange, linear interpolation, error) |
| `include/InterpolationTask.h`, `src/InterpolationTask.cpp` | class `InterpolationTask` (set of nodes) |
| `include/ScopeLogger.h` | RAII class that prints constructor/destructor messages |
| `demo/main.cpp` | error scenarios, strong guarantee, stack unwinding |
| `antipattern/main.cpp` | `throw 42` and `throw std::string` |
| `tests/InterpolationTests.cpp` | Google Test unit tests |

## Class invariants (`InterpolationTask`)

1. `xs.size() == ys.size()`
2. `1 <= size() <= 1000`
3. all coordinates are finite (no NaN, no Inf)
4. abscissas are strictly increasing (nodes are sorted, no duplicates)

The invariants are checked in the constructor and in every modifying operation (`replaceNodes`, `addNode`).

## Exception hierarchy

```
std::exception
 +- std::runtime_error
 |   +- InterpolationException
 |       +- EmptyNodeSetException
 |       +- InvalidNodeException        (parameter, index, value)
 |       +- DuplicateAbscissaException  (x)
 |       +- NodeCountException          (count, min, max)
 +- std::invalid_argument   (different sizes of xs/ys, NaN query point)
 +- std::out_of_range       (point outside the node range, no extrapolation)
```

| Operation | Condition | Exception |
|-----------|-----------|-----------|
| constructor, `replaceNodes`, `addNode` | no nodes | `EmptyNodeSetException` |
| constructor, `replaceNodes`, `addNode` | duplicate abscissas | `DuplicateAbscissaException` |
| constructor, `replaceNodes`, `addNode` | NaN / Inf in nodes | `InvalidNodeException` |
| constructor, `replaceNodes`, `addNode` | more than 1000 nodes | `NodeCountException` |
| constructor, `replaceNodes` | `xs.size() != ys.size()` | `std::invalid_argument` |
| `interpolate` | point outside `[minX, maxX]` | `std::out_of_range` |
| `lagrange`, `interpolate` | NaN query point | `std::invalid_argument` |
| `linearInterpolation` | `x0 == x1` | `DuplicateAbscissaException` |

Exceptions are caught by `const&`, the most specific handler first.

## Exception safety guarantees

| Method | Guarantee |
|--------|-----------|
| constructor | strong (nothing is created on failure; data are validated in locals) |
| `replaceNodes` | **strong**: new data validated and sorted in local copies, then `swap` (noexcept) |
| `addNode` | **strong**: copy of the state, modification of the copy, validation, then `swap` |
| `size`, `xs`, `ys`, `minX`, `maxX` | no-throw (`noexcept`) |
| `lagrange`, `interpolate`, `error` | const, state never changes |

Destructors do not throw (nothing is allocated manually, all memory is owned by `std::vector`).

## Expected output of stack unwinding (written BEFORE running)

Call chain: `main -> level1 -> level2 -> level3`, each function has a local `ScopeLogger`.
`level3` throws `std::out_of_range`, `level2` catches it, prints a diagnostic and rethrows with `throw;`, `main` handles it.
The handler of `main` is outside the `try` block where `ScopeLogger("main")` lives, so the logger of `main` is destroyed before the handler runs.

```
[ctor] main
[ctor] level1
[ctor] level2
[ctor] level3
[dtor] level3
level2: caught "...", rethrowing
[dtor] level2
[dtor] level1
[dtor] main
main: std::out_of_range: ...
```

## Antipattern: throwing `int` and `std::string`

`antipattern/main.cpp` throws `42` and `std::string`. `catch (const std::exception&)` does not catch them, only `catch (...)` does, and it has no access to the type or the value.

Conclusion: standard and custom exceptions (derived from `std::exception`) have a common base, a `what()` message, a type for selecting the handler and can carry diagnostic data. Primitive types give none of this, are not caught by `std::exception` handlers and may leave the error unhandled, so they must not be thrown.

## Build and run (Windows, Visual Studio, vcpkg)

In "x64 Native Tools Command Prompt for VS":

```
set VCPKG_ROOT=C:\vcpkg
cmake --preset default
cmake --build build --config Release
build\Release\InterpolationDemo.exe
build\Release\AntipatternDemo.exe
build\Release\InterpolationTests.exe
ctest --test-dir build -C Release
```

`vcpkg.json` declares the `gtest` dependency, vcpkg installs it automatically during `cmake --preset default`.

## Actual output

```
[ctor] main
[ctor] level1
[ctor] level2
[ctor] level3
[dtor] level3
level2: caught "interpolation point is outside the node range", rethrowing
[dtor] level2
[dtor] level1
[dtor] main
main: std::out_of_range: interpolation point is outside the node range
```

The actual order matches the expected one. Destructors run in reverse order of construction during stack unwinding, so no resources are leaked.
