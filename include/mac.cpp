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
 * $Id: mac.cpp 60023 2009-09-24 23:59:22Z dshaheen $
 * 
 * Description:
 *     This file contains the implementation for MAC.
 *     
 *
 *****************************************************************************
 */

#include <vector>
#include <memory.h>
#include <assert.h>
#include "mac.h"
#include "mac_transport.h"
#include "compat_lib.h"
#include "hostpkts.h"
#include "rfid_exceptions.h"
#include "rfid_extern.h"
#include "tracer.h"

namespace rfid
{
////////////////////////////////////////////////////////////////////////////////
// Name:        Mac::Mac
// Description: Initializes a Mac object.
////////////////////////////////////////////////////////////////////////////////
Mac::Mac(
    std::auto_ptr<MacTransport> pTransport
    ) :
    m_pTransport(pTransport)
{
    assert(NULL != m_pTransport.get());

    // Get the transport characteristics
    m_pTransport->GetTransportCharacteristics(
        &m_driverVersion,
        &m_maxBufferSize,
        &m_maxPacketSize);
} // Mac::Mac

////////////////////////////////////////////////////////////////////////////////
// Name:        Mac::~Mac
// Description: Destroys a Mac object.
// Parameters:  None
// Returns:     Nothing
////////////////////////////////////////////////////////////////////////////////
Mac::~Mac()
{
} // Mac::~Mac

////////////////////////////////////////////////////////////////////////////////
// Name:        Mac::GetDriverVersion
// Description: Retrieves the driver version for the transport driver that
//              is being used by the Mac object.
////////////////////////////////////////////////////////////////////////////////
void Mac::GetDriverVersion(
    RFID_VERSION* pVersion
    ) const
{
    assert(NULL != pVersion);

    // Return cached driver version
    memcpy(pVersion, &m_driverVersion, sizeof(RFID_VERSION));
} // Mac::Mac

////////////////////////////////////////////////////////////////////////////////
// Name:        Reset
// Description: Tells the Mac to reset.
////////////////////////////////////////////////////////////////////////////////
void Mac::Reset(
    RFID_MAC_RESET_TYPE resetType
    )
{
    // Instruct transport driver to reset the MAC
    m_pTransport->ResetRadio(resetType);
} // Mac::Reset

////////////////////////////////////////////////////////////////////////////////
// Name:        Mac::CancelOperation
// Description: Tells the Mac to cancel the currently-executing operation.
////////////////////////////////////////////////////////////////////////////////
void Mac::CancelOperation()
{
    // Instruct transport driver to issue cancel for radio
    m_pTransport->CancelRadio();
} // Mac::CancelOperation

////////////////////////////////////////////////////////////////////////////////
// Name:        Mac::AbortOperation
// Description: Tells the Mac to abort the currently-executing operation.
////////////////////////////////////////////////////////////////////////////////
void Mac::AbortOperation()
{
    // Instruct transport driver to issue cancel for radio
    m_pTransport->AbortRadio();
} // Mac::AbortOperation

////////////////////////////////////////////////////////////////////////////////
// Name:        Mac::SendData
// Description: Sends a raw data buffer to the MAC
////////////////////////////////////////////////////////////////////////////////
void Mac::SendData(
    const INT8U*    pBuffer,
    INT32U          bufferSize
    )
{
    // Instruct transport driver to send data from the radio module
    m_pTransport->WriteRadio(pBuffer, bufferSize);
} // Mac::SendData

////////////////////////////////////////////////////////////////////////////////
// Name:        Mac::RetrieveData
// Description: Request to retrieve raw data from the Mac object.
////////////////////////////////////////////////////////////////////////////////
INT32U Mac::RetrieveData(
    INT8U*  pBuffer,
    INT32U  bufferSize
    )
{
    // Instruct transport driver to read response data from the radio module
    return m_pTransport->ReadRadio(pBuffer, bufferSize);
} // Mac::RetrieveData

////////////////////////////////////////////////////////////////////////////////
// Name:        Mac::WriteRegister
// Description: Requests that the value be written to the MAC's register
////////////////////////////////////////////////////////////////////////////////
void Mac::WriteRegister(
    INT16U  registerAddress,
    INT32U  value
    )
{
    g_pTracer->PrintMessage(
        Tracer::RFID_LOG_SEVERITY_DEBUG,
        "%s: Write 0x%.8x to MAC virtual register 0x%.4x\n",
        __FUNCTION__,
        value,
        registerAddress);

    // Send the register write request to the MAC
    host_reg_req request =
        {
            CPL_HostToMac16(HOST_REG_REQ_ACCESS_WRITE),
            CPL_HostToMac16(registerAddress),
            CPL_HostToMac32(value)
        };
    m_pTransport->WriteRadio(reinterpret_cast<INT8U *>(&request),
                             sizeof(request));
} // Mac::WriteRegister

////////////////////////////////////////////////////////////////////////////////
// Name:        Mac::ReadRegister
// Description: Requests that the value be read from the MAC's register
////////////////////////////////////////////////////////////////////////////////
INT32U Mac::ReadRegister(
    INT16U  registerAddress
    )
{
    g_pTracer->PrintMessage(
        Tracer::RFID_LOG_SEVERITY_DEBUG,
        "%s: Read MAC virtual register 0x%.4x\n",
        __FUNCTION__,
        registerAddress);

    // Send the register read request to the MAC
    host_reg_req request =
        {
            CPL_HostToMac16(HOST_REG_REQ_ACCESS_READ),
            CPL_HostToMac16(registerAddress),
            0
        };
    m_pTransport->WriteRadio(reinterpret_cast<INT8U *>(&request),
                             sizeof(request));

    host_reg_resp response;
    INT8U*        pBuffer    = reinterpret_cast<INT8U *>(&response);
    INT32U        bufferSize = sizeof(response);

    // Retrieve the register read response.
    m_pTransport->ReadRadio(pBuffer, bufferSize);

    // If the register returned is not the register read, then we have a
    // problem (state is out of sync with MAC, requested an invalid register,
    // etc.)
    if (CPL_MacToHost16(response.reg_addr) != registerAddress)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Requested MAC virtual register 0x%.4x, but we received "
            "register 0x%.4x\n",
            __FUNCTION__,
            registerAddress,
            CPL_MacToHost16(response.reg_addr));
        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    g_pTracer->PrintMessage(
        Tracer::RFID_LOG_SEVERITY_DEBUG,
        "%s: MAC virtual register 0x%.4x returned value 0x%.8x\n",
        __FUNCTION__,
        registerAddress,
        CPL_MacToHost32(response.reg_data));

    // Convert to host format and return
    return CPL_MacToHost32(response.reg_data);
} // Mac::ReadRegister

} // namespace rfid
