#ifndef MYNUMBER_EXPORT_HPP
#define MYNUMBER_EXPORT_HPP

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(MYNUMBER_BUILT_AS_STATIC)
#    define EXPORT_API
#  elif defined(MYNUMBER_EXPORTS)
#    define EXPORT_API __declspec(dllexport)
#  else
#    define EXPORT_API __declspec(dllimport)
#  endif
#else
#  if __GNUC__ >= 4
#    define EXPORT_API __attribute__((visibility("default")))
#  else
#    define EXPORT_API
#  endif
#endif

#endif // MYNUMBER_EXPORT_HPP
