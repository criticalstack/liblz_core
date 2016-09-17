#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>

#include "lzapi.h"

#include "heap/lz_heap.h"
#include "lz_tailq.h"

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


static void
_lz_tailq_freefn(void * data) {
    if (data) {
        free(data);
    }
}

lz_tailq *
lz_tailq_new(void) {
    lz_tailq * tq;

    tq = malloc(sizeof(lz_tailq));

    if (lz_unlikely(tq == NULL)) {
        return NULL;
    }

    TAILQ_INIT(&tq->elems);

    tq->n_elem = 0;

    return tq;
}

void
lz_tailq_free(lz_tailq * tq) {
    lz_tailq_elem * elem;
    lz_tailq_elem * temp;

    if (lz_unlikely(tq == NULL)) {
        return;
    }

    for (elem = lz_tailq_first(tq); elem != NULL; elem = temp) {
        temp = lz_tailq_next(elem);

        lz_tailq_elem_remove(elem);
        lz_tailq_elem_free(elem);
    }

    free(tq);
}

void
lz_tailq_clear(lz_tailq * tq) {
    lz_tailq_elem * elem;
    lz_tailq_elem * temp;

    if (!tq) {
        return;
    }

    for (elem = lz_tailq_first(tq); elem != NULL; elem = temp) {
        temp = lz_tailq_next(elem);

        lz_tailq_elem_remove(elem);
        lz_tailq_elem_free(elem);
    }

    TAILQ_INIT(&tq->elems);
    tq->n_elem = 0;
}

static int
lz_tailq_dup_itercb(lz_tailq_elem * elem, void * arg) {
    lz_tailq * tq = arg;
    size_t     len;
    void     * data;

    len  = lz_tailq_elem_size(elem);
    data = lz_tailq_elem_data(elem);

    if (len) {
        if (!(data = malloc(len))) {
            return -1;
        }

        memcpy(data, lz_tailq_elem_data(elem), len);
    }

    if (!lz_tailq_append(tq, data, len, elem->free_fn)) {
        return -1;
    }

    return 0;
}

lz_tailq *
lz_tailq_dup(lz_tailq * tq) {
    lz_tailq * new_tq;

    if (tq == NULL) {
        return NULL;
    }

    if (!(new_tq = lz_tailq_new())) {
        return NULL;
    }

    if (lz_tailq_for_each(tq, lz_tailq_dup_itercb, new_tq)) {
        lz_tailq_free(new_tq);
        return NULL;
    }

    return new_tq;
}

lz_tailq_elem *
lz_tailq_elem_new(void * data, size_t len, lz_tailq_freefn freefn) {
    lz_tailq_elem * elem;

    if (lz_unlikely(__elem_heap == NULL)) {
        __elem_heap = lz_heap_new(sizeof(lz_tailq_elem), 1024);
    }

    elem = lz_heap_alloc(__elem_heap); /* malloc(sizeof(lz_tailq_elem)); */

    if (lz_unlikely(elem == NULL)) {
        return NULL;
    }

    elem->data    = data;
    elem->len     = len;
    elem->tq_head = NULL;

    if (lz_likely(freefn != NULL)) {
        elem->free_fn = freefn;
    } else {
        elem->free_fn = _lz_tailq_freefn;
    }

    return elem;
}

void
lz_tailq_elem_free(lz_tailq_elem * elem) {
    if (lz_unlikely(elem == NULL)) {
        return;
    }

    if (lz_likely(elem->data != NULL)) {
        (elem->free_fn)(elem->data);
    }

    lz_heap_free(__elem_heap, elem);
    /* free(elem); */
}

lz_tailq_elem *
lz_tailq_append(lz_tailq * tq, void * data, size_t len, lz_tailq_freefn freefn) {
    lz_tailq_elem * elem;

    if (lz_unlikely(tq == NULL)) {
        return NULL;
    }

    elem = lz_tailq_elem_new(data, len, freefn);

    if (lz_unlikely(elem == NULL)) {
        return NULL;
    }

    return lz_tailq_append_elem(tq, elem);
}

lz_tailq_elem *
lz_tailq_append_elem(lz_tailq * tq, lz_tailq_elem * elem) {
    if (!tq || !elem) {
        return NULL;
    }

    TAILQ_INSERT_TAIL(&tq->elems, elem, next);

    tq->n_elem   += 1;
    elem->tq_head = tq;

    return elem;
}

lz_tailq_elem *
lz_tailq_prepend(lz_tailq * tq, void * data, size_t len, lz_tailq_freefn freefn) {
    lz_tailq_elem * elem;

    if (!tq) {
        return NULL;
    }

    if (!(elem = lz_tailq_elem_new(data, len, freefn))) {
        return NULL;
    }

    return lz_tailq_prepend_elem(tq, elem);
}

lz_tailq_elem *
lz_tailq_prepend_elem(lz_tailq * tq, lz_tailq_elem * elem) {
    if (!tq || !elem) {
        return NULL;
    }

    TAILQ_INSERT_HEAD(&tq->elems, elem, next);

    tq->n_elem   += 1;
    elem->tq_head = tq;

    return elem;
}

lz_tailq_elem *
lz_tailq_first(lz_tailq * tq) {
    if (lz_unlikely(tq == NULL)) {
        return NULL;
    } else {
        return TAILQ_FIRST(&tq->elems);
    }
}

lz_tailq_elem *
lz_tailq_last(lz_tailq * tq) {
    if (lz_unlikely(tq == NULL)) {
        return NULL;
    } else {
        return TAILQ_LAST(&tq->elems, __lz_tailqhd);
    }
}

lz_tailq_elem *
lz_tailq_next(lz_tailq_elem * elem) {
    if (!elem) {
        return NULL;
    }

    return TAILQ_NEXT(elem, next);
}

lz_tailq_elem *
lz_tailq_prev(lz_tailq_elem * elem) {
    if (!elem) {
        return NULL;
    }

    return TAILQ_PREV(elem, __lz_tailqhd, next);
}

int
lz_tailq_elem_remove(lz_tailq_elem * elem) {
    lz_tailq * head;

    if (!elem) {
        return -1;
    }

    if (!(head = elem->tq_head)) {
        return -1;
    }

    TAILQ_REMOVE(&head->elems, elem, next);

    head->n_elem -= 1;

    return 0;
}

int
lz_tailq_for_each(lz_tailq * tq, lz_tailq_iterfn iterfn, void * arg) {
    lz_tailq_elem * elem;
    lz_tailq_elem * temp;

    if (!tq || !iterfn) {
        return -1;
    }


    TAILQ_FOREACH_SAFE(elem, &tq->elems, next, temp) {
        int sres;

        if ((sres = (iterfn)(elem, arg)) != 0) {
            return sres;
        }
    }

    return 0;
}

void *
lz_tailq_elem_data(lz_tailq_elem * elem) {
    if (lz_likely(elem != NULL)) {
        return elem->data;
    } else {
        return NULL;
    }
}

size_t
lz_tailq_elem_size(lz_tailq_elem * elem) {
    if (lz_likely(elem != NULL)) {
        return elem->len;
    } else {
        return 0;
    }
}

lz_tailq *
lz_tailq_elem_head(lz_tailq_elem * elem) {
    if (lz_likely(elem != NULL)) {
        return elem->tq_head;
    } else {
        return NULL;
    }
}

size_t
lz_tailq_size(lz_tailq * head) {
    if (!head) {
        return 0;
    }

    return head->n_elem;
}

lz_tailq_elem *
lz_tailq_elem_find(lz_tailq * tq, void * data) {
    lz_tailq_elem * elem;
    lz_tailq_elem * temp;

    /* this is not very efficient for large lists, but hey, whatever
     * floats your boat. I sometimes use it because I'm an idiot.
     */

    for (elem = lz_tailq_first(tq); elem != NULL; elem = temp) {
        temp = lz_tailq_next(elem);

        if (lz_tailq_elem_data(elem) == data) {
            return elem;
        }
    }

    return NULL;
}

void *
lz_tailq_get_at_index(lz_tailq * tq, int index) {
    lz_tailq_elem * elem;
    lz_tailq_elem * temp;
    int             i;

    i = 0;

    for (elem = lz_tailq_first(tq); elem; elem = temp) {
        temp = lz_tailq_next(elem);

        if (i == index) {
            return lz_tailq_elem_data(elem);
        }

        i++;
    }

    return NULL;
}

