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
 * $Id: sample_utility.h 61529 2009-12-22 21:12:04Z dshaheen $
 * 
 * Description:
 *     Generic helper functions
 *     
 *
 *****************************************************************************
 */

#ifndef SAMPLE_UTILITY_H_INCLUDED
#define SAMPLE_UTILITY_H_INCLUDED

#include "rfid_types.h"

/* Context structure used by tag access sample applications to know when 
 * they have successesfully received the expected packets, and for access 
 * to the tag data on read access
 */
typedef struct
{
    INT8U   succesfulAccessPackets;
    INT8U*  pReadData;
} CONTEXT_PARMS;


/******************************************************************************
 * Name: PrintRadioUniqueId
 *
 * Description:
 *   Prints out the unique ID for a radio.  The unique ID is printed in hex and
 *   ASCII with an indent of 4 spaces.
 *
 * Parameters:
 *   idLength - length, in bytes, of the unique ID
 *   pUniqueId - pointer to buffer that contains the unique ID
 *
 * Returns:
 *   Nothing
 ******************************************************************************/
void PrintRadioUniqueId( const char*  pHeader,     // leading text descriptor
                         INT32U       idLength,    // string length with null
                         const INT8U* pUniqueId ); // 2-byte 'unicode' string



/******************************************************************************
 * Name: RfidTagAccessCallback
 *
 * Description:
 *   General purpose callback handler for the various tag access sample apps.
 *
 * Parameters:
 *   handle - handle to the radio from which this callback is made
 *   bufferLength - length, in bytes, of the data in pBuffer
 *   pBuffer - pointer to buffer that contains recevieved data
 *   context - the application specific context data structure
 *
 * Returns:
 *   Nothing
 ******************************************************************************/
INT32S RfidTagAccessCallback(
    RFID_RADIO_HANDLE   handle,
    INT32U              bufferLength,
    const INT8U*        pBuffer,
    void*               context
    );


/******************************************************************************
 * Name: PacketTrace
 *
 * Description:
 *   Utility to print a packet trace formatted in an indented manner to show 
 *   packet pairing
 *
 * Parameters:
 *   bufferLength - The length, in bytes, of the buffer
 *   pBuffer - pointer to the packet data
 *   contextIndent - in/out value to keep track of the current indentation level
 *
 * Returns:
 *   Nothing
 ******************************************************************************/
void PacketTrace(
    INT32U              bufferLength,
    const INT8U*        pBuffer,
    int *               contextIndent
    );

/******************************************************************************
 * Name: RFIDStatusToString
 *
 * Description:
 *   Utility to conver the RFID_STATUS enum into a printable string
 *	
 * Parameters:
 *   status - The return status from a rfid_library API call
 *
 * Returns:
 *   An string interpretation of the input status
 ******************************************************************************/
const char* RFIDStatusToString(
    RFID_STATUS status
    );
#endif /* #ifndef SAMPLE_UTILITY_H_INCLUDED */
