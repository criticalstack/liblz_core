#pragma once

#include <dirent.h>

/**
 * @brief the callback function decl.
 * @return 0 on success, -1 on error
 */
typedef int (* lz_file_readdir_iter)(struct dirent *,
    const char *,
    const char *, void *);


/**
 * @brief read a directory recursively, each time calling
 *        the `iter` callback.
 *
 * @param[in] path path to start walking
 * @param[in] iter the callback executed for each file
 * @param[in] arg argument passed to the callback
 *
 * @return 0 on success, -1 on error
 */
int lz_file_recursive_readdir(const char * path,
    lz_file_readdir_iter                   iter,
    void                                 * arg);


/**
 * @brief read a directory (non-recursive)
 *
 * @param[in] path path to walk
 * @param[in] iter callback to be executed for each file
 * @param[in] arg argument passed to the callback
 *
 * @return 0 on success, -1 on error
 */
int lz_file_readdir(const char * path,
    lz_file_readdir_iter         iter,
    void                       * arg);


/**
 * @brief concat a base path and a file together
 * @example lz_file_concat(&buffer, "/home/ellzey" "passwords.txt"
 *          creates: "/home/ellzey/passwords.txt"
 *
 * @param[out] out a pre-allocated buffer to store the data
 * @param[in] prefix the basepath
 * @param[in] postfix the filename
 *
 * @return 0 on success, -1 on error
 */
int lz_file_concat(char ** out,
    const char           * prefix,
    const char           * postfix);
