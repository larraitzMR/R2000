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
 * $Id: compat_cond.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description: Describes condition variables (or events, as Windows calls
 *     them).
 *
 *****************************************************************************
 */

#ifndef COMPAT_COND_
#define COMPAT_COND_

#include <windows.h>

#include "compat_lib.h"
#include "compat_time.h"

/****************************************************************************
 * Name:        struct priv_CPL_Cond
 * Description: Encapsulates the contents of the CPL_Cond data type, which
 *              contains the handles for two Events.
 * NOTE: This structure is architecture-dependent. DON'T USE ITS CONTENTS.
 ****************************************************************************/
struct priv_CPL_Cond {
    HANDLE resetEvent[ 2 ];     /* [ 0 ] The event for CPL_CondSignal */
                                /* [ 1 ] The event for CPL_CondBroadcast */
};

typedef struct priv_CPL_Cond CPL_Cond;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************
 *  Function:    CPL_CondInit
 *  Description: Initializes a condition variable for later use.
 *  Parameters:  cond   [OUT] A pointer to the condition variable to init.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_CondInit(CPL_Cond * cond);

/****************************************************************************
 *  Function:    CPL_CondWait
 *  Description: Waits indefinitely for a condition variable to be signaled.
 *  Parameters:
 *    cond   [IN] The condition variable on which to await a signal.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_CondWait(CPL_Cond * cond);

/****************************************************************************
 *  Function:    CPL_CondAbsTimedWait
 *  Description: Waits for a condition variable to be signaled, with an upper
 *               limit on how long to wait defined in absolute time.
 *  Parameters:
 *    cond    [IN] The condition variable on which to await a signal.
 *    abstime [IN] The time to give up on waiting for the condition variable.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_CondAbsTimedWait(CPL_Cond                  * cond,
                                       const struct CPL_TimeSpec * abstime);

/****************************************************************************
 *  Function:    CPL_CondRelTimedWait
 *  Description: Waits for a condition variable to be signaled, with an upper
 *               limit on how long to wait defined relative to current time.
 *  Parameters:
 *    cond     [IN] The condition variable on which to await a signal.
 *    interval [IN] How long to wait for the condition variable.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_CondRelTimedWait(CPL_Cond                  * cond,
                                       const struct CPL_TimeSpec * interval);

/****************************************************************************
 *  Function:    CPL_CondSignal
 *  Description: Awakens exactly one of the waiting threads.
 *  Parameters:  cond   [IN] The condition variable to signal.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_CondSignal(CPL_Cond * cond);

/****************************************************************************
 *  Function:    CPL_CondBroadcast
 *  Description: Awakens all of the waiting threads.
 *  Parameters:  cond   [IN] The condition variable to signal.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_CondBroadcast(CPL_Cond * cond);

/****************************************************************************
 *  Function:    CPL_CondDestroy
 *  Description: Destroys a condition variable and frees all its resources.
 *  Parameters:  cond   [IN] The condition variable to destroy.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_CondDestroy(CPL_Cond * cond);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMPAT_COND_ */
