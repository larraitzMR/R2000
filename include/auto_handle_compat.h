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
 * $Id: auto_handle_compat.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header takes the compatibility library handle traits and the 
 *     AutoHandle class and makes typedefs for the different compatibility
 *     library resource types.
 *
 *****************************************************************************
 */

#ifndef AUTO_HANDLE_COMPAT_H_INCLUDED
#define AUTO_HANDLE_COMPAT_H_INCLUDED

#include "compat_handle_traits.h"
#include "auto_handle.h"

namespace rfid
{

// typedefs for AutoHandle wrappers around the CPL resource handle types
typedef AutoHandle<CplSemaphoreHandle>  CplSemaphoreAutoHandle;
typedef AutoHandle<CplMutexHandle>      CplMutexAutoHandle;
typedef AutoHandle<CplCondHandle>       CplCondAutoHandle;
typedef AutoHandle<CplFileHandle,
                   CplFileHandleTraits> CplFileAutoHandle;

} // namespace rfid

#endif // #ifndef AUTO_HANDLE_COMPAT_H_INCLUDED
