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
 * $Id: auto_lock_compat.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header takes the compatibility library lock traits and the 
 *     AutoLock class and makes typedefs for the different compatibility
 *     library resource types.
 *
 *****************************************************************************
 */

#ifndef AUTO_LOCK_COMPAT_H_INCLUDED
#define AUTO_LOCK_COMPAT_H_INCLUDED

#include "compat_lock_traits.h"
#include "auto_lock.h"

namespace rfid
{

// Some typedefs that should make life easier

typedef AutoLock<CplSemaphoreHandle>                CplSemaphoreAutoLock;
typedef AutoLock<CplMutexHandle>                    CplMutexAutoLock;
typedef AutoLock<CplFileHandle, CplFileLockTraits>  CplFileAutoLock;

} // namespace rfid

#endif // AUTO_LOCK_COMPAT_H_INCLUDED

