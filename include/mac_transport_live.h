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
 * $Id: mac_transport_live.h 60310 2009-10-21 18:43:13Z dshaheen $
 * 
 * Description:
 *     This header presents the interface for the class that is a used to
 *     to communicate with a radio's MAC.  This particular MAC interface class
 *     is for the live MAC.
 *     
 *
 *****************************************************************************
 */

#ifndef MAC_TRANSPORT_LIVE_H_INCLUDED
#define MAC_TRANSPORT_LIVE_H_INCLUDED

#include "mac_transport.h"
#include "radio.h"
#include "compat_thread.h"
#include "auto_handle_transport.h"

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: MacTransportLive
//
// Description: This class is used to provide an abstraction of the MAC 
//     transport for live MACs
////////////////////////////////////////////////////////////////////////////////
class MacTransportLive : public MacTransport
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name:        ~MacTransportLive
    // Description: Cleans up the MAC transport object.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    ~MacTransportLive();

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
    void GetTransportCharacteristics(
        RFID_VERSION*   pDriverVersion,
        INT32U*         pMaxBufferSize,
        INT32U*         pMaxPacketSize
        ) const;

    ////////////////////////////////////////////////////////////////////////////
    // Name:        WriteRadio
    // Description: Requests that the supplied buffer be sent to the radio
    //              module
    // Parameters:  pBuffer - pointer to buffer to send.  Must not be NULL.
    //              bufferSize - the number of bytes in the buffer
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void WriteRadio(
        const INT8U*    pBuffer,
        INT32U          bufferSize
        );

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
    INT32U ReadRadio(
        INT8U*  pBuffer,
        INT32U  bufferSize
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        CancelRadio
    // Description: Requests that the underlying radio module is issued a cancel
    //              for its current operation.
    // Parameters:  shouldCancelRead - a flag that indicates if any unconsumed
    //              read data for the radio should be discarded
    //              shouldCancelWrite - a flag that indicates if any unwritten
    //              write data for the radio should be discarded
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void CancelRadio();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        AbortRadio
    // Description: Requests that the underlying radio module is issued an abort
    //              for its current operation.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void AbortRadio();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ResetRadio
    // Description: Requests that the underlying radio module be reset
    // Parameters:  resetType - the type of reset (i.e., soft, etc.)
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ResetRadio(
        RFID_MAC_RESET_TYPE resetType
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        EnumerateAttachedRadios
    // Description: Requests that all attached radio modules be enumerated
    // Parameters:  pEnum - a pointer to an enumeration buffer that is to be
    //              filled in as per the RFID Radio Libary EAS
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    static void EnumerateAttachedRadios(
        RFID_RADIO_ENUM*    pEnum
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        OpenRadio
    // Description: Requests that a radio module be opened.
    // Parameters:  transportHandle - the handle/cookie that was returned in the
    //              enumeration data that corresponds to the radio module to
    //              open
    // Returns:     An auto_ptr-wrapped pointer to a new radio object.
    ////////////////////////////////////////////////////////////////////////////
    static std::auto_ptr<Radio> OpenRadio(
        INT32U  transportHandle
        );

private:
    // A wrapper around the transport handle so that it is automatically cleaned
    // up
    TransportRadioAutoHandle    m_transportHandleWrapper;

    enum { RING_SIZE = 2048 };

    ////////////////////////////////////////////////////////////////////////////
    // Name: RingBuffer
    //
    // Description: This class provides a simple implementation of a ring buffer
    //   that will be used to cache data from the underlying transport layer.
    ////////////////////////////////////////////////////////////////////////////
    class RingBuffer
    {
    public:
        ////////////////////////////////////////////////////////////////////////////
        // Name:        RingBuffer
        // Description: Creates the ring buffer
        // Parameters:  None
        // Returns:     Nothing
        ////////////////////////////////////////////////////////////////////////////
        RingBuffer() :
            m_used(0),
            m_head(0),
            m_tail(0)
        {
        } // RingBuffer::RingBuffer

        ////////////////////////////////////////////////////////////////////////////
        // Name:        Empty
        // Description: Returns whether or not ring buffer is empty
        // Parameters:  None
        // Returns:     0 - ring buffer is not empty, !0 - ring buffer is empty
        ////////////////////////////////////////////////////////////////////////////
        BOOL32 Empty() const
        {
            return m_used == 0;
        } // RingBuffer::Empty

        ////////////////////////////////////////////////////////////////////////////
        // Name:        Full
        // Description: Returns whether or not ring buffer is full
        // Parameters:  None
        // Returns:     0 - ring buffer is not full, !0 - ring buffer is full
        ////////////////////////////////////////////////////////////////////////////
        BOOL32 Full() const
        {
            return m_used == RING_SIZE;
        } // RingBuffer::Full

        ////////////////////////////////////////////////////////////////////////////
        // Name:        BytesUsed
        // Description: Returns the number of bytes in the ring buffer
        // Parameters:  None
        // Returns:     The number of bytes in the ring buffer
        ////////////////////////////////////////////////////////////////////////////
        INT32U BytesUsed() const
        {
            return m_used;
        } // RingBuffer::BytesUsed

        ////////////////////////////////////////////////////////////////////////////
        // Name:        BytesFree
        // Description: Returns the number of free bytes available in the ring
        //              buffer
        // Parameters:  None
        // Returns:     The number of bytes free in the ring buffer
        ////////////////////////////////////////////////////////////////////////////
        INT32U BytesFree() const
        {
            return RING_SIZE - m_used;
        } // RingBuffer::BytesFree

        ////////////////////////////////////////////////////////////////////////////
        // Name:        Clear
        // Description: Empties out the ring buffer
        // Parameters:  None
        // Returns:     Nothing
        ////////////////////////////////////////////////////////////////////////////
        void Clear()
        {
            m_head = m_tail = m_used = 0;
        } // RingBuffer::Clear

        ////////////////////////////////////////////////////////////////////////////
        // Name:        Add
        // Description: Adds the bytes in the buffer provided to the ring buffer
        // Parameters:  pBuffer - pointer to bytes to add to ring buffer
        //              bufferSize - the number of bytes in buffer
        // Returns:     The number of free bytes in ring buffer after addition
        ////////////////////////////////////////////////////////////////////////////
        INT32U Add(
            const INT8U*    pBuffer,
            INT32U          bufferSize
            )
        {
            assert(bufferSize <= this->BytesFree());

            // If the bytes will extend beyond the end of the buffer...
            if ((m_tail + bufferSize) > RING_SIZE)
            {
                // Add however bytes necessary to get to end of array
                INT32U  appendSize = RING_SIZE - m_tail;
                memcpy(m_ring + m_tail, pBuffer, appendSize);

                // Now copy the remaining bytes to the front of the array
                INT32U  prependSize = bufferSize - appendSize;
                memcpy(m_ring, pBuffer + appendSize, prependSize);
            }
            else
            {
                // Simply copy bytes to array
                memcpy(m_ring + m_tail, pBuffer, bufferSize);
            }

            // Increment the number of bytes in ring buffer and adjust tail
            // to "point" to where next byte is to be copied
            m_used += bufferSize;
            m_tail = (m_tail + bufferSize) % RING_SIZE;

            return this->BytesFree();
        }

        ////////////////////////////////////////////////////////////////////////////
        // Name:        Remove
        // Description: Copies bytes from the ring buffer to buffer provided
        // Parameters:  pBuffer - pointer to buffer into which to put bytes
        //              bufferSize - the number of bytes to retrieve
        // Returns:     The number of bytes in the ring buffer after the removal
        ////////////////////////////////////////////////////////////////////////////
        INT32U Remove(
            INT8U*  pBuffer,
            INT32U  bufferSize
            )
        {
            assert(bufferSize <= this->BytesUsed());

            // If the bytes will extend beyond the end of the buffer...
            if ((m_head + bufferSize) > RING_SIZE)
            {
                // Copy the bytes up through the end of the array
                INT32U  firstCopy = RING_SIZE - m_head;
                memcpy(pBuffer, m_ring + m_head, firstCopy);

                // Copy the bytes from the front of the array
                INT32U  secondCopy = bufferSize - firstCopy;
                memcpy(pBuffer + firstCopy, m_ring, secondCopy);
            }
            else
            {
                // Simply copy bytes from array
                memcpy(pBuffer, m_ring + m_head, bufferSize);
            }

            // Decrement the number of bytes in ring buffer and update the head
            // to "point" to the next byte to consume
            m_used -= bufferSize;
            m_head = (m_head + bufferSize) % RING_SIZE;

            return this->BytesUsed();
        }

    private:
        INT8U           m_ring[RING_SIZE];  // The array that holds ring buffer
        INT32U          m_used;             // The number of bytes in buffer
        INT32U          m_head;             // The next byte to consume
        INT32U          m_tail;             // The next byte to fill
    } m_cache;  // A cache of the bytes from the transport layer
    
    ////////////////////////////////////////////////////////////////////////////
    // Name:        MacTransportLive
    // Description: Initializes a live MAC transport object
    // Parameters:  transportHandle - the handle that is used to reference the
    //              radio module
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    explicit MacTransportLive(
        INT32U transportHandle
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetTransportCharacteristics
    // Description: The real implementation of the get characteristics function.
    //              This one actually calls into the transport library.
    // Parameters:  transportHandle - the handle that is used to reference the
    //              radio module
    //              pDriverVersion - pointer to a structure that upon return
    //              will contain the transport driver version for the radio
    //              module
    //              pMaxBufferSize - pointer to a 32-bit unsigned integer that
    //              upon return will contain the maximum buffer size supported
    //              by the radio module's transport driver.  May be NULL if not
    //              needed.
    //              pMaxPacketSize - pointer to a 32-bit unsigned integer that
    //              upon return will contain the maximum packet size supported
    //              by the radio module's transport driver.  May be NULL if not
    //              needed.
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    static void GetTransportCharacteristics(
        INT32U          transportHandle,
        RFID_VERSION*   pDriverVersion,
        INT32U*         pMaxBufferSize = NULL,
        INT32U*         pMaxPacketSize = NULL
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        RawReadRadio
    // Description: Requests to read data from the underlying radio module
    // Parameters:  pBuffer - pointer to buffer into which data will be placed.
    //                May be NULL if bufferSize is zero.  Must not be NULL if
    //                bufferSize is non-zero.
    //              bufferSize - the size of the buffer to fill.  If non-zero
    //                blocks until bufferSize bytes are read.  If zero, simply
    //                determines how many bytes are available.
    // Returns:     The number of bytes that can be retrieved without blocking
    ////////////////////////////////////////////////////////////////////////////
    INT32U RawReadRadio(
        INT8U*  pBuffer,
        INT32U  bufferSize
        );

    // Prevent copying of live transport objects
    MacTransportLive(MacTransportLive&);
    const MacTransportLive& operator = (const MacTransportLive&);
}; // class MacTransportLive

} // namespace rfid

#endif // #ifndef MAC_TRANSPORT_LIVE_H_INCLUDED
