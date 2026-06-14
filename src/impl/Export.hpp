#ifndef MYNUMBER_IMPL_EXPORT_HPP_
#define MYNUMBER_IMPL_EXPORT_HPP_

// Fallback for static-only builds (Emscripten, node-gyp) that do not run CMake.
// CMake replaces this with a generated header when building native libraries.
#ifndef EXPORT_API
#define EXPORT_API
#endif

#endif
