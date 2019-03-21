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
 * $Id: nvmemupd.h 66186 2010-12-04 00:00:31Z clyle $
 * 
 * Description:
 *
 *   header file defines structures and constants used for the MAC's
 *   NV MEM UPDATE MODE.
 *   *** See the MAC EDS for detailed command documentation. ****
 *
 *****************************************************************************
 */
#ifndef H_NVMEMUPD_H
#define H_NVMEMUPD_H

/* common types header - used by host and mac */
#include "rfid_platform_types.h"

/* number of ms the mac will delay after exiting
   nv mem update mode and before rebooting the device
   A delay is needed to allow any final communications from mac
   to host to reach the host application */
#define NVMEMUPD_REBOOT_DELAY_MS 5000

/* Timeout for receiving and transmitting a packet when in nvmem update mode.
   This allows the mac to become responsive again in the event the host
   goes away */
#define NVMEMUPD_MAC_RX_TIMEOUT 15000
#define NVMEMUPD_MAC_TX_TIMEOUT 10000

/*
  NV Mem Packet "cmd" field values (Command Values)
*/
typedef enum
{
  NVMEMUPD_CMD_STATUS,
  NVMEMUPD_CMD_UPD_RANGE,
  NVMEMUPD_CMD_UPD_COMPLETE,
  /* end of list marker - used for bounds checking in code!*/
  NVMEMUPD_CMD_LAST = NVMEMUPD_CMD_UPD_COMPLETE,
} NVMEMUPD_CMD_VALS;

/*
  NV Mem status values - these are set in the status field of the NVMEMPKT_ACK
  packet
*/
typedef enum
{
  NVMEMUPD_STAT_UPD_SUCCESS,      /* 0x00000000 */
  NVMEMUPD_STAT_RESERVED_01,      /* 0x00000001 */
  NVMEMUPD_STAT_WR_FAIL,          /* 0x00000002 */
  NVMEMUPD_STAT_UNK_CMD,          /* 0x00000003 */
  NVMEMUPD_STAT_CMD_IGN,          /* 0x00000004 */
  NVMEMUPD_STAT_BNDS,             /* 0x00000005 */
  NVMEMUPD_STAT_MAGIC,            /* 0x00000006 */
  NVMEMUPD_STAT_PKTLEN,           /* 0x00000007 */
  NVMEMUPD_STAT_EXIT_ERR,         /* 0x00000008 */
  NVMEMUPD_STAT_EXIT_SUCCESS,     /* 0x00000009 */
  NVMEMUPD_STAT_EXIT_NOWRITES,    /* 0x0000000A */
  NVMEMUPD_STAT_GEN_RXPKT_ERR,    /* 0x0000000B */
  NVMEMUPD_STAT_RESERVED_02,      /* 0x0000000C */
  NVMEMUPD_STAT_INT_MEM_BNDS,     /* 0x0000000D */
  NVMEMUPD_STAT_ENTRY_OK,         /* 0x0000000E */
  NVMEMUPD_STAT_RXPKT_MAX,        /* 0x0000000F */
  NVMEMUPD_STAT_RX_TO,            /* 0x00000010 */
  NVMEMUPD_STAT_CRC_ERR,          /* 0x00000011 */
  /* end of list marker - used for bounds checking in code!*/
  NVMEMUPD_STAT_LAST = NVMEMUPD_STAT_CRC_ERR,
} NVMEMUPD_STAT_VALS;

#define NVMEMUPD_PKT_MAGIC ((INT16U)0xF00D)
/*
  nvmempkt_common_hdr
  Common Header for all NV Mem packets.
*/
typedef struct nvmempkt_common_hdr
{
  /* MAGIC value that helps identify and verify NV Mem packets */
  INT16U magic;
  /* Command Value - see NVMEMUPD_CMD_xxx values */
  INT16U cmd;
  /* Packet length in the defined unit length - NVMEMPKT_BYTES_PER_UNIT */
  INT16U pkt_len;
  /* reserved field - however, may be used in a packet specfic way in future*/
  INT16U res0;
} NVMEMPKT_COMMON_HDR;

#define NVMEMPKT_STATUS_MACERRNO_SHIFT 16
/*
  nvmempkt_status
  Status packet - Set at the start of the nv mem update protocol sequence
  and always sent in response to other packets
*/
typedef struct nvmempkt_status
{
  /* Common header - used in all nv mem pkts */
  NVMEMPKT_COMMON_HDR cmn;
  /* "Regarding command" - the command that this ACK is regarding */
  INT32U re_cmd;

  /* 15:0  status code - one of the NVMEMUPD_STAT_xxxx values */
  /* 31:16 Lower 16-bits of macerrno */
  INT32U status;
}NVMEMPKT_STATUS;

/*
  nvmempkt_upd_range
  Update range - sent by the host to actually update a range of nv memory
  with new data.
*/
#define NVMEMPKT_UPD_RANGE_TESTMODE 0x00000001
typedef struct nvmempkt_upd_range
{
  /* Common header - used in all nv mem pkts */
  NVMEMPKT_COMMON_HDR cmn;
  /* absolute address of where the data in non volatile memory should be updated*/
  INT32U abs_addr;
  /* crc - a crc (currently CRC32) of the whole packet, excluding the crc word itself */
  INT32U crc;
  /* flags */
  INT32U flags;
  /* update data - this is what is actually written to nv memory! */
  INT32U upd_data[1];
}NVMEMPKT_UPD_RANGE;

/* Retrieve the number of padding bytes.                                      */
/* s - the structure that contains the flags field                            */
#define NVMEMPKT_UPD_RANGE_PADDING_BYTES(s) ((((s).flags) >> 1) & 0x00000003)
/* Set the number of padding bytes.                                           */
/* f - the INT32U that represents the flags field                             */
/* p - the number of padding bytes (a value between 0 and                     */
/*     NVMEMPKT_BYTES_PER_UNIT, inclusive)                                    */
#define NVMEMPKT_UPD_RANGE_SET_PADDING_BYTES(f,p)                              \
     f = ((f) & 0xFFFFFFF9) | (((p) & 0x00000003) << 1)

/*
  nvmempkt_upd_complete
  Update compelte - send by the host to indicate that no more updates are
  coming and that the mac should complete the nv memory update process and
  reboot.
*/
typedef struct nvmempkt_upd_complete
{
  /* Common header - used in all nv mem pkts */
  NVMEMPKT_COMMON_HDR cmn;
  /* no packet specific fields in this packet */
}NVMEMPKT_UPD_COMPLETE;

/*
  The length of packets in the nv mem command packet format are expressed in
  some unit.  In this case, it is 32-bit words.  These macros are used for
  determining the length of particular packets in the expressed units.
  1 Unit = 32 bits (i.e., 4 bytes)
*/
#define NVMEMPKT_BYTES_PER_UNIT 4
#define NVMEMPKT_UNIT_LEN(pkt) (sizeof(pkt) / NVMEMPKT_BYTES_PER_UNIT)

/* Macros for the common header length and additional packet specific length */
#define NVMEMPKT_CMN_LEN NVMEMPKT_UNIT_LEN(NVMEMPKT_COMMON_HDR)
#define NVMEMPKT_ADDITIONAL_UNIT_LEN(pkt) ((NVMEMPKT_UNIT_LEN(pkt)) - (NVMEMPKT_CMN_LEN))

/* Macro to get the complete byte length of a packet including the common  */
/* fields                                                                     */
#define NVMEMPKT_FULL_BYTE_LEN(pktlen)                                       \
        ((INT32U) ((NVMEMPKT_CMN_LEN * NVMEMPKT_BYTES_PER_UNIT) +            \
        ((pktlen) * NVMEMPKT_BYTES_PER_UNIT)))

/* Macro to convert packet specific length to a byte len                      */
#define NVMEMPKT_PKT_BYTE_LEN(pktlen)                                        \
        ((INT32U) ((pktlen) * NVMEMPKT_BYTES_PER_UNIT))

/* macro for the minimum length of any packet */
#define NVMEMPKT_BYTE_LEN_MIN sizeof(NVMEMPKT_COMMON_HDR)

/********************************************************************************
  Length values for all the NVMEMPKT_xxx packet structures defined above - these
  are used for the pkt_len field of the common header.
  For variable length packets a value of 1 is subtracted to indicate a "minimum"
  length of the packet - the code must then add in the variable length to the
  pkt_len field before transmit.*/
#define NVMEMPKT_LEN_STATUS NVMEMPKT_ADDITIONAL_UNIT_LEN(NVMEMPKT_STATUS)
#define NVMEMPKT_LEN_MIN_UPD_RANGE (NVMEMPKT_ADDITIONAL_UNIT_LEN(NVMEMPKT_UPD_RANGE) - 1)
#define NVMEMPKT_LEN_UPD_COMPLETE NVMEMPKT_ADDITIONAL_UNIT_LEN(NVMEMPKT_UPD_COMPLETE)

/* macro for getting the data length of an update range packet */
#define NVMEMPKT_LEN_UPD_RANGE_DATA(pktlen,pad) (NVMEMPKT_PKT_BYTE_LEN(pktlen) - \
        (NVMEMPKT_LEN_MIN_UPD_RANGE * NVMEMPKT_BYTES_PER_UNIT) - pad)

#endif //H_NVMEMUPD_H
