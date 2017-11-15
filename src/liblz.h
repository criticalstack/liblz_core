#pragma once

#if (defined __GNUC__ && __GNUC__ >= 4) || defined __INTEL_COMPILER || defined __clang__
#define LZ_EXPORT __attribute__ ((visibility("default")))
#else
#define LZ_EXPORT
#endif

#include <liblz/core/lz_heap.h>
#include <liblz/core/lz_tailq.h>
#include <liblz/core/lz_kvmap.h>
#include <liblz/core/lz_file.h>
