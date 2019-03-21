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
 * $Id: compat_handles.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header presents typedefs for handles to the compatibility library
 *     resource types.
 *     
 *
 *****************************************************************************
 */

#ifndef COMPAT_HANDLES_H_INCLUDED
#define COMPAT_HANDLES_H_INCLUDED

#include "rfid_platform_types.h"
#include "compat_sem.h"
#include "compat_mutex.h"
#include "compat_cond.h"
#include "compat_thread.h"

namespace rfid
{

// typedefs that declare handles for the CPL resource types
typedef CPL_Semaphore*  CplSemaphoreHandle;
typedef CPL_Mutex*      CplMutexHandle;
typedef CPL_Cond*       CplCondHandle;
typedef CPL_Thread*     CplThreadHandle;
typedef INT32S          CplFileHandle;

} // namespace rfid

#endif // #ifndef COMPAT_HANDLES_H_INCLUDED
