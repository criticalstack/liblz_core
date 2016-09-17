#ifndef __LIBLZ_H__
#define __LIBLZ_H__

#if (defined __GNUC__ && __GNUC__ >= 4) || defined __INTEL_COMPILER || defined __clang__
#define LZ_EXPORT __attribute__ ((visibility("default")))
#else
#define LZ_EXPORT
#endif

#endif
