/*
 *****************************************************************************
 *                                                                           *
 *                 IMPINJ CONFIDENTIAL AND PROPRIETARY                       *
 *                                                                           *
 * This source code is the sole property of Impinj, Inc.  Reproduction or    *
 * utilization of this source code in whole or in part is forbidden without  *
 * the prior written consent of Impinj, Inc.                                 *
 *                                                                           *
 * (c) Copyright Impinj, Inc. 2009. All rights reserved.                     *
 *                                                                           *
 *****************************************************************************
 */

/*
 *****************************************************************************
 *
 * $Id: compat_lib.h 66189 2010-12-04 01:51:32Z dshaheen $
 * 
 * Description: The core of the cross-platform library.  Will be included
 *     by all other headers in the library.
 *
 *****************************************************************************
 */

#ifndef COMPAT_LIB_
#define COMPAT_LIB_

/****************************************************************************
* Next is to prevent dup auto-inclusions by Windows of things like winsock 1
****************************************************************************/

#define WIN32_LEAN_AND_MEAN


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rfid_platform_types.h"

#ifndef MAX_PATH
    #ifdef _MAX_PATH
        #define MAX_PATH _MAX_PATH
    #endif
#endif

#ifdef UNDER_CE
/* The buffer used in tmpnam() needs to be at least this long */
#define L_tmpnam MAX_PATH
#endif /* UNDER_CE */

#ifdef CPL_BUILD_LIBRARY
#define CPL_EXPORT __declspec(dllexport)
#else
#define CPL_EXPORT __declspec(dllimport)
#endif /* CPL_BUILD_LIBRARY */

#ifdef __cplusplus
extern "C" {
#else /* not __cplusplus */
#define inline __inline
#endif /* __cplusplus */

/****************************************************************************
 *  Function:    CPL_HostToMac16
 *  Description: Converts a 16-bit value from host endianness to the
 *               little-endian format used by the MAC.
 *  Parameters:  hostWord   [IN] The value to convert.
 *  Returns:     The converted value.
 ****************************************************************************/
CPL_EXPORT INT16U CPL_HostToMac16(INT16U hostWord);

/****************************************************************************
 *  Function:    CPL_HostToMac32
 *  Description: Converts a 32-bit value from host endianness to the
 *               little-endian format used by the MAC.
 *  Parameters:  hostDword  [IN] The value to convert.
 *  Returns:     The converted value.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_HostToMac32(INT32U hostDword);

/****************************************************************************
 *  Function:    CPL_HostToMac64
 *  Description: Converts a 64-bit value from host endianness to the
 *               little-endian format used by the MAC.
 *  Parameters:  hostQword  [IN] The value to convert.
 *  Returns:     The converted value.
 ****************************************************************************/
CPL_EXPORT INT64U CPL_HostToMac64(INT64U hostQword);

/****************************************************************************
 *  Function:    CPL_MacToHost16
 *  Description: Converts a 16-bit value from the little-endian format used
 *               by the MAC to the host's endianness.
 *  Parameters:  macWord   [IN] The value to convert.
 *  Returns:     The converted value.
 ****************************************************************************/
CPL_EXPORT INT16U CPL_MacToHost16(INT16U macWord);

/****************************************************************************
 *  Function:    CPL_MacToHost32
 *  Description: Converts a 32-bit value from the little-endian format used
 *               by the MAC to the host's endianness.
 *  Parameters:  macDword  [IN] The value to convert.
 *  Returns:     The converted value.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_MacToHost32(INT32U macDword);

/****************************************************************************
 *  Function:    CPL_MacToHost64
 *  Description: Converts a 64-bit value from the little-endian format used
 *               by the MAC to the host's endianness.
 *  Parameters:  macQword  [IN] The value to convert.
 *  Returns:     The converted value.
 ****************************************************************************/
CPL_EXPORT INT64U CPL_MacToHost64(INT64U macQword);

inline int CPL_StrCaseCmp(const char *s1, const char *s2) {
    return _stricmp(s1,s2);
}
inline int CPL_WideStrCaseCmp(const wchar_t *s1, const wchar_t *s2) {
    return _wcsicmp(s1,s2);
}
inline int CPL_StrNCaseCmp(const char *s1, const char *s2, INT32U n) {
    return _strnicmp(s1,s2,n);
}
inline int CPL_WideStrNCaseCmp(const wchar_t *s1, const wchar_t *s2, INT32U n){
    return _wcsnicmp(s1,s2,n);
}

#if (defined(UNDER_CE) && (_WIN32_WCE==0x500))  /* wince 5.0 only */
#include <winerror.h>
#include <winbase.h>

inline char *CPL_StrToken(char *s, char *delim, char **ptrptr) {
    if ((NULL == ptrptr) ||
        ((NULL == s) && (NULL == *ptrptr)))
    {
        SetLastError(ERROR_INVALID_DATA);
        return NULL;
    } else {
        char *rval;
        char *str = (NULL == s) ? *ptrptr : s;
        size_t oldlen, newlen;

        oldlen = strlen(str);
        rval = strtok(str, delim);
        newlen = strlen(str);
        if (oldlen == newlen) {
            *ptrptr = NULL;
        } else {
            *ptrptr = str + newlen + 1;
        }
        return rval;
    }
}
#else  /* not wince 5.00 */

inline char *CPL_StrToken(char *s, char *delim, char **ptrptr) {
    return strtok_s(s, delim, ptrptr);
}

#endif

#ifdef UNDER_CE
/****************************************************************************
 *  Function:    abort
 *  Description: Exits the program without calling atexit() functions.
 *  Parameters:  None.
 *  Returns:     This function does not return.
 ****************************************************************************/
CPL_EXPORT void abort(void);

/****************************************************************************
 *  Function:    bsearch
 *  Description: Does a binary search of a sorted array looking for a key.
 *  Parameters:  key     [IN] A pointer to the object to search for.
 *               base    [IN] A pointer to the initial element of the array.
 *               nmemb   [IN] The number of elements in the array.
 *               size    [IN] The size of the elements in the array.
 *               compar  [IN] A function to use to compare two elements.
 *  Returns:     A pointer to the key in the array if it is present, or
 *               NULL if it is not present.
 ****************************************************************************/
CPL_EXPORT void *bsearch(const void * key,
                         const void * base,
                         size_t       nmemb,
                         size_t       size,
                         int       (* compar )(const void *, const void *));

/****************************************************************************
 *  Function:    freopen
 *  Description: Identical to fopen(), but reuses an existing FILE *.
 *  Parameters:  path        [IN] A path to the file to open.
 *               mode        [IN] Whether to open the file to read &/or write.
 *               stream  [IN/OUT] The FILE * to reuse.
 *  Returns:     Its third argument on success, or NULL on error.
 ****************************************************************************/
CPL_EXPORT FILE *freopen(const char * path,
                         const char * mode,
                         FILE       * stream);

/****************************************************************************
 *  Function:    remove
 *  Description: Deletes a file from the filesystem.
 *  Parameters:  pathname  [IN] The file to delete.
 *  Returns:     Zero on success, or -1 on error.
 ****************************************************************************/
CPL_EXPORT int remove(const char *pathname);

/****************************************************************************
 *  Function:    rename
 *  Description: Renames a file.
 *  Parameters:  oldpath   [IN] The current name of the file.
 *               newpath   [IN] The name to rename the file to.
 *  Returns:     Zero on success, or -1 on error.
 ****************************************************************************/
CPL_EXPORT int rename(const char *oldpath, const char *newpath);

/****************************************************************************
 *  Function:    rewind
 *  Description: Rewinds a FILE * back to the beginning of the file.
 *  Parameters:  stream    [IN] The FILE * to rewind.
 *  Returns:     Nothing.
 ****************************************************************************/
CPL_EXPORT void rewind(FILE *stream);

/****************************************************************************
 *  Function:    tmpfile
 *  Description: Creates a FILE * to a temporary file.
 *  Parameters:  None.
 *  Returns:     A FILE * to an empty file that did not previously exist,
 *               or NULL on failure.
 ****************************************************************************/
CPL_EXPORT FILE *tmpfile (void);

/****************************************************************************
 *  Function:    tmpnam
 *  Description: Generates a name for a file that does not currently exist.
 *  Parameters:  s   [OUT] An optional pointer to a place to store the
 *                         filename.  If this is NULL, static storage is used
 *                         and race conditions may result.
 *  Returns:     The name of the temporary file, or NULL on failure.
 ****************************************************************************/
CPL_EXPORT char *tmpnam(char *s);

#endif /* UNDER_CE */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMPAT_LIB_ */
