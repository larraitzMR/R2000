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
 * $Id: compat_thread.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description: Basic functions for starting new threads and manipulating
 *     them in limited ways.
 *
 *****************************************************************************
 */

#ifndef COMPAT_THREAD_
#define COMPAT_THREAD_

#include <windows.h>

#include "compat_lib.h"

/****************************************************************************
 * Name:        struct priv_CPL_Thread
 * Description: Encapsulates the contents of the CPL_Thread data type, which
 *              at the moment has just two fields, one of which is filled in
 *              but not currently used.
 * NOTE: This structure is architecture-dependent. DON'T USE ITS CONTENTS.
 ****************************************************************************/
struct priv_CPL_Thread {
    HANDLE threadHandle;  /* A handle to the thread */
    DWORD  threadID;      /* The thread's ID */
};

typedef struct priv_CPL_Thread CPL_Thread;
typedef DWORD CPL_ThreadID;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************
 *  Function:    CPL_ThreadCreate
 *  Description: Creates a new thread.
 *  Parameters:  thread         [OUT] Where to store the ID of the new thread.
 *               start_routine  [IN]  Where the new thread starts execution.
 *               arg            [IN]  The argument to supply to start_routine.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_ThreadCreate(CPL_Thread * thread,
                                   void     *(* start_routine )(void*),
                                   void       * arg);

/****************************************************************************
 *  Function:    CPL_ThreadEqual
 *  Description: Determine if two thread objects represent the same thread.
 *  Parameters:  t1         [IN] LHS of the object comparison
 *               t2			[IN] RHS of the object comparison
 *  Returns:     Non-zero (TRUE ) if equal, zero ( FALSE ) otherwise.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_ThreadEqual(const CPL_Thread * const t1,
                                  const CPL_Thread * const t2);

/****************************************************************************
 *  Function:    CPL_ThreadDetach
 *  Description: Declares that the exit value of the thread won't be checked.
 *  Parameters:  thread   [IN] The thread whose return value is of no interest.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_ThreadDetach(const CPL_Thread * const thread);

/****************************************************************************
 *  Function:    CPL_ThreadJoin
 *  Description: Waits for a thread to exit, and retrieves its exit value.
 *  Parameters:  thread    [IN]  The thread whose exit value is desired.
 *               value_ptr [OUT] The location to place the return value.
 *  Returns:     Zero on success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_ThreadJoin(const CPL_Thread * const thread,
                                 void            **       value_ptr);

/****************************************************************************
 *  Function:    CPL_ThreadExit
 *  Description: Causes the calling thread to exit prematurely.
 *  Parameters:  value_ptr [IN] The value to use as the thread's exit value.
 *  Returns:     This function does not return.
 ****************************************************************************/
CPL_EXPORT void CPL_ThreadExit(void * value_ptr);

inline CPL_ThreadID CPL_ThreadGetID(void) { return GetCurrentThreadId(); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMPAT_THREAD_ */
