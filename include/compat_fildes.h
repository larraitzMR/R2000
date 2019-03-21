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
 * $Id: compat_fildes.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description: Defines functions that manipulate raw files and device
 *     virtual files.
 *
 *****************************************************************************
 */

#ifndef COMPAT_FILDES_
#define COMPAT_FILDES_

#include <windows.h>

#include "compat_lib.h"

/* Valid non-zero values for the accessMode argument to CPL_FileOpen */
enum CPL_accessModeValues {
    CPL_READ      = GENERIC_READ,
    CPL_WRITE     = GENERIC_WRITE,
    CPL_READWRITE = GENERIC_READ | GENERIC_WRITE
};

/* Valid components to the optFlag argument to CPL_FileOpen */
enum CPL_optFlagValues {
    CPL_APPEND = 0x01,
    CPL_SYNC   = 0x02,
    CPL_CREAT  = 0x04,
    CPL_TRUNC  = 0x08,
    CPL_EXCL   = 0x10
};

/* Valid values for the offset argument to CPL_FileSeek */
enum CPL_offsetValues {
    CPL_SEEKSTART   = FILE_BEGIN,
    CPL_SEEKCURRENT = FILE_CURRENT,
    CPL_SEEKEND     = FILE_END
};

/* Valid components of the operation argument of CPL_FileLock */
enum CPL_lockOperationValues {
    CPL_SHAREDLOCK    = 0,
    CPL_EXCLUSIVELOCK = LOCKFILE_EXCLUSIVE_LOCK,
    CPL_UNLOCK        = 0x1000,
    CPL_NONBLOCKING   = LOCKFILE_FAIL_IMMEDIATELY
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************
 *  Function:    CPL_FileOpen
 *  Description: Opens a file for use by the following functions.
 *  Parameters:
 *    path        [IN] The path to the file to be opened or created.
 *    accessMode  [IN] What kinds of access are needed.  Valid values are
 *                     zero, CPL_READ, CPL_WRITE, and CPL_READWRITE (which is
 *                     just shorthand for CPL_READ | CPL_WRITE).
 *    optFlag     [IN] A bitfield of flags.  Values to be ORed together:
 *                        CPL_APPEND  Seek to the end of the file.
 *                        CPL_SYNC    Keep the physical device in sync.
 *                        CPL_CREAT   Create the file if necessary.
 *                        CPL_TRUNC   Truncate the file if it exists.
 *                        CPL_EXCL    Return an error if file exists.
 *  Returns:     An open file descriptor, or -1 on error.
 ****************************************************************************/
CPL_EXPORT INT32S CPL_FileOpen(const char *path,
                               INT32S      accessMode,
                               INT32S      optFlag);

/****************************************************************************
 *  Function:    CPL_FileRead
 *  Description: Reads bytes from a file descriptor.
 *  Parameters:  fildes   [IN] The file descriptor to read from.
 *               buf     [OUT] A buffer to put the read bytes into.
 *               nbyte    [IN] The maximum number of bytes to read.
 *  Returns:     The actual number of bytes read, or -1 on error.
 ****************************************************************************/
CPL_EXPORT INT32S CPL_FileRead(INT32S fildes,
                               void * buf,
                               INT32U nbyte);

/****************************************************************************
 *  Function:    CPL_FileWrite
 *  Description: Writes bytes to a file descriptor.
 *  Parameters:  fildes  [IN] The file descriptor to write to.
 *               buf     [IN] A buffer containing the bytes to write.
 *               nbyte   [IN] The maximum number of bytes to write.
 *  Returns:     The actual number of bytes written, or -1 on error.
 ****************************************************************************/
CPL_EXPORT INT32S CPL_FileWrite(INT32S       fildes,
                                const void * buf,
                                INT32U       nbyte);

/****************************************************************************
 *  Function:    CPL_FileSeek
 *  Description: Moves the location where read and write operations occur.
 *  Parameters:  fildes  [IN] The file descriptor to move the pointer of.
 *               offset  [IN] Where to move the file pointer, relative to...
 *               whence  [IN] The location of an offset of 0.  Valid values
 *                            are SEEK_SET (beginning), SEEK_CUR and SEEK_END.
 *  Returns:     The new offset (measured from start of file) or -1 on error.
 ****************************************************************************/
CPL_EXPORT INT32S CPL_FileSeek(INT32S fildes,
                               INT32S offset,
                               INT32S whence);

/****************************************************************************
 *  Function:    CPL_FileLock
 *  Description: Locks or unlocks the file pointed to by a file descriptor.
 *  Parameters:  fildes    [IN] A descriptor for the file to lock or unlock.
 *               operation [IN] What file-locking operation to perform.  Use
 *                              CPL_SHAREDLOCK for a shared lock,
 *                              CPL_EXCLUSIVELOCK for an exclusive lock,
 *                              and CPL_UNLOCK to unlock.
 *                              Adding CPL_NONBLOCKING via OR makes it a
 *                              non-blocking operation.
 *  Returns:     Zero on success, or -1 on error.  If CPL_NONBLOCKING was
 *               provided and the file was locked, CPL_GetError() returns
 *               CPL_WARN_WOULDBLOCK.
 ****************************************************************************/
CPL_EXPORT INT32S CPL_FileLock(INT32S fildes,
                               INT32S operation);

/****************************************************************************
 *  Function:    CPL_FileClose
 *  Description: Closes a file descriptor.
 *  Parameters:  fildes   [IN] The file descriptor to close.
 *  Returns:     Zero on success, or -1 on error.
 ****************************************************************************/
CPL_EXPORT INT32S CPL_FileClose(INT32S fildes);

/****************************************************************************
 *  Function:    CPL_FileFlush
 *  Description: Flushes all written bytes to storage
 *  Parameters:  fildes  [IN] The file descriptor to write to.
 *  Returns:     Zero on successs, or -1 on error.
 ****************************************************************************/
CPL_EXPORT INT32S CPL_FileFlush(INT32S fildes);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMPAT_FILDES_ */
