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
  * $Id: sample_utility.c 66029 2010-11-11 17:41:32Z dshaheen $
  *
  * Description:
  *     This file contains the implementation for a simple stack.
  *
  *
  *****************************************************************************
  */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "rfid_library.h"
#include "sample_utility.h"
#include "rfid_packets.h"
#include "byte_swap.h"


#include "print_packet.h"

#define BYTES_PER_LEN_UNIT  4


static void ProcessTagAccessPacket(
	const RFID_PACKET_18K6C_TAG_ACCESS* pTagAccess,
	void*                               context
);

/*  static variable to track the current indentation level for the           */
/*  PacketTrace utility                                                      */
static int indent = 0;


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
void PrintRadioUniqueId(
	const char*  pHeader,  // string character prefix, for labeling rows
	INT32U       idLength, // string length in char bytes including null
	const INT8U* pUniqueId // unicode encoded string characters, 2-bytes
)
{
	INT32U leader = (INT32U)strlen(pHeader);
	INT32U offset = 0;
	for (; offset < idLength; offset += 16) // loop on each 32-bit word
	{
		INT32U index;
		if (offset)
		{
			printf("\t%*s", leader, "");
		}
		else
		{
			printf("\t%s", pHeader);
		}
		for (index = 0; index < 16 && ((offset + index) < idLength); index++)
		{
			printf("%s%.2x", index % 4 ? "" : " ", pUniqueId[offset + index]);
		}
		printf("%*s|", ((((16 - index) % 4) * 2) + (((16 - index) / 4) * 9) + 2), "");

		for (index = 0; index < 16 && ((offset + index) < idLength); ++index)
		{
			printf("%c", isprint(pUniqueId[offset + index])
				? pUniqueId[offset + index] : ' ');
		}
		printf("%*s|\n", (16 - index), "");
	}
	fflush(stdout);
} /* PrintRadioUniqueId */




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
)
{
	INT32S          status = 0;
	CONTEXT_PARMS*  pParms = (CONTEXT_PARMS *)context;
	RFID_PACKET_18K6C_INVENTORY *inv = (RFID_PACKET_18K6C_INVENTORY *)pBuffer;

	RFID_UNREFERENCED_LOCAL(handle);

	/* Enable to get a trace of the received packets */
#if 0
	PacketTrace(bufferLength, pBuffer, &indent);
#endif 

	/* Process the packets in the buffer until either the entire buffer is    */
	/* processed or there is insufficient data                                */
	while (!status && bufferLength)
	{
		/* Get the packet pointer and determine the length in bytes           */
		const RFID_PACKET_COMMON*   pPacket =
			(const RFID_PACKET_COMMON *)pBuffer;
		INT32U                      packetLength =
			(MacToHost16(pPacket->pkt_len) * BYTES_PER_LEN_UNIT) +
			sizeof(RFID_PACKET_COMMON);

		/* Verify that the buffer is large enough for the packet              */
		if (bufferLength < packetLength)
		{
			fprintf(
				stderr,
				"ERROR: Remaining buffer = %d bytes, need %d bytes\n",
				bufferLength,
				packetLength);
			status = -1;
		}
		/* Otherwise, if it is a tag access packet, we want to inspect it     */
		else if (RFID_PACKET_TYPE_18K6C_TAG_ACCESS ==
			MacToHost16(pPacket->pkt_type))
		{
			ProcessTagAccessPacket(
				(const RFID_PACKET_18K6C_TAG_ACCESS *)pPacket,
				context);
		}
		else if (RFID_PACKET_TYPE_COMMAND_END ==
			MacToHost16(pPacket->pkt_type))
		{
			const RFID_PACKET_COMMAND_END* pEndPkt = (const RFID_PACKET_COMMAND_END *)pBuffer;

			status = pEndPkt->status;
		}


		/* Adjust the buffer length and pointer based upon the packet size    */
		bufferLength -= packetLength;
		pBuffer += packetLength;
	}

	return status;
} /* RfidTagAccessCallback */


/******************************************************************************
 * Name: ProcessTagAccessPacket
 *
 * Description:
 *   Tag access packet decoder.  Looks for errors in the packet flag word, and
 *   extracts the data if it is a read access packet
 *
 * Parameters:
 *   pTagAccess - pointer to a tag access packet
 *   context - the application specific context data structure
 *
 * Returns:
 *   Nothing
 ******************************************************************************/
static void ProcessTagAccessPacket(
	const RFID_PACKET_18K6C_TAG_ACCESS* pTagAccess,
	void*                               context
)
{
	CONTEXT_PARMS*  pParms = (CONTEXT_PARMS *)context;

	/* Check to see if the tag access failed                                  */
	if (RFID_18K6C_TAG_ACCESS_BACKSCATTER_ERROR(pTagAccess->cmn.flags))
	{
		fprintf(
			stderr,
			"ERROR: Tag backscattered error 0x%.8x on access\n",
			pTagAccess->tag_error_code);
	}
	else if (RFID_18K6C_TAG_ACCESS_MAC_ERROR(pTagAccess->cmn.flags))
	{
		fprintf(
			stderr,
			"ERROR: Protocol access error 0x%.8x on access\n",
			pTagAccess->prot_error_code);
	}
	else /* no error reported in the flag word */
	{
		/* If it is a read, then extract the data from the packet             */
		if (RFID_18K6C_READ == pTagAccess->command)
		{
			/* Calculate the length of the data portion of the packet         */
			INT32U dataLength =
				/* The length of the packet beyond the common header          */
				MacToHost16(pTagAccess->cmn.pkt_len) * BYTES_PER_LEN_UNIT +
				/* Add back in the size of the common header                  */
				sizeof(RFID_PACKET_COMMON) -
				/* Minus the fixed part of the access packet                  */
				sizeof(RFID_PACKET_18K6C_TAG_ACCESS) +
				/* Add back data field in the packet                          */
				sizeof(pTagAccess->data) -
				/* Subtract out the padding bytes                             */
				RFID_18K6C_TAG_ACCESS_PADDING_BYTES(pTagAccess->cmn.flags);
			/* Copy the data from the packet (if there is data)               */
			if (dataLength)
			{
				memcpy(pParms->pReadData, pTagAccess->data, dataLength);
			}
		}

		pParms->succesfulAccessPackets++;
	}
} /* ProcessTagAccessPacket */



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
)
{
	int *indent = contextIndent;
	RFID_PACKET_COMMON *common;
	INT16U packetType;
	int packetLength;

	while (bufferLength > 0)
	{
		common = (RFID_PACKET_COMMON *)pBuffer;
		packetType = MacToHost16(common->pkt_type);
		switch (packetType)
		{
		case RFID_PACKET_TYPE_18K6C_INVENTORY_ROUND_END:
		case RFID_PACKET_TYPE_INVENTORY_CYCLE_END:
		case RFID_PACKET_TYPE_ANTENNA_END:
		case RFID_PACKET_TYPE_ANTENNA_CYCLE_END:
		case RFID_PACKET_TYPE_COMMAND_END:
			*indent -= 1;
			break;
		}

		PrintPacket(pBuffer, *indent);

		switch (packetType)
		{
		case RFID_PACKET_TYPE_18K6C_INVENTORY_ROUND_BEGIN:
		case RFID_PACKET_TYPE_INVENTORY_CYCLE_BEGIN:
		case RFID_PACKET_TYPE_ANTENNA_BEGIN:
		case RFID_PACKET_TYPE_ANTENNA_CYCLE_BEGIN:
		case RFID_PACKET_TYPE_COMMAND_BEGIN:
			*indent += 1;
			break;
		}
		/* pkt_len is measured in 4-byte words, and
		   doesn't include the 8-byte common preamble */
		packetLength = MacToHost16(common->pkt_len) * 4 + 8;

		pBuffer += packetLength;
		bufferLength -= packetLength;
	}
}


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
)
{
	const char* pString;

	switch (status)
	{
	case RFID_STATUS_OK:
		pString = "Success";
		break;

	case RFID_ERROR_ALREADY_OPEN:
		pString = "Radio is already opened";
		break;

	case RFID_ERROR_BUFFER_TOO_SMALL:
		pString = "Buffer is too small";
		break;

	case RFID_ERROR_FAILURE:
		pString = "General failure";
		break;

	case RFID_ERROR_DRIVER_LOAD:
		pString = "Failed to load radio driver";
		break;

	case RFID_ERROR_DRIVER_MISMATCH:
		pString = "Driver version does not match required version";
		break;

	case RFID_ERROR_INVALID_ANTENNA:
		pString = "Antenna port is not valid";
		break;

	case RFID_ERROR_INVALID_HANDLE:
		pString = "Radio handle is not valid";
		break;

	case RFID_ERROR_INVALID_PARAMETER:
		pString = "Parameter is not valid";
		break;
	case RFID_ERROR_NO_SUCH_RADIO:
		pString = "Radio with supplied ID is not attached to the system";
		break;

	case RFID_ERROR_NOT_INITIALIZED:
		pString = "RFID library has not been previously initialized";
		break;

	case RFID_ERROR_NOT_SUPPORTED:
		pString = "Function is currently not supported";
		break;

	case RFID_ERROR_OPERATION_CANCELLED:
		pString = "The operation was cancelled";
		break;

	case RFID_ERROR_OUT_OF_MEMORY:
		pString = "RFID library failed to allocate memory";
		break;

	case RFID_ERROR_RADIO_BUSY:
		pString = "Operation cannot be performed because radio is busy";
		break;

	case RFID_ERROR_RADIO_FAILURE:
		pString = "The radio module indicated a failure";
		break;

	case RFID_ERROR_RADIO_NOT_PRESENT:
		pString = "The radio has been detached from the system";
		break;

	case RFID_ERROR_CURRENTLY_NOT_ALLOWED:
		pString = "RFID function is not currently allowed";
		break;

	case RFID_ERROR_RADIO_NOT_RESPONDING:
		pString = "The radio is not responding";
		break;

	case RFID_ERROR_NONVOLATILE_INIT_FAILED:
		pString =
			"The radio failed to initialize nonvolatile memory update";
		break;

	case RFID_ERROR_NONVOLATILE_OUT_OF_BOUNDS:
		pString = "Nonvolatile memory address is out of range";
		break;

	case RFID_ERROR_NONVOLATILE_WRITE_FAILED:
		pString = "The radio failed to write to nonvolatile memory";
		break;

	case RFID_ERROR_RECEIVE_OVERFLOW:
		pString = "The tranport layer detected an Rx overflow";
		break;

	case RFID_ERROR_UNEXPECTED_VALUE:
		pString = "The radio returned an unexpected value";
		break;

	case RFID_ERROR_NONVOLATILE_CRC_FAILED:
		pString = "Nonvolatile update range packet contained a CRC error";
		break;

	case RFID_ERROR_NONVOLATILE_PACKET_HEADER:
		pString = "Nonvolatile update packet contained an error in the header";
		break;

	case RFID_ERROR_NONVOLATILE_MAX_PACKET_LENGTH:
		pString = "The radio received more than the specified maximum packet size";
		break;

	default:
		pString = "Unknown error";
		break;
	}

	return pString;
} /* RFIDStatusToString */
