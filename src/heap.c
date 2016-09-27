#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <stddef.h>
#include <sys/queue.h>

#include <liblz.h>
#include <liblz/lzapi.h>

struct lz_heap_page_s;
typedef struct lz_heap_page_s lz_heap_page;

struct lz_heap_page_s {
    SLIST_ENTRY(lz_heap_page_s) next;
    char data[];
};

struct lz_heap_s {
    size_t page_size;             /* page size */

    SLIST_HEAD(, lz_heap_page_s) page_list_free;
    SLIST_HEAD(, lz_heap_page_s) page_list_used;
};


static lz_heap_page *
heap_page_new_(lz_heap * heap)
{
    lz_heap_page * page;

    if (!(page = malloc(heap->page_size + sizeof(lz_heap_page))))
    {
        return NULL;
    }

    SLIST_INSERT_HEAD(&heap->page_list_free, page, next);

    return page;
};


static lz_heap *
heap_new_(size_t elts, size_t size)
{
    lz_heap * heap;

    if (!(heap = malloc(sizeof(lz_heap))))
    {
        return NULL;
    }

    heap->page_size = size;

    SLIST_INIT(&heap->page_list_free);
    SLIST_INIT(&heap->page_list_used);

    while (elts-- > 0)
    {
        if (heap_page_new_(heap) == NULL)
        {
            return NULL;
        }
    }

    return heap;
} /* _lz_heap_new */

static void
heap_free_(lz_heap * heap, void * d)
{
    lz_heap_page * page;

    if (lz_unlikely(!heap || !d))
    {
        return;
    }

    page = (lz_heap_page *)((char *)(d - offsetof(lz_heap_page, data)));

    /* NOTE: we should return something to designate errors like this */
    if (!page || !(page->data == d))
    {
        return;
    }

    SLIST_REMOVE(&heap->page_list_used, page, lz_heap_page_s, next);
    SLIST_INSERT_HEAD(&heap->page_list_free, page, next);
}

static void *
heap_alloc_(lz_heap * heap)
{
    lz_heap_page * page;

    if (SLIST_EMPTY(&heap->page_list_free))
    {
        if (!heap_page_new_(heap))
        {
            return NULL;
        }
    }

    if (!(page = SLIST_FIRST(&heap->page_list_free)))
    {
        return NULL;
    }

    SLIST_REMOVE(&heap->page_list_free, page, lz_heap_page_s, next);
    SLIST_INSERT_HEAD(&heap->page_list_used, page, next);

    return page->data;
}

lz_alias(heap_alloc_, lz_heap_alloc);
lz_alias(heap_new_, lz_heap_new);
lz_alias(heap_free_, lz_heap_free);
