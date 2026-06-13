# mynumber — native CMake source package

Maintainer reference for `scripts/package-source.mjs`. This README is **not** included in the release archives.

CI publishes `mynumber-cmake-{version}.zip` and `.tar.gz` with the same contents.

The staged archive contains only files required to build the native CPack release (`.tar.gz`):

- `CMakeLists.txt`, `cmake/`, `packaging/project.json`
- `include/mynumber.hpp`, `include/impl/`, `include/polyfill/`
- `src/impl/`, `src/main.cpp`

Node and WASM release binaries are built from the full monorepo (`make dist-node`, `make dist-wasm`).

## Build (from extracted zip or tar.gz)

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
