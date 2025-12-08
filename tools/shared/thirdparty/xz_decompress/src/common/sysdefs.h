#ifndef LZMA_SYSDEFS_H
#define LZMA_SYSDEFS_H

#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

typedef uint8_t  uint8_t;
typedef uint16_t uint16_t;
typedef uint32_t uint32_t;
typedef uint64_t uint64_t;

typedef int8_t   int8_t;
typedef int16_t  int16_t;
typedef int32_t  int32_t;
typedef int64_t  int64_t;

#ifndef __cplusplus
    typedef int bool;
#endif
#define true 1
#define false 0

////////////
// Macros //
////////////

#undef memzero
#define memzero(s, n) memset(s, 0, n)

// NOTE: Avoid using MIN() and MAX(), because even conditionally defining
// those macros can cause some portability trouble, since on some systems
// the system headers insist defining their own versions.
#define my_min(x, y) ((x) < (y) ? (x) : (y))
#define my_max(x, y) ((x) > (y) ? (x) : (y))

#ifndef ARRAY_SIZE
#   define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

#if defined(__GNUC__) \
        && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 3) || __GNUC__ > 4)
#   define lzma_attr_alloc_size(x) __attribute__((__alloc_size__(x)))
#else
#   define lzma_attr_alloc_size(x)
#endif

#if __STDC_VERSION__ >= 202311
#   define FALLTHROUGH [[__fallthrough__]]
#elif (defined(__GNUC__) && __GNUC__ >= 7) \
        || (defined(__clang_major__) && __clang_major__ >= 10)
#   define FALLTHROUGH __attribute__((__fallthrough__))
#else
#   define FALLTHROUGH ((void)0)
#endif

#endif
