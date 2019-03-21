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
 * $Id: rfid_packets.h 80089 2012-04-13 21:38:33Z dciampi $
 *
 * Description:
 *
 *     This is the RFID Library header file that contains the declarations and
 *     definitions for structures for the packets that are described in the
 *     RFID Common Packet Format Specification.  These are the packets that are
 *     provided to the application via the application's callback function that
 *     is provided to the RFID_18K6CTag* functions.
 *     Consult the RFID Common Packet Format Specification for details.
 *     Note:  All 16- and 32-bit integers in the structures are in little-endian.
 *            Therefore, byte-swapping may be necessary depending upon the native
 *            endianess of the processor upon which code that is consuming said
 *            packets will be executed.
 *
 *****************************************************************************
 */

#ifndef RFID_PACKETS_H_INCLUDED
#define RFID_PACKETS_H_INCLUDED

#include "rfid_platform_types.h"

/* RFID 18k6c command codes */
enum
{
    RFID_18K6C_QUERYREP   = 0x00,
    RFID_18K6C_ACK        = 0x01,
    RFID_18K6C_QUERY      = 0x08,
    RFID_18K6C_QUERYADJ   = 0x09,
    RFID_18K6C_SELECT     = 0x0A,
    RFID_18K6C_NAK        = 0xC0,
    RFID_18K6C_REQRN      = 0xC1,
    RFID_18K6C_READ       = 0xC2,
    RFID_18K6C_WRITE      = 0xC3,
    RFID_18K6C_KILL       = 0xC4,
    RFID_18K6C_LOCK       = 0xC5,
    RFID_18K6C_ACCESS     = 0xC6,
    RFID_18K6C_BLOCKWRITE = 0xC7,
    RFID_18K6C_BLOCKERASE = 0xC8,
    RFID_18K6C_QT         = 0xE0 /* MSB of 18k6c command code only */
};


/* Packets belong to different classes.  These are the defined classes        */
typedef enum
{
    RFID_PACKET_CLASS_COMMON,
    RFID_PACKET_CLASS_DIAGNOSTICS,
    RFID_PACKET_CLASS_STATUS,
    RFID_PACKET_CLASS_RESERVED,
    RFID_PACKET_CLASS_DEBUG,
    /* A marker to indicate the end of the list.                              */
    RFID_PACKET_CLASS_LAST
} RFID_PACKET_CLASS;

/* A packet type number is comprised of an 8-bit class and a 12-bit packet    */
/* number.  The layout of the packet type number is 0xCNNN where C is the     */
/* class and N is the number within the class.                                */

/* The macros help in creating and breaking apart packet types from/into their*/
/* their component values.                                                    */
#define RFID_PACKET_CLASS_MASK      0x000F
#define RFID_PACKET_CLASS_SHIFT     12
#define RFID_PACKET_NUMBER_MASK     0x0FFF

/* Creates the base packet type number for a class.  The base packet type     */
/* number is incremented to obtain packet type numbers for subsequent packets.*/
#define RFID_PACKET_CLASS_BASE(c)   ((INT16U) ((c) << RFID_PACKET_CLASS_SHIFT))

/* Extracts the RFID class or number from a packet type (t)                   */
#define EXTRACT_RFID_PACKET_CLASS(t)                                           \
    (((t) >> RFID_PACKET_CLASS_SHIFT) & RFID_PACKET_CLASS_MASK)
#define EXTRACT_RFID_PACKET_NUMBER(t)                                          \
    ((t) & RFID_PACKET_NUMBER_MASK)

/* The 16-bit packet types that will be found in the common packet header     */
/* pkt_type field.                                                            */
/*                                                                            */
/* When adding a new packet type to a class, simply append it to end of the   */
/* appropriate type's enumeration list.                                       */
/*                                                                            */
/* NOTE: These packet type constants are in the endian format for the system  */
/* upon which the compile is being performed.  Before comparing them against  */
/* the packet type field from the packet, ensure that, if necessary, the      */
/* packet type field is converted from little endian (i.e., MAC format) to    */
/* the endian format for the system running the application.                  */
enum
{
    /* The packet types for the common packets.                               */
    RFID_PACKET_TYPE_COMMAND_BEGIN               =
        RFID_PACKET_CLASS_BASE(RFID_PACKET_CLASS_COMMON),
    RFID_PACKET_TYPE_COMMAND_END,
    RFID_PACKET_TYPE_ANTENNA_CYCLE_BEGIN,
    RFID_PACKET_TYPE_ANTENNA_BEGIN,
    RFID_PACKET_TYPE_18K6C_INVENTORY_ROUND_BEGIN,
    RFID_PACKET_TYPE_18K6C_INVENTORY,
    RFID_PACKET_TYPE_18K6C_TAG_ACCESS,
    RFID_PACKET_TYPE_ANTENNA_CYCLE_END,
    RFID_PACKET_TYPE_ANTENNA_END,
    RFID_PACKET_TYPE_18K6C_INVENTORY_ROUND_END,
    RFID_PACKET_TYPE_INVENTORY_CYCLE_BEGIN,
    RFID_PACKET_TYPE_INVENTORY_CYCLE_END,
    RFID_PACKET_TYPE_CARRIER_INFO,
    RFID_PACKET_TYPE_RES5,
    RFID_PACKET_TYPE_COMMAND_ACTIVE,
    
    /* The packet types for the diagnostics packets.                          */
    RFID_PACKET_TYPE_RES0          =
        RFID_PACKET_CLASS_BASE(RFID_PACKET_CLASS_DIAGNOSTICS),
    RFID_PACKET_TYPE_RES1,
    RFID_PACKET_TYPE_RES2,
    RFID_PACKET_TYPE_RES3,
    RFID_PACKET_TYPE_18K6C_INVENTORY_ROUND_BEGIN_DIAGS,
    RFID_PACKET_TYPE_18K6C_INVENTORY_ROUND_END_DIAGS,
    RFID_PACKET_TYPE_18K6C_INVENTORY_DIAGS,
    RFID_PACKET_TYPE_RES4,
    RFID_PACKET_TYPE_INVENTORY_CYCLE_END_DIAGS,
    RFID_PACKET_TYPE_SJC_SCANRESULT,
    RFID_PACKET_TYPE_TX_RANDOM_DATA_STATUS,
    RFID_PACKET_TYPE_CSM_PROTSCHED_SM_STATUS,
    RFID_PACKET_TYPE_CSM_PROTSCHED_LBT_STATUS,

    /* The packet types for the status packets.                               */
    RFID_PACKET_TYPE_NONCRITICAL_FAULT           =
        RFID_PACKET_CLASS_BASE(RFID_PACKET_CLASS_STATUS),


    /* The packet types for the reserved/command response packets             */
    RFID_PACKET_TYPE_ENGTESTPAT_ZZS = 
        RFID_PACKET_CLASS_BASE(RFID_PACKET_CLASS_RESERVED),
    RFID_PACKET_TYPE_ENGTESTPAT_FFS,
    RFID_PACKET_TYPE_ENGTESTPAT_W1S,
    RFID_PACKET_TYPE_ENGTESTPAT_W0S,
    RFID_PACKET_TYPE_ENGTESTPAT_BND,
    RFID_PACKET_TYPE_MBP_READ_REG,
    RFID_PACKET_TYPE_GPIO_READ,
    RFID_PACKET_TYPE_OEMCFG_READ,
    RFID_PACKET_TYPE_ENG_RSSI,
    RFID_PACKET_TYPE_ENG_INVSTATS,
    RFID_PACKET_TYPE_ENG_BERTSTRESULT,
    RFID_PACKET_TYPE_NVMEMUPDCFG,
    RFID_PACKET_TYPE_LPROF_READ_REG,
    RFID_PACKET_TYPE_RES6,
    RFID_PACKET_TYPE_ENG_XY,

    /* The packet types for the debug packets.                               */
    RFID_PACKET_TYPE_DEBUG                         =
        RFID_PACKET_CLASS_BASE(RFID_PACKET_CLASS_DEBUG),
};

/******************************************************************************
 * Name:  RFID_RFID_PACKET_COMMON - The common packet preamble that contains
 *       fields that are common to all packets.
 ******************************************************************************/
typedef struct hostpkt_cmn
{
    /* Packet specific version number                                         */
    INT8U   pkt_ver;
    /* Packet specific flags*/
    INT8U   flags;
    /* Packet type identifier                                                 */
    INT16U  pkt_type;
    /* Packet length indicator - number of 32-bit words that follow the common*/
    /* packet preamble (i.e., this struct)                                    */
    INT16U  pkt_len;
    /* Reserved for future use                                                */
    INT16U  res0;
} RFID_PACKET_COMMON;

/******************************************************************************
 * Name:  RFID_PACKET_COMMAND_BEGIN - The command-begin packet.
 ******************************************************************************/
typedef struct hostpkt_cmd_beg
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* The command for which the packet sequence is in response to              */
  INT32U                command;
  /* Current millisecond timer/counter                                        */
  INT32U                ms_ctr;
} RFID_PACKET_COMMAND_BEGIN;

/* Macros to make it easier to extract the bit fields out of the command      */
/* begin flags field (i.e., cmn.flags)                                        */
#define RFID_COMMAND_IN_CONTINUOS_MODE(f)        ((f) & 0x01)
#define RFID_COMMAND_NOT_IN_CONTINUOUS_MODE(f)   \
    (!(RFID_COMMAND_IN_CONTINUOUS_MODE(f)))

/******************************************************************************
 * Name:  RFID_PACKET_COMMAND_END - The command-end packet.
 ******************************************************************************/
typedef struct hostpkt_cmd_end
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* Current millisecond timer/counter                                        */
  INT32U                ms_ctr;
  /* Command status indicator                                                 */
  INT32U                status;
} RFID_PACKET_COMMAND_END;

/* Command end flags field i.e., cmn.flags) bit definitions                   */
#define HOSTIF_TX_SATURATE_FLAG       0x01
    
/******************************************************************************
 * Name:  RFID_PACKET_ANTENNA_CYCLE_BEGIN - The antenna-cycle-begin packet.
 ******************************************************************************/
typedef struct hostpkt_cyc_beg
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* No other packet specific fields                                          */
} RFID_PACKET_ANTENNA_CYCLE_BEGIN;

/******************************************************************************
 * Name:  RFID_PACKET_ANTENNA_CYCLE_END - The antenna cycle-end packet.
 ******************************************************************************/
typedef struct hostpkt_cyc_end
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* No other packet specific fields                                          */
} RFID_PACKET_ANTENNA_CYCLE_END;

/******************************************************************************
 * Name:  RFID_PACKET_ANTENNA_BEGIN - The antenna-begin packet.
 ******************************************************************************/
typedef struct hostpkt_ant_beg
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* The logical antenna ID                                                   */
  INT32U                antenna;
} RFID_PACKET_ANTENNA_BEGIN;

/******************************************************************************
 * Name:  RFID_PACKET_ANTENNA_END - The antenna-end packet.
 ******************************************************************************/
typedef struct hostpkt_ant_end
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* No other packet specific fields                                          */
} RFID_PACKET_ANTENNA_END;

/******************************************************************************
 * Name:  RFID_PACKET_INVENTORY_CYCLE_BEGIN - The inventory-cycle-begin packet.
 ******************************************************************************/
typedef struct hostpkt_inv_cyc_beg
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* Current millisecond timer/counter                                        */
  INT32U                ms_ctr;
} RFID_PACKET_INVENTORY_CYCLE_BEGIN;

/******************************************************************************
 * Name:  RFID_PACKET_INVENTORY_CYCLE_END - The inventory-cycle-end packet.
 ******************************************************************************/
typedef struct hostpkt_inv_cyc_end
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* Current millisecond timer/counter                                        */
  INT32U                ms_ctr;
} RFID_PACKET_INVENTORY_CYCLE_END;

/******************************************************************************
 * Name:  RFID_PACKET_INVENTORY_CYCLE_END_DIAGS - The inventory-cycle-end
 *        diagnostics packet.
 ******************************************************************************/
typedef struct hostpkt_inv_cyc_end_diag
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* Number of query's issued */
  INT32U                querys;
  /* Number of RN16's received */
  INT32U                rn16rcv;
  /* Number of RN16 timeouts (i.e., no detected response to ISO 18000-6C      */
  /* Query or QueryRep)                                                       */
  INT32U                rn16to;
  /* Number of EPC timeouts (i.e., no detected response to ISO 18000-6C RN16  */
  /* ACK)                                                                     */
  INT32U                epcto;
  /* Number of good EPC reads */
  INT32U                good_reads;
  /* Number of CRC failures                                                   */
  INT32U                crc_failures;
} RFID_PACKET_INVENTORY_CYCLE_END_DIAGS;

/******************************************************************************
 * Name:  RFID_PACKET_18K6C_INVENTORY_ROUND_BEGIN - The ISO 18000-6C inventory-
 *        round-begin packet.
 ******************************************************************************/
typedef struct hostpkt_inv_rnd_beg
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* No packet specific fields                                                */
} RFID_PACKET_18K6C_INVENTORY_ROUND_BEGIN;

/******************************************************************************
 * Name:  RFID_PACKET_18K6C_INVENTORY_ROUND_END - The ISO 18000-6C inventory-
 *        round-end packet.
 ******************************************************************************/
typedef struct hostpkt_inv_rnd_end
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* No packet specific fields                                                */
} RFID_PACKET_18K6C_INVENTORY_ROUND_END;

/******************************************************************************
 * Name:  RFID_PACKET_18K6C_INVENTORY_ROUND_BEGIN_DIAGS - The ISO 18000-6C
 *        inventory-round-begin diagnostics packet.
 ******************************************************************************/
typedef struct hostpkt_inv_rnd_beg_diag
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* Current millisecond timer/counter                                        */
  INT32U                ms_ctr;
  /* Starting singulation parameters                                          */
  INT32U                sing_params;
} RFID_PACKET_18K6C_INVENTORY_ROUND_BEGIN_DIAGS;

/* Macros to make it easier to extract the bit fields out of the singulation  */
/* parameters field (i.e., sing_params).  NOTE - since the packet format      */
/* specifies that 16-bit fields are transmitted in little endian, ensure that */
/* the field is byte swapped, if necessary, for the host system before        */
/* applying any of the macros.                                                */
#define RFID_SINGULATION_PARMS_CURRENT_Q(sing)          (((sing) & 0x000F)
#define RFID_SINGULATION_PARMS_CURRENT_SLOT(sing)       (((sing) >> 4) & 0x1ffff)
#define RFID_SINGULATION_PARMS_INVENTORY_A(sing)        (!(((sing) >> 21) & 0x0001))
#define RFID_SINGULATION_PARMS_INVENTORY_B(sing)        (((sing) >> 21)& 0x0001)
#define RFID_SINGULATION_PARMS_CURRENT_RETRY(sing)      (((sing) >> 22) & 0xff)

/******************************************************************************
 * Name:  RFID_PACKET_18K6C_INVENTORY_ROUND_END_DIAGS - The ISO 18000-6C
 *        inventory-round-end diagnostics packet.
 ******************************************************************************/
typedef struct hostpkt_inv_rnd_end_diag
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* Current millisecond timer/counter                                        */
  INT32U                ms_ctr;
  /* Number of query's issued */
  INT32U                querys;
  /* Number of RN16's received */
  INT32U                rn16rcv;
  /* Number of RN16 timeouts (i.e., no detected response to ISO 18000-6C      */
  /* Query or QueryRep)                                                       */
  INT32U                rn16to;
  /* Number of EPC timeouts (i.e., no detected response to ISO 18000-6C RN16  */
  /* ACK)                                                                     */
  INT32U                epcto;
  /* Number of good EPC reads */
  INT32U                good_reads;
  /* Number of CRC failures                                                   */
  INT32U                crc_failures;
} RFID_PACKET_18K6C_INVENTORY_ROUND_END_DIAGS;

/******************************************************************************
 * Name:  RFID_PACKET_18K6C_INVENTORY - The ISO 18000-6C inventory packet.
 ******************************************************************************/
typedef struct hostpkt_18k6c_inv
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* current millisecond timer/counter                                        */
  INT32U                ms_ctr;
  /* Receive Signal Strength Indicator - backscattered tab signal */
  /* amplitude.                                                               */
  INT8U                 nb_rssi;
  INT8U                 wb_rssi_other;
  INT16U                ana_ctrl1;
  INT16U                rssi;
  INT8U                 phase;
  INT8U                 chidx_phyant;
  /* Variable length inventory data (i.e., PC, EPC, and CRC)                  */
  INT32U                inv_data[1];
} RFID_PACKET_18K6C_INVENTORY;

/* Macros to make it easier to extract the bit fields out of the 18k6c        */
/* inventory flags field (i.e., cmn.flags)                                    */
#define RFID_18K6C_INVENTORY_CRC_IS_INVALID(f)   ((f) & 0x01)
#define RFID_18K6C_INVENTORY_CRC_IS_VALID(f)                                   \
    !(RFID_18K6C_INVENTORY_CRC_IS_INVALID(f))
#define RFID_18K6C_INVENTORY_PADDING_BYTES(f)    (((f) >> 6) & 0x03)

/******************************************************************************
 * Name:  RFID_PACKET_18K6C_INVENTORY_DIAGS - The ISO 18000-6C inventory
 *        diagnostics packet.
 ******************************************************************************/
typedef struct hostpkt_18k6c_inv_diag
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* Protocol parameters                                                      */
  INT32U                prot_parms;
} RFID_PACKET_18K6C_INVENTORY_DIAGS;

/* Macros to make it easier to extract the bit fields out of the 18k6c        */
/* inventory diagnostics protocol parameters (i.e., prot_parms) field.  NOTE -*/
/* since the packet format specifies that 32-bit fields are transmitted in    */
/* little endian, ensure that the field is byte swapped, if necessary, for the*/
/* host system before applying any of the macros.                             */
#define RFID_18K6C_PROTOCOL_PARMS_Q(parms)       ((parms) & 0x0000000F)
#define RFID_18K6C_PROTOCOL_PARMS_C(parms)       (((parms) & 0x00000070) >> 4)
#define RFID_18K6C_PROTOCOL_PARMS_TARI(parms)    (((parms) & 0x0000FF00) >> 8)

/******************************************************************************
 * Name:  RFID_PACKET_18K6C_TAG_ACCESS - The ISO 18000-6C tag-access packet.
 ******************************************************************************/
typedef struct hostpkt_18k6c_acc
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* Current millisecond timer/counter                                        */
  INT32U                ms_ctr;
  /* ISO 18000-6C access command                                              */
  INT8U                 command;
  /* Backscatter error code from tag access                                   */
  INT8U                 tag_error_code;
  /* Protocol attributed error code from tag access                                */
  INT16U                prot_error_code;
  /* The number of words successfully written; Write and BlockWrite only      */
  INT16U                write_word_count;
  /* Reserved                                                                 */
  INT16U                res0;
  /* Variable length access data                                              */
  INT32U                data[1];
} RFID_PACKET_18K6C_TAG_ACCESS;

/* Errors detected by protocol code during 18k6 caccess */
#define PROT_ACCESS_ERROR_FAIL                          0xFFFF /* -1 */
#define PROT_ACCESS_ERROR_NONE                          0x0000
#define PROT_ACCESS_ERROR_HANDLE_MISMATCH               0x0001
#define PROT_ACCESS_ERROR_BAD_CRC                       0x0002
#define PROT_ACCESS_ERROR_NO_REPLY                      0x0003
#define PROT_ACCESS_ERROR_INVALID_PASSWORD              0x0004
#define PROT_ACCESS_ERROR_ZERO_KILL_PASSWORD            0x0005
#define PROT_ACCESS_ERROR_TAG_LOST                      0x0006
#define PROT_ACCESS_ERROR_CMD_FORMAT_ERROR              0x0007
#define PROT_ACCESS_ERROR_READ_COUNT_INVALID            0x0008
#define PROT_ACCESS_ERROR_RETRY_COUNT_EXCEEDED          0x0009

/* 18k6c backscatter error codes */
#define BACKSCATTER_OTHER_ERROR                         0x00
#define BACKSCATTER_MEM_OVERRUN                         0x03
#define BACKSCATTER_MEM_LOCKED                          0x04
#define BACKSCATTER_INSUFFICIENT_POWER                  0x0B
#define BACKSCATTER_NONSPECIFIC_ERROR                   0x0F

/* Macros to make it easier to extract the bit fields out of the 18k6c        */
/* tag access flags (i.e., cmn.flags) field                                   */
#define RFID_18K6C_TAG_ACCESS_MAC_ERROR(f)             ((f) & 0x01)
#define RFID_18K6C_TAG_ACCESS_BACKSCATTER_ERROR(f)     ((f) & 0x02)
#define RFID_18K6C_TAG_ACCESS_ANY_ERROR(f)                    \
	       (RFID_18K6C_TAG_ACCESS_MAC_ERROR(f)           ||   \
	        RFID_18K6C_TAG_ACCESS_BACKSCATTER_ERROR(f))

#define RFID_18K6C_TAG_ACCESS_PADDING_BYTES(f)  (((f) >> 6) & 0x3)

/* For command field values, refer to 18k6c command code definitions in this file */



/******************************************************************************
 * Name:  RFID_PACKET_NONCRITICAL_FAULT - The non-critical-fault packet.
 ******************************************************************************/
typedef struct hostpkt_noncrit_fault
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* Current millisecond timer/counter                                        */
  INT32U                ms_ctr;
  /* Fault type                                                               */
  INT16U                fault_type;
  /* Fault subtype                                                            */
  INT16U                fault_subtype;
  /* Context specific data for fault                                          */
  INT32U                context;
} RFID_PACKET_NONCRITICAL_FAULT;

/******************************************************************************
 * Name:  RFID_PACKET_CARRIER_INFO (type = RFID_PACKET_TYPE_CARRIER_INFO)
 *        Contains info related to tranmit carrier
 ******************************************************************************/
typedef struct hostpkt_carrier_info
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* Current millisecond timer/counter                                        */
  INT32U                ms_ctr;
  /* current plldivmult setting                                               */
  INT32U                plldivmult;
  /* channel                                                                  */
  INT16U                chan;
  /* carrier flags                                                            */
  INT16U                cw_flags;
} RFID_PACKET_CARRIER_INFO;

#define CWFLAGS_CWON  0x0001
#define CWFLAGS_CWOFF 0x0000
#define CARRIER_INFO_IS_CWON(cw_flags)  (cw_flags & CWFLAGS_CWON)
#define CARRIER_INFO_IS_CWOFF(cw_flags) ((cw_flags & CWFLAGS_CWON) == 0 ? 1: 0)
#define CARRIER_INFO_SET_CWON(cw_flags) (cw_flags |= CWFLAGS_CWON)
#define CARRIER_INFO_SET_CWOFF(cw_flags) (cw_flags &= ~CWFLAGS_CWON)



/******************************************************************************
 * Name:  RFID_PACKET_COMMAND_ACTIVE - (type=RFID_PACKET_TYPE_COMMAND_ACTIVE)
 *        The comamnd active indicator packet.  Periodically output during 
 *        operational commands, but only if no other host interface output 
 *        traffic occurs within the 3 second timeout.  
 ******************************************************************************/
typedef struct hostpkt_command_active
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* Current millisecond timer/counter                                        */
  INT32U                ms_ctr;
} RFID_PACKET_COMMAND_ACTIVE;

/* Host interface timeout value (in seconds) for sending the command
   active packet, in the abscence of other host interface tx data.
*/
#define CMD_ACTIVE_TIMEOUT 3  

/******************************************************************************
 * Name:  RFID_PACKET_TX_RANDOM_DATA_STATUS (type = RFID_PACKET_TYPE_TX_RANDOM_DATA_STATUS)
 *        Contains info related to CW Random Transmit Duration
 ******************************************************************************/
typedef struct hostpkt_tx_random_data_status
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* Current millisecond timer/counter                                        */
  INT32U                ms_ctr;
  /* Elapsed Time Ms                                                            */
  INT32U                elapsedTimeMs;
} RFID_PACKET_TX_RANDOM_DATA_STATUS;

/******************************************************************************
 * Name:  RFID_PACKET_DEBUG - Debug Packet
 ******************************************************************************/
typedef struct hostpkt_debug
{
  /* Common preamble - part of every packet!                                  */
  RFID_PACKET_COMMON    cmn;
  /* Current millisecond timer/counter                                        */
  INT32U                ms_ctr;
  /* Packet Counter                                                                 */
  INT16U                counter;
  /* Debug Id                                                                 */
  INT16U                id;
  /* Variable length debug data                                               */
  INT8U                 debug_data[sizeof(INT32U)];
} RFID_PACKET_DEBUG;


#endif /* #ifndef RFID_PACKETS_H_INCLUDED */
