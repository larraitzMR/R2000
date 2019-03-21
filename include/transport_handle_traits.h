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
 * $Id: transport_handle_traits.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header presents traits classes for the transport library resource
 *     types.  These traits are used in the AutoHandle class (see auto_handle.h).
 *     
 *
 *****************************************************************************
 */

#ifndef TRANSPORT_HANDLE_TRAITS_H_INCLUDED
#define TRANSPORT_HANDLE_TRAITS_H_INCLUDED

#include "translib.h"

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: TransportRadioHandleTraits
//
// Description: The handle traits for the transport library's radio.  The 
//     HandleTraits template is not used because the radio handle is a 32-bit
//     signed integer and if there are other handles that are the same base 
//     type, the compiler cannot differentiate between them.
////////////////////////////////////////////////////////////////////////////////
struct TransportRadioHandleTraits
{
    ////////////////////////////////////////////////////////////////////////////
    // Name:        InvalidHandle
    // Description: Returns the value for an invalid transport radio handle
    // Parameters:  None
    // Returns:     The value for an invalid transport radio handle
    ////////////////////////////////////////////////////////////////////////////
    static TransHandle InvalidHandle()
    {
        return static_cast<TransHandle>(-1);
    } // InvalidHandle

    ////////////////////////////////////////////////////////////////////////////
    // Name:        CloseHandle
    // Description: Closes the transport radio handle.
    // Parameters:  handle - the transport radio handle to close
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    static void CloseHandle(
        TransHandle handle
        )
	{
        RfTrans_CloseRadio(handle);
	} // CloseHandle
}; // struct TransportRadioHandleTraits

} // namespace rfid

#endif // #ifndef TRANSPORT_HANDLE_TRAITS_H_INCLUDED
