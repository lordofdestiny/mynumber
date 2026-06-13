#ifndef __EXPORT_HPP__
#define __EXPORT_HPP__

#if defined(MYNUMBER_SHARED_BUILD)
#define EXPORT_API __attribute__((visibility("default")))
#else
#define EXPORT_API
#endif

#endif
