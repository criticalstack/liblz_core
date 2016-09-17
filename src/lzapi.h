#ifndef __LZ_LZAPI_H__
#define __LZ_LZAPI_H__

#include <liblz.h>
#include <sys/tree.h>

#if defined __GNUC__ || defined __llvm__
#define lz_likely(x)   __builtin_expect(!!(x), 1)
#define lz_unlikely(x) __builtin_expect(!!(x), 0)
#else
#define lz_likely(x)   (x)
#define lz_unlikely(x) (x)
#endif


#ifndef TAILQ_FOREACH_SAFE
#define TAILQ_FOREACH_SAFE(var, head, field, tvar)        \
    for ((var) = TAILQ_FIRST((head));                     \
         (var) && ((tvar) = TAILQ_NEXT((var), field), 1); \
         (var) = (tvar))
#endif

#ifndef SLIST_FOREACH_SAFE
#define SLIST_FOREACH_SAFE(var, head, field, tvar)        \
    for ((var) = SLIST_FIRST((head));                     \
         (var) && ((tvar) = SLIST_NEXT((var), field), 1); \
         (var) = (tvar))
#endif

#ifndef SLIST_REMOVE
#define SLIST_REMOVE(head, elm, type, field) do {             \
        if (SLIST_FIRST((head)) == (elm))                     \
        {                                                     \
            SLIST_REMOVE_HEAD((head), field);                 \
        }else {                                               \
            struct type * curelm = SLIST_FIRST((head));       \
            while (SLIST_NEXT(curelm, field) != (elm))        \
            {                                                 \
                curelm = SLIST_NEXT(curelm, field);           \
            }                                                 \
            SLIST_NEXT(curelm, field) =                       \
                SLIST_NEXT(SLIST_NEXT(curelm, field), field); \
        }                                                     \
} while (0)
#endif

#define lz_safe_free(_var, _freefn)          do { \
        _freefn((_var));                          \
        (_var) = NULL;                            \
}  while (0)


#define lz_assert(x)                                                  \
    do {                                                              \
        if (lz_unlikely(!(x)))                                        \
        {                                                             \
            fprintf(stderr, "Assertion failed: %s (%s:%s:%d)\n", # x, \
                    __func__, __FILE__, __LINE__);                    \
            fflush(stderr);                                           \
            abort();                                                  \
        }                                                             \
    } while (0)

#define lz_alloc_assert(x)                                \
    do {                                                  \
        if (lz_unlikely(!x))                              \
        {                                                 \
            fprintf(stderr, "Out of memory (%s:%s:%d)\n", \
                    __func__, __FILE__, __LINE__);        \
            fflush(stderr);                               \
            abort();                                      \
        }                                                 \
    } while (0)

#define lz_assert_fmt(x, fmt, ...)                                       \
    do {                                                                 \
        if (lz_unlikely(!(x)))                                           \
        {                                                                \
            fprintf(stderr, "Assertion failed: %s (%s:%s:%d) " fmt "\n", \
                    # x, __func__, __FILE__, __LINE__, __VA_ARGS__);     \
            fflush(stderr);                                              \
            abort();                                                     \
        }                                                                \
    } while (0)

#define lz_errno_assert(x)                          \
    do {                                            \
        if (lz_unlikely(!(x)))                      \
        {                                           \
            fprintf(stderr, "%s [%d] (%s:%s:%d)\n", \
                    strerror(errno), errno,         \
                    __func__, __FILE__, __LINE__);  \
            fflush(stderr);                         \
            abort();                                \
        }                                           \
    } while (0)

#define lz_str3_cmp(m, c0, c1, c2, c3) \
    *(uint32_t *)m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

#define lz_str6_cmp(m, c0, c1, c2, c3, c4, c5)                   \
    *(uint32_t *)m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0) \
    && (((uint32_t *)m)[1] & 0xffff) == ((c5 << 8) | c4)

#define lz_str30_cmp(m, c0, c1, c2, c3) \
    *(uint32_t *)m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

static inline int
lz_atoi(const char * line, size_t n)
{
    int value;

    if (lz_unlikely(n == 0))
    {
        return 0;
    }

    for (value = 0; n--; line++)
    {
        if (lz_unlikely(*line < '0' || *line > '9'))
        {
            return -1;
        }

        value = value * 10 + (*line - '0');
    }

    if (value < 0)
    {
        return -1;
    } else {
        return value;
    }
}


#define lz_isascii(c) \
    ((unsigned char)(c) - 040u < 0137u)

#define lz_alias(_name, _aliasname) \
    __typeof(_name) _aliasname __attribute__((alias(# _name)))

#endif
