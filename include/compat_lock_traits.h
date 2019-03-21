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
 * $Id: compat_lock_traits.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header presents traits classes for the compatibility library lock
 *     types.  These traits are used in the AutoLock class (see auto_lock.h).
 *     
 *
 *****************************************************************************
 */

#ifndef COMPAT_LOCK_TRAITS_H_INCLUDED
#define COMPAT_LOCK_TRAITS_H_INCLUDED

#include <stddef.h>
#include "compat_handles.h"
#include "compat_fildes.h"

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: LockTraits
//
// Description: Used to encapsulate the traits for a particular lock type.  This
//     class has three static functions that must be supported by all lock
//     traits classes:
//     InvalidHandle() - returns the value for an invalid lock handle
//     Lock() - attempts to acquire the lock
//     Unlock() - releases the lock
//
// Template Parameters:
//     HandleType - the type of handle for which traits are defined
////////////////////////////////////////////////////////////////////////////////
template <typename HandleType> class LockTraits;

////////////////////////////////////////////////////////////////////////////////
// Name: LockTraits<CplSemaphoreHandle>
//
// Description: The lock traits for the cross-platform library semaphore.
////////////////////////////////////////////////////////////////////////////////
template<> class LockTraits<CplSemaphoreHandle>
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name:        InvalidHandle
    // Description: Returns the value for an invalid CPL semaphore handle
    // Parameters:  None
    // Returns:     The value for an invalid CPL semaphore handle
    ////////////////////////////////////////////////////////////////////////////
    static CplSemaphoreHandle InvalidHandle()
    {
        return NULL;
    } // InvalidHandle

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Lock
    // Description: Attempts to obtain the semaphore
    // Parameters:  handle - the handle to the semaphore to obtain
    // Returns:     true - lock successful.  false - lock unsuccessful.
    ////////////////////////////////////////////////////////////////////////////
    static bool Lock(
        CplSemaphoreHandle handle
        )
    {
        return !CPL_SemWait(handle);
    } // Lock

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Unlock
    // Description: Releases the semaphore
    // Parameters:  handle - the handle to the semaphore to release
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    static void Unlock(
        CplSemaphoreHandle handle
        )
    {
        CPL_SemRelease(handle);
    } // Unlock
}; // class LockTraits<CplSemaphoreHandle>

////////////////////////////////////////////////////////////////////////////////
// Name: LockTraits<CplMutexHandle>
//
// Description: The lock traits for the cross-platform library mutex.
////////////////////////////////////////////////////////////////////////////////
template<> class LockTraits<CplMutexHandle>
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name:        InvalidHandle
    // Description: Returns the value for an invalid CPL mutex handle
    // Parameters:  None
    // Returns:     The value for an invalid CPL mutex handle
    ////////////////////////////////////////////////////////////////////////////
    static CplMutexHandle InvalidHandle()
    {
        return NULL;
    } // InvalidHandle

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Lock
    // Description: Attempts to obtain the mutex
    // Parameters:  handle - the handle to the mutex to obtain
    // Returns:     true - lock successful.  false - lock unsuccessful.
    ////////////////////////////////////////////////////////////////////////////
    static bool Lock(
        CplMutexHandle handle
        )
    {
        return !CPL_MutexLock(handle);
    } // Lock

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Unlock
    // Description: Releases the mutex
    // Parameters:  handle - the handle to the mutex to release
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    static void Unlock(
        CplMutexHandle handle
        )
    {
        CPL_MutexUnlock(handle);
    } // Unlock
}; // class LockTraits<CplMutexHandle>

// Some convenient typedefs for the CPL LockTraits<>
typedef LockTraits<CplSemaphoreHandle>  CplSemaphoreLockTraits;
typedef LockTraits<CplMutexHandle>      CplMutexLockTraits;

////////////////////////////////////////////////////////////////////////////////
// Name: CplFileLockTraits
//
// Description: The lock traits for the cross-platform library file descriptor.
//     The LockTraits template is not used because the file handle is a 32-bit
//     signed integer and if there are other handles that are the same base
//     type, there is no way to differentiate them in the template.
////////////////////////////////////////////////////////////////////////////////
class CplFileLockTraits
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name:        InvalidHandle
    // Description: Returns the value for an invalid CPL file handle
    // Parameters:  None
    // Returns:     The value for an invalid CPL file handle
    ////////////////////////////////////////////////////////////////////////////
    static CplFileHandle InvalidHandle()
    {
        return -1;
    } // InvalidHandle

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Lock
    // Description: Attempts to obtain the file lock
    // Parameters:  handle - the handle to the file to lock
    // Returns:     true - lock successful.  false - lock unsuccessful.
    ////////////////////////////////////////////////////////////////////////////
    static bool Lock(
        CplFileHandle handle
        )
    {
        return (!CPL_FileLock(handle, CPL_EXCLUSIVELOCK));
    } // Lock

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Unlock
    // Description: Releases the file lock
    // Parameters:  handle - the handle to the file to unlock
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    static void Unlock(
        CplFileHandle handle
        )
    {
        CPL_FileLock(handle, CPL_UNLOCK);
    } // Unlock
}; // class CplFileLockTraits

} // namespace rfid

#endif // COMPAT_LOCK_TRAITS_H_INCLUDED

