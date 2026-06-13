# mynumber — native CMake source package

Maintainer reference for `scripts/package-source.mjs`. This README is **not** included in the release zip.

The staged archive contains only files required to build the native CPack release (`.tar.gz` / `.zip`):

- `CMakeLists.txt`, `cmake/`, `packaging/project.json`
- `include/mynumber.hpp`, `include/impl/`, `include/polyfill/`
- `src/impl/`, `src/main.cpp`

Node and WASM release binaries are built from the full monorepo (`make dist-node`, `make dist-wasm`).

## Build (from extracted zip)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --build build --target package   # CPack archives
sudo cmake --install build             # optional system install
```

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
