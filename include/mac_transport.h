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
 * $Id: mac_transport.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header presents the interface for the base class that is a used to
 *     to communicate with a radio's MAC.
 *     
 *
 *****************************************************************************
 */

#ifndef MAC_TRANSPORT_H_INCLUDED
#define MAC_TRANSPORT_H_INCLUDED

#include "rfid_platform_types.h"
#include "rfid_structs.h"

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: MacTransport
//
// Description: This class is used to provide an abstraction of the MAC 
//     transport and is used as the base class for derivations for specific
//     MAC transports.
////////////////////////////////////////////////////////////////////////////////
class MacTransport
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name:        ~MacTransport
    // Description: Cleans up the MAC transport object.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    virtual ~MacTransport() {}

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetTransportCharacteristics
    // Description: Retrieves the transport characteristics for the underlying
    //              transport.
    // Parameters:  pDriverVersion - pointer to structure that upon return will
    //              contain the driver version information
    //              pMaxBufferSize - pointer to 32-bit unsigned integer that
    //              upon return will contain the maximum transfer buffer size
    //              pMaxPacketSize - pointer to 32-bit unsigned integer that
    //              upon return will contain the maximum transfer packet size
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    virtual void GetTransportCharacteristics(
        RFID_VERSION*   pDriverVersion,
        INT32U*         pMaxBufferSize,
        INT32U*         pMaxPacketSize
        ) const = 0;

    ////////////////////////////////////////////////////////////////////////////
    // Name:        WriteRadio
    // Description: Requests that the supplied buffer be sent to the radio
    //              module
    // Parameters:  pBuffer - pointer to buffer to send
    //              bufferSize - the number of bytes in the buffer
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    virtual void WriteRadio(
        const INT8U*    pBuffer,
        INT32U          bufferSize
        ) = 0;

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ReadRadio
    // Description: Requests to read data from the underlying radio module
    // Parameters:  pBuffer - pointer to buffer into which data will be placed.
    //                May be NULL if bufferSize is zero.  Must not be NULL if
    //                bufferSize is non-zero.
    //              bufferSize - the size of the buffer to fill.  If non-zero
    //                blocks until bufferSize bytes are read.  If zero, simply
    //                determines how many bytes are available.
    // Returns:     The number of bytes that can be retrieved without blocking
    ////////////////////////////////////////////////////////////////////////////
    virtual INT32U ReadRadio(
        INT8U*  pBuffer,
        INT32U  bufferSize
        ) = 0;

    ////////////////////////////////////////////////////////////////////////////
    // Name:        CancelRadio
    // Description: Requests that the underlying radio module is issued a cancel
    //              for its current operation.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    virtual void CancelRadio() = 0;

    ////////////////////////////////////////////////////////////////////////////
    // Name:        AbortRadio
    // Description: Requests that the underlying radio module is issued an abort
    //              for its current operation.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    virtual void AbortRadio() = 0;

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ResetRadio
    // Description: Requests that the underlying radio module be reset
    // Parameters:  resetType - the type of reset (i.e., soft, etc.)
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    virtual void ResetRadio(
        RFID_MAC_RESET_TYPE resetType
        ) = 0;

protected:
    ////////////////////////////////////////////////////////////////////////////
    // Name:        MacTransport
    // Description: Initializes a MAC transport object
    // Parameters:  transportHandle - the handle that is used to reference the
    //              radio module
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    explicit MacTransport(
        INT32U  transportHandle
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetTransportHandle
    // Description: Retrieves the underlying radio module transport handle
    // Parameters:  None
    // Returns:     The underlying radio module transport handle
    ////////////////////////////////////////////////////////////////////////////
    INT32U GetTransportHandle() const
    {
        return m_transportHandle;
    } // GetTransportHandle

// A macro for calculating the amount of padding
// l - the length in bytes
// a - the alignment in bytes (i.e., 4 = 4-byte or 32-bit alignment)
#define CALCULATE_PADDING(l, a) (((a)-((l)%(a)))%(a))
#define CALCULATE_32BIT_PADDING(l) CALCULATE_PADDING(l, 4)

private:
    // The underlying radio module transport handle
    INT32U m_transportHandle;
}; // class MacTransport

} // namespace rfid

#endif // #ifndef MAC_TRANSPORT_H_INCLUDED
