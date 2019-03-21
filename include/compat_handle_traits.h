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
 * $Id: compat_handle_traits.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header presents traits classes for the compatibility library resource
 *     types.  These traits are used in the AutoHandle class (see auto_handle.h).
 *     
 *
 *****************************************************************************
 */

#ifndef COMPAT_HANDLE_TRAITS_H_INCLUDED
#define COMPAT_HANDLE_TRAITS_H_INCLUDED

#include <stddef.h>
#include "compat_handles.h"
#include "compat_fildes.h"

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: HandleTraits
//
// Description: Used to encapsulate the traits for a particular handle type.
//     This class has two static functions that must be supported by all
//     handle traits classes:
//     InvalidHandle() - returns the value for an invalid lock handle
//     CloseHandle() - close the handle
//
// Template Parameters:
//     HandleType - the type of handle for which traits are defined
////////////////////////////////////////////////////////////////////////////////
template <typename HandleType> class HandleTraits;

////////////////////////////////////////////////////////////////////////////////
// Name: HandleTraits<CplSemaphoreHandle>
//
// Description: The handle traits for the cross-platform semaphore.
////////////////////////////////////////////////////////////////////////////////
template <> class HandleTraits<CplSemaphoreHandle>
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
    // Name:        CloseHandle
    // Description: Closes the CPL semaphore handle.
    // Parameters:  handle - the CPL semaphore handle to close
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    static void CloseHandle(
        CplSemaphoreHandle handle
        )
    {
        CPL_SemDestroy(handle);
    } // CloseHandle
}; // class HandleTraits<CplSemaphoreHandle>

////////////////////////////////////////////////////////////////////////////////
// Name: HandleTraits<CplMutexHandle>
//
// Description: The handle traits for the cross-platform mutex.
////////////////////////////////////////////////////////////////////////////////
template <> class HandleTraits<CplMutexHandle>
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
    // Name:        CloseHandle
    // Description: Closes the CPL mutex handle.
    // Parameters:  handle - the CPL mutex handle to close
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    static void CloseHandle(
        CplMutexHandle handle
        )
    {
        CPL_MutexDestroy(handle);
    } // CloseHandle
}; // class HandleTraits<CplMutexHandle>

////////////////////////////////////////////////////////////////////////////////
// Name: HandleTraits<CplCondHandle>
//
// Description: The handle traits for the cross-platform condition variable.
////////////////////////////////////////////////////////////////////////////////
template <> class HandleTraits<CplCondHandle>
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name:        InvalidHandle
    // Description: Returns the value for an invalid CPL condition variable
    //              handle
    // Parameters:  None
    // Returns:     The value for an invalid CPL condition variable handle
    ////////////////////////////////////////////////////////////////////////////
    static CplCondHandle InvalidHandle()
    {
        return NULL;
    } // InvalidHandle

    ////////////////////////////////////////////////////////////////////////////
    // Name:        CloseHandle
    // Description: Closes the CPL condition variable handle.
    // Parameters:  handle - the CPL condition variable handle to close
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    static void CloseHandle(
        CplCondHandle handle
        )
    {
        CPL_CondDestroy(handle);
    } // CloseHandle
}; // class HandleTraits<CplCondHandle>

// Some convenient typedefs for the CPL HandleTraits<>
typedef HandleTraits<CplSemaphoreHandle>    CplSemaphoreHandleTraits;
typedef HandleTraits<CplMutexHandle>        CplMutexHandleTraits;
typedef HandleTraits<CplCondHandle>         CplCondHandleTraits;

////////////////////////////////////////////////////////////////////////////////
// Name: CplFileHandleTraits
//
// Description: The handle traits for the cross-platform mutex.  The 
//     HandleTraits template is not used because the file handle is a 32-bit
//     signed integer and if there are other handles that are the same base 
//     type, the compiler cannot differentiate between them.
////////////////////////////////////////////////////////////////////////////////
class CplFileHandleTraits
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
    // Name:        CloseHandle
    // Description: Closes the CPL file handle.
    // Parameters:  handle - the CPL file handle to close
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    static void CloseHandle(
        CplFileHandle handle
        )
	{
        CPL_FileClose(handle);
	} // CloseHandle
}; // class CplFileHandleTraits

} // namespace rfid

#endif // #ifndef COMPAT_HANDLE_TRAITS_H_INCLUDED
