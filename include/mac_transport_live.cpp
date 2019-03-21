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
 * $Id: mac_transport_live.cpp 65946 2010-11-08 00:41:15Z dshaheen $
 * 
 * Description:
 *     This file contains the implementation for the live MAC transport.
 *     
 *
 *****************************************************************************
 */

#include <memory>
#include <vector>
#include <assert.h>
#include "mac_transport_live.h"
#include "mac.h"
#include "radio.h"
#include "translib.h"
#include "compat_error.h"
#include "compat_time.h"
#include "rfid_exceptions.h"
#include "auto_lock_compat.h"

#include "rfid_extern.h"

namespace rfid
{

// Typedefs that make code a little cleaner
typedef std::vector<TransHandle>  HandleList;
typedef std::vector<INT8U>        SerialNumber;
typedef std::vector<SerialNumber> SerialNumberList;
typedef std::vector<RFID_VERSION> DriverVersionList;

////////////////////////////////////////////////////////////////////////////////
// Name:        MacTransportLive
// Description: Initializes a live MAC transport object
////////////////////////////////////////////////////////////////////////////////
MacTransportLive::MacTransportLive(
    INT32U transportHandle
    ) :
    MacTransport(transportHandle)
{
    // Attempt to open the radio
    TransStatus status = RfTrans_OpenRadio(transportHandle);
    switch (status)
    {
        case CPL_SUCCESS:
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_DEBUG,
                "%s: Radio 0x%.8x opened succesfully\n",
                __FUNCTION__,
                transportHandle);
            break;
        case CPL_ERROR_BUSY:
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Radio 0x%.8x already opened\n",
                __FUNCTION__,
                transportHandle);
            throw RfidErrorException(RFID_ERROR_ALREADY_OPEN, __FUNCTION__);
            break;
        case CPL_ERROR_NOTFOUND:
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Radio 0x%.8x does not exist\n",
                __FUNCTION__,
                transportHandle);
            throw RfidErrorException(RFID_ERROR_NO_SUCH_RADIO, __FUNCTION__);
            break;
        case CPL_ERROR_DEVICEGONE:
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Radio 0x%.8x has been detached\n",
                __FUNCTION__,
                transportHandle);
            throw RfidErrorException(RFID_ERROR_RADIO_NOT_PRESENT, __FUNCTION__);
            break;
        default:
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Attempting to open radio 0x%.8x caused an unexpected "
                "error (0x%.8x)\n",
                __FUNCTION__,
                transportHandle,
                status);
            throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            break;
    } // switch (status)

    // Wrap the transport handle so that it is automatically cleaned up upon
    // object destruction
    m_transportHandleWrapper.Assume(transportHandle);
} // MacTransportLive::MacTransportLive

////////////////////////////////////////////////////////////////////////////////
// Name:        ~MacTransportLive
// Description: Cleans up the MAC transport object.
////////////////////////////////////////////////////////////////////////////////
MacTransportLive::~MacTransportLive()
{
} // MacTransportLive::~MacTransportLive

////////////////////////////////////////////////////////////////////////////////
// Name:        GetTransportCharacteristics
// Description: Retrieves the transport characteristics for the underlying
//              transport.
////////////////////////////////////////////////////////////////////////////////
void MacTransportLive::GetTransportCharacteristics(
    RFID_VERSION*   pDriverVersion,
    INT32U*         pMaxBufferSize,
    INT32U*         pMaxPacketSize
    ) const
{
    MacTransportLive::GetTransportCharacteristics(this->GetTransportHandle(),
                                                  pDriverVersion,
                                                  pMaxBufferSize,
                                                  pMaxPacketSize);
} // MacTransportLive::GetTransportCharacteristics

////////////////////////////////////////////////////////////////////////////////
// Name:        WriteRadio
// Description: Requests that the supplied buffer be sent to the radio
//              module
////////////////////////////////////////////////////////////////////////////////
void MacTransportLive::WriteRadio(
    const INT8U*    pBuffer,
    INT32U          bufferSize
    )
{
    assert(pBuffer != NULL);

    // Let the transport library write to the radio and verify that it worked
    TransStatus status = RfTrans_WriteRadio(this->GetTransportHandle(),
                                            const_cast<INT8U *>(pBuffer),
                                            bufferSize);
    switch (status)
    {
        case CPL_SUCCESS:           
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_DEBUG,
                "%s: Wrote %u bytes to radio 0x%.8x\n",
                __FUNCTION__,
                bufferSize,
                this->GetTransportHandle());
            break;
        case CPL_ERROR_ACCESSDENIED:
        case CPL_ERROR_NOTFOUND:    
        case CPL_ERROR_DEVICEGONE:  
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Write to radio 0x%.8x failed because it has been "
                "detached\n",
                __FUNCTION__,
                this->GetTransportHandle());
            throw RfidErrorException(RFID_ERROR_RADIO_NOT_PRESENT, __FUNCTION__);
            break;
        case CPL_WARN_CANCELLED:    
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Write to radio 0x%.8x failed because of a cancel\n",
                __FUNCTION__,
                this->GetTransportHandle());
            throw RfidErrorException(RFID_ERROR_OPERATION_CANCELLED, __FUNCTION__);
            break;
        default:                    
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Write to radio 0x%.8x returned an unexpected error "
                "(0x%.8x)\n",
                __FUNCTION__,
                this->GetTransportHandle(),
                status);
            throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            break;
    }
} // MacTransportLive::WriteRadio

////////////////////////////////////////////////////////////////////////////////
// Name:        ReadRadio
// Description: Requests to read data from the underlying radio module
////////////////////////////////////////////////////////////////////////////////
INT32U MacTransportLive::ReadRadio(
    INT8U*  pBuffer,
    INT32U  bufferSize
    )
{
    INT32U bytesAvailable;

    assert(!bufferSize || (NULL != pBuffer));

    // If the caller only cares about the number of bytes available, tell the
    // caller how many bytes are available in the transport layer plus what
    // has already been cached.
    if (!bufferSize)
    {
        bytesAvailable = m_cache.BytesUsed() + this->RawReadRadio(NULL, 0);
    }
    // Otherwise, if the request can be satisfied by the cache, remove
    // the bytes from the cache
    else if (m_cache.BytesUsed() >= bufferSize)
    {
        bytesAvailable = m_cache.Remove(pBuffer, bufferSize);
    }
    // Otherwise, we need to get bytes from the transport layer in order
    // to satisfy the request
    else
    {
        // First copy whatever we can from the cache
        INT32U  cachedBytes = m_cache.BytesUsed();
        if (cachedBytes)
        {
            m_cache.Remove(pBuffer, cachedBytes);
        }

        // Now go ahead and read the remaining bytes
        bytesAvailable =
            this->RawReadRadio(
                pBuffer + cachedBytes,
                bufferSize - cachedBytes);

        // If the driver has bytes available and the cache has room, then
        // If the driver has bytes available, then prefetch them in order to
        // satisfy a subsequent request
        if (bytesAvailable)
        {
            // Retrieve the lesser of the number of bytes available or the
            // remaining room in the cache into our cache
            INT8U   buffer[RING_SIZE];
            INT32U  bytesToRead = m_cache.BytesFree();
            if (bytesAvailable < bytesToRead)
            {
                bytesToRead = bytesAvailable;
            }

            // The number of bytes available is now the number in the
            // cache plus whatever the transport layer indicates are 
            // remaining
            bytesAvailable = 
                this->RawReadRadio(buffer, bytesToRead) + bytesToRead;
            m_cache.Add(buffer, bytesToRead);
        }
    }

    return bytesAvailable;
} // MacTransportLive::ReadRadio

////////////////////////////////////////////////////////////////////////////////
// Name:        CancelRadio
// Description: Requests that the underlying radio module is issued a cancel
//              for its current operation.
////////////////////////////////////////////////////////////////////////////////
void MacTransportLive::CancelRadio()
{
    // Let the transport library issue the cancel and verify it worked
    TransStatus status = RfTrans_CancelRadio(this->GetTransportHandle(), 0);
    switch (status)
    {
        case CPL_SUCCESS:           
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_DEBUG,
                "%s: Cancel of radio 0x%.8x successful\n",
                __FUNCTION__,
                this->GetTransportHandle());
            break;
        case CPL_ERROR_NOTFOUND:    
        case CPL_ERROR_DEVICEGONE:  
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Cancel of radio 0x%.8x failed as radio has been "
                "detached\n",
                __FUNCTION__,
                this->GetTransportHandle());
            throw RfidErrorException(RFID_ERROR_RADIO_NOT_PRESENT, __FUNCTION__);
            break;
        case CPL_WARN_CANCELLED:
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Cancel of radio 0x%.8x failed as radio did not respond\n",
                __FUNCTION__,
                this->GetTransportHandle());
            throw RfidErrorException(RFID_ERROR_RADIO_NOT_RESPONDING, __FUNCTION__);
            break;
        default:                    
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Cancel of radio 0x%.8x returned an unexpected error "
                "(0x%.8x)\n",
                __FUNCTION__,
                this->GetTransportHandle(),
                status);
            throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            break;
    } // switch (status)
} // MacTransportLive::CancelRadio

////////////////////////////////////////////////////////////////////////////////
// Name:        AbortRadio
// Description: Requests that the underlying radio module is issued an abort
//              for its current operation.
////////////////////////////////////////////////////////////////////////////////
void MacTransportLive::AbortRadio()
{
    // Clear out the cache
    m_cache.Clear();

    // Let the transport library issue the cancel and verify it worked
    TransStatus status =
        RfTrans_AbortRadio(this->GetTransportHandle());
    switch (status)
    {
        case CPL_SUCCESS:           
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_DEBUG,
                "%s: Abort of radio 0x%.8x successful\n",
                __FUNCTION__,
                this->GetTransportHandle());
            break;
        case CPL_ERROR_NOTFOUND:    
        case CPL_ERROR_DEVICEGONE:  
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Abort of radio 0x%.8x failed as radio has been detached\n",
                __FUNCTION__,
                this->GetTransportHandle());
            throw RfidErrorException(RFID_ERROR_RADIO_NOT_PRESENT, __FUNCTION__);
            break;
        case CPL_WARN_CANCELLED:
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Abort of radio 0x%.8x failed as radio did not respond\n",
                __FUNCTION__,
                this->GetTransportHandle());
            throw RfidErrorException(RFID_ERROR_RADIO_NOT_RESPONDING, __FUNCTION__);
            break;
        default:                    
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Cancel of radio 0x%.8x returned an unexpected error "
                "(0x%.8x)\n",
                __FUNCTION__,
                this->GetTransportHandle(),
                status);
            throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            break;
    } // switch (status)
} // MacTransportLive::AbortRadio

////////////////////////////////////////////////////////////////////////////////
// Name:        ResetRadio
// Description: Requests that the underlying radio module be reset
////////////////////////////////////////////////////////////////////////////////
void MacTransportLive::ResetRadio(
    RFID_MAC_RESET_TYPE resetType
    )
{
    INT32U transResetType;

    // translate from RFID Lib Reset Type to transport lib reset type
    if (RFID_MAC_RESET_TYPE_SOFT == resetType)
    {
        transResetType = RESETRADIO_SOFT;
    }
    else if (RFID_MAC_RESET_TYPE_TO_BOOTLOADER == resetType)
    {
        transResetType = RESETRADIO_TO_BL;
    }
    else
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Radio 0x%.8x reset invalid parameter error "
            "(0x%.8x)\n",
            __FUNCTION__,
            this->GetTransportHandle(),
            resetType);
        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    // Tell the transport library to reset the MAC
    TransStatus status = RfTrans_ResetRadio(this->GetTransportHandle(), transResetType);
    switch (status)
    {
        case CPL_SUCCESS:           
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_DEBUG,
                "%s: Radio 0x%.8x reset successful\n",
                __FUNCTION__,
                this->GetTransportHandle());
            break;
        case CPL_ERROR_NOTFOUND:    
        case CPL_ERROR_DEVICEGONE:  
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Radio 0x%.8x reset failed as radio has been detached\n",
                __FUNCTION__,
                this->GetTransportHandle());
            throw RfidErrorException(RFID_ERROR_RADIO_NOT_PRESENT, __FUNCTION__);
            break;

        case CPL_ERROR_INVALID_FUNC:  
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Radio 0x%.8x reset failed invalid function (driver probably needs to be updated\n",
                __FUNCTION__,
                this->GetTransportHandle());
            throw RfidErrorException(RFID_ERROR_DRIVER_MISMATCH, __FUNCTION__);
            break;
            
        default:                    
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Radio 0x%.8x reset returned an unexpected error "
                "(0x%.8x)\n",
                __FUNCTION__,
                this->GetTransportHandle(),
                status);
            throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            break;
    } // switch (status)
} // MacTransportLive::ResetRadio

////////////////////////////////////////////////////////////////////////////////
// Name:        EnumerateAttachedRadios
// Description: Requests that all attached radio modules be enumerated
////////////////////////////////////////////////////////////////////////////////
void MacTransportLive::EnumerateAttachedRadios(
    RFID_RADIO_ENUM*    pEnum
    )
{
    // The result status from the transport library
    TransStatus        status;
    // A list of radio handles for the radios known by the transport library
    HandleList         handles;
    // A list of serial numbers for the radios known by the transport library
    SerialNumberList   serialNumbers;
    // A list of driver versions for the radios known by the transport library
    DriverVersionList  driverVersions;
    // Context information for the transport library radio enumeration
    RadioEnumContext_t context;
    // The action to perform while enumerating
    EnumAction         action = EA_FirstRadio;
    // The radio handle returned by the transport library enumeration function
    TransHandle        handle;
    // The serial number returned by the transport librar enumeration function
    SerialNumber       serialNumber;
    // The length, in bytes, of the serial number
    INT32U             serialNumberLength;
    // The size required for the radio enumeraion structure supplied by the
    // caller
    INT32U             requiredSize = sizeof(RFID_RADIO_ENUM);
    // A pointer to the radio module serial number
    INT8U*             pSerialNumber;

    // Iterate through the radios and get the handles and serial numbers
    // until the end of list is encountered
    do
    {
        // Set the serial number pointer and the size before calling to
        // enumerate the radio
        serialNumberLength = serialNumber.size();
        pSerialNumber      = (serialNumberLength ? &serialNumber[0] : NULL);

        // First determine the size of the radio's serial number
        status = RfTrans_EnumerateRadios(&handle,
                                         pSerialNumber,
                                         &serialNumberLength,
                                         &context,
                                         action);

        // Based upon the status of the call, do the right thing
        switch (status)
        {
            // End of the list...this is okay as it is our terminating condition
            case CPL_WARN_ENDOFLIST:
            {
                break;
            } // case CPL_WARN_ENDOFLIST
            // Buffer is not large enough...make it larger and try again
            case CPL_ERROR_BUFFERTOOSMALL:
            {
                serialNumber.resize(serialNumberLength);
                action = EA_SameRadio;
                break;
            } // case CPL_ERROR_BUFFERTOOSMALL
            // Radio is gone...then we just go on
            case CPL_ERROR_DEVICEGONE:
            {
                action = EA_NextRadio;
                break;
            } // case CPL_ERROR_DEVICEGONE
            // Successfully enumerated radio...get the information
            case CPL_SUCCESS:
            {
                // Resize the serial number so that if there is extra at the
                // end, we chop if off
                if (serialNumberLength < serialNumber.size())
                {
                    serialNumber.resize(serialNumberLength);
                }

                try
                {
                    // Get the driver version number for the radio
                    RFID_VERSION driverVersion;
                    MacTransportLive::GetTransportCharacteristics(
                        handle,
                        &driverVersion);

                    // Copy the handle, serial number, and driver version
                    handles.push_back(handle);
                    serialNumbers.push_back(serialNumber);
                    driverVersions.push_back(driverVersion);

                    // Increment the required size to account for the radio
                    // info pointer array entry, the radio info structure,
                    // serial number, and any necessary padding
                    requiredSize +=
                        (sizeof(RFID_RADIO_INFO *)  +
                         sizeof(RFID_RADIO_INFO)    +
                         serialNumberLength         +
                         CALCULATE_32BIT_PADDING(serialNumberLength));
                }
                catch (std::bad_alloc)
                {
                    throw;
                }
                catch (...)
                {
                    // Intentionally left blank.  If we get an error while
                    // getting the transport characteristics, we will simply
                    // not add this radio to the list.
                }

                // Move on to the next radio
                action = EA_NextRadio;
                break;
            } // case CPL_SUCCESS
            // A status that results in an unrecoverable error
            default:
            {
                g_pTracer->PrintMessage(
                    Tracer::RFID_LOG_SEVERITY_ERROR,
                    "%s: Enumerating radios returned an unexpected error "
                    "(0x%.8x)\n",
                    __FUNCTION__,
                    status);
                throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
                break;
            }
        } // switch (status)
    }
    while (CPL_WARN_ENDOFLIST != status);

    // If the buffer is not large enough...
    if (pEnum->totalLength < requiredSize)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_DEBUG,
            "%s: Caller's buffer is %u bytes, but needs to be %u bytes\n",
            __FUNCTION__,
            pEnum->totalLength,
            requiredSize);

        pEnum->totalLength = requiredSize;
        throw RfidErrorException(RFID_ERROR_BUFFER_TOO_SMALL, __FUNCTION__);
    }

    // Clear out the enumeration structure
    memset(pEnum, 0, requiredSize);

    // Set up the enumeration buffer for the application
    pEnum->length      = sizeof(RFID_RADIO_ENUM);
    pEnum->totalLength = requiredSize;
    pEnum->countRadios = handles.size();
    pEnum->ppRadioInfo = 
        (pEnum->countRadios ? reinterpret_cast<RFID_RADIO_INFO **>(pEnum + 1) :
                              NULL);

    g_pTracer->PrintMessage(
        Tracer::RFID_LOG_SEVERITY_DEBUG,
        "%s: There are %u radios attached to the system\n",
        __FUNCTION__,
        pEnum->countRadios);

    // Set the first radio information structure to fall at the end of the 
    // enumeration structure (including the pointer list)
    INT8U* pBuffer = reinterpret_cast<INT8U *>(pEnum + 1) +
                     (pEnum->countRadios * sizeof(RFID_RADIO_INFO *));

    for (INT32U index = 0; index < pEnum->countRadios; ++index)
    {
        // Set the radio info pointer
        pEnum->ppRadioInfo[index] =
            reinterpret_cast<RFID_RADIO_INFO *>(pBuffer);

        // Set up the radio information fixed-length fields first
        RFID_RADIO_INFO info;
        info.cookie        = handles[index];
        info.driverVersion = driverVersions[index];
        info.idLength      = serialNumbers[index].size();
        info.pUniqueId     = pBuffer + sizeof(RFID_RADIO_INFO);

        INT32U serialNumberPad = CALCULATE_32BIT_PADDING(info.idLength);

        info.length        = sizeof(RFID_RADIO_INFO) +
                             info.idLength           +
                             serialNumberPad;

        // Copy the fixed-length fields to the buffer
        memcpy(pBuffer, &info, sizeof(info));

        // Move the buffer pointer to the end of the fixed fields and copy the
        // serial number
        pBuffer += sizeof(info);
        memcpy(pBuffer, &(serialNumbers[index][0]), info.idLength);

        // Move the buffer to the next radio information structure
        pBuffer += (info.idLength + serialNumberPad);
    }
} // MacTransportLive::EnumerateAttachedRadios

////////////////////////////////////////////////////////////////////////////////
// Name:        OpenRadio
// Description: Requests that a radio module be opened.
////////////////////////////////////////////////////////////////////////////////
std::auto_ptr<Radio> MacTransportLive::OpenRadio(
    INT32U  transportHandle
    )
{
    // Create a radio.  This requires a MAC, which requires a MAC transport.
    // Obviously, since we have to provide the lower-level object to the
    // higher-level one, we need to create them from bottom up.
    std::auto_ptr<MacTransport> pMacTransport(
                                    new MacTransportLive(
                                        transportHandle));
    std::auto_ptr<Mac>          pMac(new Mac(pMacTransport));
    std::auto_ptr<Radio>        pRadio(new Radio(pMac));

    return pRadio;
} // MacTransportLive::OpenRadio

////////////////////////////////////////////////////////////////////////////////
// Name:        GetTransportCharacteristics
// Description: The real implementation of the get characteristics function.
//              This one actually calls into the transport library.
////////////////////////////////////////////////////////////////////////////////
void MacTransportLive::GetTransportCharacteristics(
    INT32U          transportHandle,
    RFID_VERSION*   pDriverVersion,
    INT32U*         pMaxBufferSize,
    INT32U*         pMaxPacketSize
    )
{
    // The size of the transport characteristics buffer
    INT32U             bufferSize = 0;
    // The transport characteristics buffer
    std::vector<INT8U> buffer(bufferSize);
    // The result of the transport library call
    TransStatus        status;
    // A pointer to the transport characteristics buffer
    INT8U*             pBuffer = NULL;

    // Try to get the transport characteristics for this radio.  Repeat if the
    // error tells us that the buffer is too small.
    while (CPL_ERROR_BUFFERTOOSMALL == 
           (status = RfTrans_GetRadioTransportCharacteristics(
                        transportHandle,
                        reinterpret_cast<TransportCharacteristics *>(pBuffer),
                        &bufferSize)))
    {
        buffer.resize(bufferSize);
        pBuffer = &buffer[0];
    }

    // If we stopped for any reason other than success, throw error
    if (CPL_SUCCESS != status)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Retrieving transport characteristics for radio 0x%.8x "
            "returned an unexpected error (0x%.8x)\n",
            __FUNCTION__,
            transportHandle,
            status);
        throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
    }

    TransportCharacteristics* pCharacteristics = 
        reinterpret_cast<TransportCharacteristics *>(&buffer[0]);

    // Set the pertinent data
    pDriverVersion->major = pCharacteristics->transportVersion.major;
    pDriverVersion->minor = pCharacteristics->transportVersion.minor;
    pDriverVersion->maintenance = pCharacteristics->transportVersion.maintenance;
    pDriverVersion->release = pCharacteristics->transportVersion.release;
    if (NULL != pMaxBufferSize) 
    {
        *pMaxBufferSize = pCharacteristics->maxTransfer;
    }
    if (NULL != pMaxPacketSize)
    {
        *pMaxPacketSize = pCharacteristics->maxPacket;
    }
} // MacTransportLive::GetTransportCharacteristics

////////////////////////////////////////////////////////////////////////////////
// Name:        RawReadRadio
// Description: Requests to read data from the underlying radio module
////////////////////////////////////////////////////////////////////////////////
INT32U MacTransportLive::RawReadRadio(
    INT8U*  pBuffer,
    INT32U  bufferSize
    )
{
    INT32U bytesAvailable;
    INT32U bytesRetrieved = bufferSize;

    assert(!bufferSize || (NULL != pBuffer));

    // Let the transport library perform the read and verify that it worked
    TransStatus status = RfTrans_ReadRadio(
        this->GetTransportHandle(),
        pBuffer,
        &bytesRetrieved,
        &bytesAvailable,
        0);
    assert(bytesRetrieved == bufferSize);

    switch (status)
    {
        case CPL_SUCCESS:           
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_DEBUG,
                "%s: Read %u bytes from radio 0x%.8x.  %u bytes still "
                "available\n",
                __FUNCTION__,
                bytesRetrieved,
                this->GetTransportHandle(),
                bytesAvailable);
            break;
        case CPL_ERROR_ACCESSDENIED:
        case CPL_ERROR_NOTFOUND:    
        case CPL_ERROR_DEVICEGONE:  
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Read from radio 0x%.8x failed because it has been "
                "detached\n",
                __FUNCTION__,
                this->GetTransportHandle());
            throw RfidErrorException(RFID_ERROR_RADIO_NOT_PRESENT, __FUNCTION__);
            break;
        case CPL_WARN_CANCELLED:    
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Read from radio 0x%.8x failed because of a cancel\n",
                __FUNCTION__,
                this->GetTransportHandle());
            throw RfidErrorException(RFID_ERROR_OPERATION_CANCELLED, __FUNCTION__);
            break;
        case CPL_ERROR_RXOVERFLOW:
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_WARNING,
                "%s: Read from radio 0x%.8x failed because of a RX overflow\n",
                __FUNCTION__,
                this->GetTransportHandle());
            try
            {
                this->AbortRadio();
            }
            catch (...)
            {
            }
            throw RfidErrorException(RFID_ERROR_RECEIVE_OVERFLOW, __FUNCTION__);
            break;
        default:       
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Read from radio 0x%.8x returned an unexpected error "
                "(0x%.8x)\n",
                __FUNCTION__,
                this->GetTransportHandle(),
                status);
            throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            break;
    }

    return bytesAvailable;
} // MacTransportLive::RawReadRadio

} // namespace rfid
