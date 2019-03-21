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
 * $Id: mac.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header presents the interface for the class that is a used to
 *     to represent the MAC's external register interface.
 *     
 *
 *****************************************************************************
 */

#ifndef MAC_H_INCLUDED
#define MAC_H_INCLUDED

#include <memory>
#include "rfid_platform_types.h"
#include "rfid_structs.h"
#include "rfid_constants.h"
#include "mac_transport.h"

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: Mac
//
// Description: This class is used by the radio to represent the interface
//    (i.e., read register, etc.) that is supported by the MAC.
////////////////////////////////////////////////////////////////////////////////
class Mac
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name:        Mac
    // Description: Initializes a Mac object.
    // Parameters:  pTransport - a pointer to the MAC transport object this Mac
    //              object will use
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    explicit Mac(
        std::auto_ptr<MacTransport> pTransport
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ~Mac
    // Description: Destroys a Mac object.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    ~Mac();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetDriverVersion
    // Description: Retrieves the driver version for the transport driver that
    //              is being used by the Mac object.
    // Parameters:  pVersion - pointer to an RFID_VERSION structure that upon
    //              return will be filled in with the transport driver version
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void GetDriverVersion(
        RFID_VERSION* pVersion
        ) const;

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Reset
    // Description: Tells the Mac to reset.
    // Parameters:  resetType - the type (e.g., soft) of reset that will be
    //              performed by the Mac.
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Reset(
        RFID_MAC_RESET_TYPE resetType
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        CancelOperation
    // Description: Tells the Mac to cancel the currently-executing operation.
    //              Unconsumed data is retained.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void CancelOperation();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        AbortOperation
    // Description: Tells the Mac to abort the currently-executing operation.
    //              Unconsumed data is discarded.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void AbortOperation();

    ////////////////////////////////////////////////////////////////////////////////
    // Name:        Mac::SendData
    // Description: Sends a raw data buffer to the MAC
    // Parameters:  pBuffer - pointer to buffer with data to send.
    //              Must not be NULL.
    //              pBufferSize - the number of bytes to send
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////////
    void SendData(
        const INT8U*    pBuffer,
        INT32U          bufferSize
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        RetrieveData
    // Description: Request to retrieve raw data from the Mac object.
    // Parameters:  pBuffer - pointer to buffer where data is to be placed.
    //              Must not be NULL.
    //              pBufferSize - the size of the buffer to fill
    // Returns:     The number of bytes that can be retrieved without blocking
    ////////////////////////////////////////////////////////////////////////////
    INT32U RetrieveData(
        INT8U*  pBuffer,
        INT32U  bufferSize
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        WriteRegister
    // Description: Requests that the value be written to the MAC's register.
    //              This function will ensure that the data is converted to the
    //              correct endianness for the MAC processor.
    // Parameters:  registerAddress - the register to write
    //              value - the value to write to the register
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void WriteRegister(
        INT16U  registerAddress,
        INT32U  value
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ReadRegister
    // Description: Requests that the value be read from the MAC's register.
    //              This function will ensure that the data value is converted
    //              from the endianness of the MAC processor to the host
    //              processor.
    // Parameters:  registerAddress - the register to read
    // Returns:     The value for the MAC register
    ////////////////////////////////////////////////////////////////////////////
    INT32U ReadRegister(
        INT16U  registerAddress
        );

private:
    // A pointer to the Mac objects' underlying transport object
    const std::auto_ptr<MacTransport>   m_pTransport;
    // The maximum buffer size supported by the underlying transport
    INT32U                              m_maxBufferSize;
    // The maximum packet size supported by the underlying transport
    INT32U                              m_maxPacketSize;
    // The driver version for the underlying transport driver
    RFID_VERSION                        m_driverVersion;

    // Don't want MAC objects being copied
    Mac(const Mac&);
    const Mac& operator = (const Mac&);
}; // class Mac

} // namespace rfid

#endif // #ifndef MAC_H_INCLUDED
