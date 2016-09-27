#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>

#include <liblz.h>
#include <liblz/lzapi.h>
#include <liblz/core/lz_heap.h>

#include "tailq.h"

static __thread void * __elem_heap = NULL;

struct lz_tailq_elem {
    lz_tailq      * tq_head;
    void          * data;
    size_t          len;
    lz_tailq_freefn free_fn;

    TAILQ_ENTRY(lz_tailq_elem) next;
};

TAILQ_HEAD(__lz_tailqhd, lz_tailq_elem);

struct lz_tailq {
    size_t              n_elem;
    struct __lz_tailqhd elems;
};

static lz_tailq_elem * tq_first_(lz_tailq * tq);
static lz_tailq_elem * tq_next_(lz_tailq_elem * elem);
static void          * tq_elem_data_(lz_tailq_elem * elem);
static void            tq_elem_free_(lz_tailq_elem * elem);
static int             tq_elem_remove_(lz_tailq_elem * elem);
static int             tq_foreach_(lz_tailq *, lz_tailq_iterfn, void *);
static lz_tailq_elem * tq_prepend_elem(lz_tailq *, lz_tailq_elem *);
static lz_tailq_elem * tq_append_(lz_tailq *, void *, size_t, lz_tailq_freefn);
static lz_tailq_elem * tq_prepend_elem(lz_tailq * tq, lz_tailq_elem * elem);
static lz_tailq_elem * tq_elem_new_(void *, size_t, lz_tailq_freefn);

static size_t tq_elem_size_(lz_tailq_elem * elem);


static void
tq_freefn_(void * data)
{
    if (data == NULL)
    {
        return;
    }

    lz_safe_free(data, free);
}

static lz_tailq *
tq_new_(void)
{
    lz_tailq * tq;

    tq = malloc(sizeof(lz_tailq));

    if (lz_unlikely((tq = malloc(sizeof(lz_tailq)))))
    {
        return NULL;
    }

    TAILQ_INIT(&tq->elems);

    tq->n_elem = 0;

    return tq;
}

static void
tq_free_(lz_tailq * tq)
{
    lz_tailq_elem * elem;
    lz_tailq_elem * temp;

    if (lz_unlikely(tq == NULL))
    {
        return;
    }

    for (elem = tq_first_(tq); elem != NULL; elem = temp)
    {
        temp = tq_next_(elem);

        tq_elem_remove_(elem);
        lz_safe_free(elem, tq_elem_free_);
    }

    lz_safe_free(tq, free);
}

static void
tq_clear_(lz_tailq * tq)
{
    lz_tailq_elem * elem;
    lz_tailq_elem * temp;

    if (lz_unlikely(tq == NULL))
    {
        return;
    }

    for (elem = tq_first_(tq); elem != NULL; elem = temp)
    {
        temp = tq_next_(elem);

        tq_elem_remove_(elem);
        tq_elem_free_(elem);
    }

    tq->n_elem = 0;

    TAILQ_INIT(&tq->elems);
}

static int
tq_dup_itercb_(lz_tailq_elem * elem, void * arg)
{
    lz_tailq * tq = arg;
    size_t     len;
    void     * data;

    len  = tq_elem_size_(elem);
    data = tq_elem_data_(elem);

    if (len)
    {
        if (!(data = malloc(len)))
        {
            return -1;
        }

        memcpy(data, tq_elem_data_(elem), len);
    }

    if (!tq_append_(tq, data, len, elem->free_fn))
    {
        return -1;
    }

    return 0;
}

static lz_tailq *
tq_dup_(lz_tailq * tq)
{
    lz_tailq * new_tq;

    if (tq == NULL)
    {
        return NULL;
    }

    if (!(new_tq = tq_new_()))
    {
        return NULL;
    }

    if (tq_foreach_(tq, tq_dup_itercb_, new_tq))
    {
        tq_free_(new_tq);
        return NULL;
    }

    return new_tq;
}

static lz_tailq_elem *
tq_elem_new_(void * data, size_t len, lz_tailq_freefn freefn)
{
    lz_tailq_elem * elem;

    if (lz_unlikely(__elem_heap == NULL))
    {
        __elem_heap = lz_heap_new(sizeof(lz_tailq_elem), 1024);
    }

    elem = lz_heap_alloc(__elem_heap); /* malloc(sizeof(lz_tailq_elem)); */

    if (lz_unlikely(elem == NULL))
    {
        return NULL;
    }

    elem->data    = data;
    elem->len     = len;
    elem->tq_head = NULL;

    if (lz_likely(freefn != NULL))
    {
        elem->free_fn = freefn;
    } else {
        elem->free_fn = tq_freefn_;
    }

    return elem;
}

static void
tq_elem_free_(lz_tailq_elem * elem)
{
    if (lz_unlikely(elem == NULL))
    {
        return;
    }

    if (lz_likely(elem->data != NULL))
    {
        lz_safe_free(elem->data, elem->free_fn);
    }

    lz_heap_free(__elem_heap, elem);
    /* free(elem); */
}

static lz_tailq_elem *
tq_append_elem_(lz_tailq * tq, lz_tailq_elem * elem)
{
    if (lz_unlikely(!tq || !elem))
    {
        return NULL;
    }

    TAILQ_INSERT_TAIL(&tq->elems, elem, next);

    tq->n_elem   += 1;
    elem->tq_head = tq;

    return elem;
}

static lz_tailq_elem *
tq_append_(lz_tailq * tq, void * data, size_t len, lz_tailq_freefn freefn)
{
    lz_tailq_elem * elem;

    if (lz_unlikely(tq == NULL))
    {
        return NULL;
    }

    if (!(elem = tq_elem_new_(data, len, freefn)))
    {
        return NULL;
    }

    return tq_append_elem_(tq, elem);
}

static lz_tailq_elem *
tq_prepend_elem_(lz_tailq * tq, lz_tailq_elem * elem)
{
    if (lz_unlikely(!tq || !elem))
    {
        return NULL;
    }

    TAILQ_INSERT_HEAD(&tq->elems, elem, next);

    tq->n_elem   += 1;
    elem->tq_head = tq;

    return elem;
}

static lz_tailq_elem *
tq_prepend_(lz_tailq * tq, void * data, size_t len, lz_tailq_freefn freefn)
{
    lz_tailq_elem * elem;

    if (lz_unlikely(tq == NULL))
    {
        return NULL;
    }

    if (!(elem = tq_elem_new_(data, len, freefn)))
    {
        return NULL;
    }

    return tq_prepend_elem_(tq, elem);
}

static lz_tailq_elem *
tq_first_(lz_tailq * tq)
{
    return tq ? TAILQ_FIRST(&tq->elems) : NULL;
}

static lz_tailq_elem *
tq_last_(lz_tailq * tq)
{
    return tq ? TAILQ_LAST(&tq->elems, __lz_tailqhd) : NULL;
}

static lz_tailq_elem *
tq_next_(lz_tailq_elem * elem)
{
    return elem ? TAILQ_NEXT(elem, next) : NULL;
}

static lz_tailq_elem *
tq_prev_(lz_tailq_elem * elem)
{
    return elem ? TAILQ_PREV(elem, __lz_tailqhd, next) : NULL;
}

static int
tq_elem_remove_(lz_tailq_elem * elem)
{
    lz_tailq * head;

    if (lz_unlikely(!elem))
    {
        return -1;
    }

    if (!(head = elem->tq_head))
    {
        return -1;
    }

    TAILQ_REMOVE(&head->elems, elem, next);

    head->n_elem -= 1;

    return 0;
}

static int
tq_foreach_(lz_tailq * tq, lz_tailq_iterfn iterfn, void * arg)
{
    lz_tailq_elem * elem;
    lz_tailq_elem * temp;
    int             sres;

    if (lz_unlikely(!tq || !iterfn))
    {
        return -1;
    }

    TAILQ_FOREACH_SAFE(elem, &tq->elems, next, temp)
    {
        if ((sres = (iterfn)(elem, arg)))
        {
            return sres;
        }
    }

    return 0;
}

static void *
tq_elem_data_(lz_tailq_elem * elem)
{
    return elem ? elem->data : NULL;
}

static size_t
tq_elem_size_(lz_tailq_elem * elem)
{
    return elem ? elem->len : 0;
}

static lz_tailq *
tq_elem_head_(lz_tailq_elem * elem)
{
    return elem ? elem->tq_head : NULL;
}

/* NOTE: we should think about changing this to ssize_t */
static size_t
tq_size_(lz_tailq * head)
{
    return head ? head->n_elem : 0;
}

static lz_tailq_elem *
tq_elem_find_(lz_tailq * tq, void * data)
{
    lz_tailq_elem * elem;
    lz_tailq_elem * temp;

    /* this is not very efficient for large lists, but hey, whatever
     * floats your boat. I sometimes use it because I'm an idiot.
     */

    for (elem = tq_first_(tq); elem != NULL; elem = temp)
    {
        temp = tq_next_(elem);
    }

    if (tq_elem_data_(elem) == data)
    {
        return elem;
    }

    return NULL;
}

static void *
tq_get_at_index_(lz_tailq * tq, int index)
{
    lz_tailq_elem * elem;
    lz_tailq_elem * temp;
    int             i;

    i = 0;

    for (elem = tq_first_(tq); elem; elem = temp)
    {
        temp = tq_next_(elem);

        if (i == index)
        {
            return tq_elem_data_(elem);
        }

        i++;
    }

    return NULL;
}

