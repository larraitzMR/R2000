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
 * $Id: compat_sem.h 66483 2011-01-05 00:40:04Z dshaheen $
 * 
 * Description: Defines semaphores and the functions that manipulate them.
 *
 *****************************************************************************
 */

#ifndef COMPAT_SEM_
#define COMPAT_SEM_

#include <windows.h>

#include "compat_lib.h"

/****************************************************************************
 * Name:        struct priv_CPL_Sem
 * Description: Encapsulates the contents of the CPL_Semaphore data type, which
 *              at the moment is just a handle to the semaphore itself.
 * NOTE: This structure is architecture-dependent. DON'T USE ITS CONTENTS.
 ****************************************************************************/
struct priv_CPL_Sem {
    HANDLE semHandle;   /* A handle to the semaphore */
};

typedef struct priv_CPL_Sem CPL_Semaphore;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************
 *  Function:    CPL_SemInit
 *  Description: Initializes a semaphore.
 *  Parameters:  sem     [OUT] The semaphore to initialize.
 *               value    [IN] The initial count of the semaphore.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_SemInit(CPL_Semaphore * sem,
                              INT32U          value);

/****************************************************************************
 *  Function:    CPL_SemWait
 *  Description: Waits for a semaphore to have a positive count (if necessary),
 *               then decreases the count and returns.
 *  Parameters:  sem    [IN] The semaphore to wait for.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_SemWait(CPL_Semaphore * sem);

/****************************************************************************
 *  Function:    CPL_SemWaitTimeout
 *  Description: Waits for the specified timeout for a semaphore to have a 
 *               positive count (if necessary), then decreases the count and 
 *               returns.
 *  Parameters:  sem     [IN] The semaphore to wait for.
 *               timeout [IN] The timeout value in milliseconds
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_SemWaitTimeout(CPL_Semaphore * sem, INT32U timeout);

/****************************************************************************
 *  Function:    CPL_SemTryWait
 *  Description: If the semaphore has a positive count, decrements it and
 *               returns success.  Otherwise, returns failure.
 *  Parameters:  sem    [IN] The semaphore to check the value of.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.  If the
 *               semaphore had a count of zero, it returns CPL_WARN_WOULDBLOCK.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_SemTryWait(CPL_Semaphore * sem);

/****************************************************************************
 *  Function:    CPL_SemRelease
 *  Description: Increments the semaphore's count.
 *  Parameters:  sem    [IN] The semaphore whose count is to be incremented.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_SemRelease(CPL_Semaphore * sem);

/****************************************************************************
 *  Function:    CPL_SemDestroy
 *  Description: Destroys the semaphore.
 *  Parameters:  sem    [IN] The semaphore to destroy.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_SemDestroy(CPL_Semaphore * sem);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMPAT_SEM_ */
