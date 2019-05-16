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
  * $Id: print_packet.c 65716 2010-10-17 20:03:54Z dshaheen $
  *
  * Description:
  *     This file contains a function that prints out replies from the radio
  *     in a descriptive fashion.
  *
  *
  *****************************************************************************
  */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "byte_swap.h"
#include "print_packet.h"
#include "rfid_library.h"
#include "rfid_packets.h"
#include "maccmds.h"

static void PrintIndentedLine(
	int indent,
	char *fmt,
	...
)
{
	va_list list;
	int i;

	va_start(list, fmt);
	for (i = 0; i < indent; i++)
		printf("  ");
	vprintf(fmt, list);
}

static void PrintByteArray(
	const INT8U *bytes,
	int length
)
{
	int index;

	for (index = 0; index < length; ++index)
	{
		printf("%s%.2x", (index && !(index % 4)) ? " " : "", bytes[index]);
	}
	printf("\n");
}

static void PrintByteArrayNoFormatting(
	const INT8U *bytes,
	int length,
	char *prefix,
	char *suffix
)
{
	int index;

	if (prefix != NULL)
	{
		printf("%s", prefix);
	}
	for (index = 0; index < length; ++index)
	{
		printf("%.2x", bytes[index]);
	}
	if (suffix != NULL)
	{
		printf("%s", suffix);
	}
}

void PrintPacket(
	const INT8U *packet,
	int indent
)
{
	RFID_PACKET_COMMON *common = (RFID_PACKET_COMMON *)packet;
	INT16U packetType = MacToHost16(common->pkt_type);

	//PrintIndentedLine(indent, "Packet version: %u\n", common->pkt_ver);
	switch (packetType)
	{
	case RFID_PACKET_TYPE_COMMAND_BEGIN:
	{
		RFID_PACKET_COMMAND_BEGIN *cmdbegin =
			(RFID_PACKET_COMMAND_BEGIN *)packet;
		INT32U command = MacToHost32(cmdbegin->command);

		//PrintIndentedLine(indent,
		//	"Packet type: RFID_PACKET_COMMAND_BEGIN\n");
		//PrintIndentedLine(indent,
		//	"Operation was%s executed in continuous mode\n",
		//	common->flags & 1 ? "" : " not");
		//PrintIndentedLine(indent, "Operation started at millisecond %u\n",
		//	MacToHost32(cmdbegin->ms_ctr));
		//PrintIndentedLine(indent, "Operation that is beginning:");
		switch (command)
		{
		case CMD_18K6CINV:
			//printf(" Inventory\n");
			break;
		case CMD_18K6CREAD:
			printf(" Read\n");
			break;
		case CMD_18K6CWRITE:
			printf(" Write\n");
			break;
		case CMD_18K6CLOCK:
			printf(" Lock\n");
			break;
		case CMD_18K6CKILL:
			printf(" Kill\n");
			break;
		case CMD_18K6CBLOCKERASE:
			printf(" BlockErase\n");
			break;
		case CMD_18K6CBLOCKWRITE:
			printf(" BlockWrite\n");
			break;
		case CMD_18K6CQT:
			printf(" QT\n");
			break;
		default:
			printf(" Unknown (%u)\n", command);
			break;
		}
		break;
	}
	case RFID_PACKET_TYPE_COMMAND_END:
	{
		RFID_PACKET_COMMAND_END *cmdend =
			(RFID_PACKET_COMMAND_END *)packet;
		PrintIndentedLine(indent, "Packet type: RFID_PACKET_COMMAND_END\n");
		//PrintIndentedLine(indent, "Operation ended at millisecond %u\n",
		//	MacToHost32(cmdend->ms_ctr));
		//PrintIndentedLine(indent, "Packet Flags: %u\n",
		//	MacToHost32(cmdend->cmn.flags));
		//PrintIndentedLine(indent, "Operation status: %u (%s)\n",
		//	MacToHost32(cmdend->status),
		//	cmdend->status ? "Failed" : "Succeeded");
		break;
	}
	case RFID_PACKET_TYPE_ANTENNA_CYCLE_BEGIN:
		PrintIndentedLine(indent, "Packet type: RFID_PACKET_ANTENNA_CYCLE_BEGIN\n");
		break;
	case RFID_PACKET_TYPE_ANTENNA_CYCLE_END:
		PrintIndentedLine(indent, "Packet type: RFID_PACKET_ANTENNA_CYCLE_END\n");
		break;
	case RFID_PACKET_TYPE_ANTENNA_BEGIN:
	{
		RFID_PACKET_ANTENNA_BEGIN *antennabegin =
			(RFID_PACKET_ANTENNA_BEGIN *)packet;
		PrintIndentedLine(indent,
			"Packet type: RFID_PACKET_ANTENNA_BEGIN\n");
		PrintIndentedLine(indent, "Antenna ID: %u\n",
			MacToHost32(antennabegin->antenna));
		break;
	}
	case RFID_PACKET_TYPE_ANTENNA_END:
		PrintIndentedLine(indent, "Packet type: RFID_PACKET_ANTENNA_END\n");
		break;
	case RFID_PACKET_TYPE_INVENTORY_CYCLE_BEGIN:
		PrintIndentedLine(indent,
			"Packet type: RFID_PACKET_INVENTORY_CYCLE_BEGIN\n");
		break;
	case RFID_PACKET_TYPE_INVENTORY_CYCLE_END:
		PrintIndentedLine(indent,
			"Packet type: RFID_PACKET_INVENTORY_CYCLE_END\n");
		break;
	case RFID_PACKET_TYPE_18K6C_INVENTORY_ROUND_BEGIN:
		PrintIndentedLine(indent,
			"Packet type: RFID_PACKET_18K6C_INVENTORY_ROUND_BEGIN\n");
		break;
	case RFID_PACKET_TYPE_18K6C_INVENTORY_ROUND_END:
		PrintIndentedLine(indent,
			"Packet type: RFID_PACKET_18K6C_INVENTORY_ROUND_END\n");
		break;
	case RFID_PACKET_TYPE_18K6C_INVENTORY:
	{
		RFID_PACKET_18K6C_INVENTORY *inv =
			(RFID_PACKET_18K6C_INVENTORY *)packet;
		int length =
			((MacToHost16(common->pkt_len) - 3) * 4) - (common->flags >> 6);

		INT8U *byteData = (INT8U *)&inv->inv_data[0];
		INT16U rssi = (INT8U *)&inv->rssi;
		int epcLength = 0;
		int tidLength = 0;
		if (((common->flags >> 2) & 0x03) == 0x01)  /* M4 TID (12 bytes) is included in data */
		{
			tidLength = 12;
		}
		epcLength = length - tidLength - 4;  /* -4 for 16-bit PC and CRC */

		/*PrintIndentedLine(indent,
			"Packet type: RFID_PACKET_18K6C_INVENTORY\n");
		PrintIndentedLine(indent, "Packet CRC was %s\n",
			common->flags & 1 ? "invalid" : "valid");
		PrintIndentedLine(indent, "Tag was inventoried at millisecond %u\n",
			MacToHost32(inv->ms_ctr));
		PrintIndentedLine(indent, "Inventory data is PC,EPC,CRC[,TID]: ");*/
		/*PrintByteArrayNoFormatting(&byteData[0], 2, NULL, ",");*/
		printf("EPC: ");
		PrintByteArrayNoFormatting(&byteData[2], epcLength, NULL, ",");
		printf("RSSI: %d ", rssi);
		//PrintByteArrayNoFormatting(&byteData[2 + epcLength], 2, NULL, NULL);  /* +2 to get past PC */
		/* if TID is included, print it out */
		if (tidLength != 0)
		{
			PrintByteArrayNoFormatting(&byteData[4 + epcLength], tidLength, ",", NULL); /* +4 to get past PC and CRC */
		}
		printf("\n");

		break;
	}
	case RFID_PACKET_TYPE_TX_RANDOM_DATA_STATUS:
		PrintIndentedLine(indent,
			"Packet type: RFID_PACKET_TYPE_TX_RANDOM_DATA_STATUS\n");
		break;
	default:
		PrintIndentedLine(indent, "Packet type %u not handled yet\n",
			packetType);
		break;
	}
	printf("\n");
	fflush(stdout);
}
