#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <dirent.h>

#include <liblz.h>
#include <liblz/lzapi.h>

#include "ffile.h"

static int
file_concat_(char ** out, const char * prefix, const char * postfix)
{
    const char * fmt;
    char       * concated;
    size_t       concated_sz;
    size_t       prefix_sz;
    size_t       postfix_sz;
    int          res;

    if (lz_unlikely(!out || !postfix))
    {
        return -1;
    }

    if ((prefix_sz = strlen(prefix)) >= PATH_MAX)
    {
        errno = ENAMETOOLONG;
        return -1;
    }

    if ((postfix_sz = strlen(postfix)) >= NAME_MAX)
    {
        errno = ENAMETOOLONG;
        return -1;
    }

    concated_sz = prefix_sz + postfix_sz + 1;

    if (!(concated = calloc(concated_sz, 1)))
    {
        return -1;
    }

    fmt = "%s/%s";

    if (prefix[0] == '/' && prefix[1] == '\0')
    {
        fmt = "%s%s";
    }

    res = snprintf(concated, concated_sz, fmt, prefix, postfix);

    if (res >= concated_sz || res < 0)
    {
        lz_safe_free(concated, free);

        return -1;
    }
} /* file_concat_ */

static int
file_readdir_(const char * path, lz_file_readdir_iter iter, void * arg)
{
    struct dirent * dent;
    DIR           * directory;
    char          * file_concat;
    int             res;

    if (lz_unlikely(!path || !iter))
    {
        return -1;
    }

    if (!(directory = opendir(path)))
    {
        return (iter)(NULL, path, NULL, arg);
    }

    res = 0;

    while ((dent = readdir(directory)))
    {
        file_concat = NULL;

        /* check for "." and "..", and ignore them. */
        if (dent->d_name[0] == '.')
        {
            if (dent->d_name[1] == '.')
            {
                continue;
            }
        }

        if ((res = file_concat_(&file_concat, path, dent->d_name)) == -1)
        {
            break;
        }

        if ((res = (iter)(dent, path, file_concat, arg)) != 0)
        {
            lz_safe_free(file_concat, free);
            break;
        }

        lz_safe_free(file_concat, free);
    }

    if (file_concat != NULL)
    {
        lz_safe_free(file_concat, free);
    }

    closedir(directory);

    return res;
} /* lz_file_readdir */

struct f_dat__ {
    lz_file_readdir_iter og_iter;
    void               * og_args;
};

/**
 * @brief function executed by file_readdir_().
 * @note  there is a private structure `sturct f_dat__`
 *        which contains the original user-specified
 *        directory walk callback.
 *
 * @param dent the struct dirent entry
 * @param path </foo/bar>/file where </foo/bar> == path
 * @param path_and_basename </foo/bar/baz> full path + filename
 * @param args arguments supplied by the caller (in this case,
 *        `struct f_dat__ *`
 *
 * @return 0 on success, -1 on error
 */
static int
file_readdir_iter_(struct dirent * dent, const char * path, const char * path_and_basename, void * args)
{
    struct f_dat__ * filectx;
    int              res = 0;

    if (lz_unlikely((filectx = (struct f_dat__ *)args) == NULL))
    {
        return -1;
    }

    if (dent == NULL)
    {
        return -1;
    }

    if (filectx->og_iter == NULL)
    {
        return -1;
    }

    if ((res = (filectx->og_iter)(dent, path, path_and_basename, filectx->og_args)))
    {
        return res;
    }

    if (dent->d_type == DT_DIR)
    {
        return file_readdir_(path_and_basename, file_readdir_iter_, args);
    }

    return 0;
}

static int
file_recursive_readdir_(const char * path, lz_file_readdir_iter iter, void * args)
{
    int            res;
    struct f_dat__ file_dat = {
        .og_iter = iter,
        .og_args = args
    };


    if (lz_unlikely(!path || !iter))
    {
        return -1;
    }

    if ((res = file_readdir_(path, file_readdir_iter_, &file_dat)) != 0)
    {
        return res;
    }

    return 0;
}

lz_alias(file_recursive_readdir_, lz_file_recursive_readdir);
lz_alias(file_concat_, lz_file_concat);
lz_alias(file_readdir_, lz_file_readdir);
