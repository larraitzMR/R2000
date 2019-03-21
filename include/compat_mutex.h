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
 * $Id: compat_mutex.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description: Routines for manipulating intra-process mutexes, which
 *     Windows calls critical sections.
 *
 *****************************************************************************
 */

#ifndef COMPAT_MUTEX_
#define COMPAT_MUTEX_

#include <windows.h>

#include "compat_lib.h"

/****************************************************************************
 * Name:        struct priv_CPL_Mutex
 * Description: Encapsulates the contents of the CPL_Mutex data type, which
 *              at the moment is just a CRITICAL_SECTION.
 * NOTE: This structure is architecture-dependent. DON'T USE ITS CONTENTS.
 ****************************************************************************/
struct priv_CPL_Mutex {
    CRITICAL_SECTION cs;  /* The critical section */
};

typedef struct priv_CPL_Mutex CPL_Mutex;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************
 *  Function:    CPL_MutexInit
 *  Description: Creates a mutex for later use.
 *  Parameters:  mutex [OUT] A pointer to the mutex to be initialized.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_MutexInit(CPL_Mutex * mutex);

/****************************************************************************
 *  Function:    CPL_MutexLock
 *  Description: Locks a mutex, blocking as long as necessary.
 *  Parameters:  mutex    [IN] A pointer to the mutex to lock.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_MutexLock(CPL_Mutex * mutex);

/****************************************************************************
 *  Function:    CPL_MutexTryLock
 *  Description: Attempts to lock a mutex, returning immediately if the
 *               mutex is already locked.
 *  Parameters:  mutex     [IN] A pointer to the mutex to attempt to lock.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 *               Notably, WAIT_WOULD_BLOCK is returned if it's already locked.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_MutexTryLock(CPL_Mutex * mutex);

/****************************************************************************
 *  Function:    CPL_MutexUnlock
 *  Description: Unlocks a mutex.
 *  Parameters:  mutex     [IN] A pointer to the mutex to unlock.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_MutexUnlock(CPL_Mutex * mutex);

/****************************************************************************
 *  Function:    CPL_MutexDestroy
 *  Description: Destroys a mutex, freeing all resources associated with it.
 *  Parameters:  mutex     [IN] A pointer to the mutex to destroy.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_MutexDestroy(CPL_Mutex * mutex);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMPAT_MUTEX_ */
