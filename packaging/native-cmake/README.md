# mynumber — native CMake source package

Maintainer reference for `scripts/package-source.mjs`. This README is **not** included in the release archives.

CI publishes `mynumber-cmake-{version}.zip` and `.tar.gz` with the same contents. CPack produces matching `.zip` and `.tar.gz` native binaries per platform.

`include/impl/Export.hpp` in the source tree is a static-build fallback. CMake generates the real export header at configure time (`GenerateExportHeader`) and installs that copy. Windows shared builds produce `mynumber.dll` plus an import `mynumber.lib`; the `verify_native_artifacts` target checks platform file extensions after build.

The staged archive contains only files required to build the native CPack release (`.tar.gz`):

- `CMakeLists.txt`, `cmake/`, `packaging/project.json`
- `include/mynumber.hpp`, `include/impl/`, `include/polyfill/`
- `src/impl/`, `src/main.cpp`

Node and WASM release binaries are built from the full monorepo (`make dist-node`, `make dist-wasm`).

## Build (from extracted zip or tar.gz)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --build build --target verify_native_artifacts   # checks .dll/.dylib/.so + static lib
cmake --build build --target package   # CPack archives (.zip + .tar.gz)
sudo cmake --install build             # optional system install
```

On Windows with Visual Studio, omit `-DCMAKE_BUILD_TYPE` at configure time and pass `--config Release` to build/package commands.

## Link your application

**CMake** (after install)

```cmake
find_package(mynumber REQUIRED)
add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE mynumber::mynumber_shared)
```

**pkg-config** (after install)

```bash
g++ -std=c++20 -o myapp main.cpp $(pkg-config --cflags --libs mynumber)
```
