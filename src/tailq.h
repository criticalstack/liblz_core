#pragma once

struct lz_tailq_elem;
struct lz_tailq;

typedef struct lz_tailq_elem lz_tailq_elem;
typedef struct lz_tailq      lz_tailq;

typedef void (*lz_tailq_freefn)(void *);
typedef int (*lz_tailq_iterfn)(lz_tailq_elem * elem, void * arg);

LZ_EXPORT lz_tailq * lz_tailq_new(void);
LZ_EXPORT void       lz_tailq_free(lz_tailq * tq);
LZ_EXPORT size_t     lz_tailq_size(lz_tailq * head);
LZ_EXPORT int        lz_tailq_foreach(lz_tailq *, lz_tailq_iterfn, void *);
LZ_EXPORT void     * lz_tailq_get_at_index(lz_tailq * tq, int idx);

LZ_EXPORT lz_tailq_elem * lz_tailq_append(lz_tailq * head,
    void                                           * data,
    size_t                                           len,
    lz_tailq_freefn                                  freefn);

LZ_EXPORT lz_tailq_elem * lz_tailq_prepend(lz_tailq * head,
    void                                            * data,
    size_t                                            len,
    lz_tailq_freefn                                   freefn);

LZ_EXPORT lz_tailq_elem * lz_tailq_first(lz_tailq * head);
LZ_EXPORT lz_tailq_elem * lz_tailq_last(lz_tailq * head);
LZ_EXPORT lz_tailq_elem * lz_tailq_next(lz_tailq_elem * elem);
LZ_EXPORT lz_tailq_elem * lz_tailq_prev(lz_tailq_elem * elem);
LZ_EXPORT lz_tailq      * lz_tailq_elem_head(lz_tailq_elem * elem);
LZ_EXPORT void          * lz_tailq_elem_data(lz_tailq_elem *);
LZ_EXPORT int             lz_tailq_elem_remove(lz_tailq_elem * elem);

/* backwards compat */
#define lz_tailq_for_each lz_tailq_foreach
