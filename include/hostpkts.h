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
 * $Id: hostpkts.h 66481 2011-01-04 17:40:05Z dshaheen $
 * 
 * Description:
 *
 *   This header file defines the local mac firmware MAC <-> Host packet
 *   structs that are private to both mac and host library (i.e. do NOT
 *   make their way up to the application level )
 *   See also - macregs.h, maccmds.h, rfid_packets.h
 *   See also - RFID Common Packet Format Specification for details on each
 *   of these packets and when the host can expect to recieve them.
 *
 *****************************************************************************
 */
#ifndef H_HOSTPKTS_H
#define H_HOSTPKTS_H

#include "rfid_platform_types.h"
#include "rfid_packets.h"

/* The length of packets in the common packet format are expressed in some    */
/* unit.  In this case, it is 32-bit words.  These macros are used for        */
/* determining the length of particular packets in the expressed units.       */
#define RFID_PACKET_BYTES_PER_UNIT  4   /* 1 Unit = 32 bits (i.e., 4 bytes)   */
#define RFID_PACKET_UNIT_LEN(pkt)   (sizeof(pkt) / RFID_PACKET_BYTES_PER_UNIT)

/* The length, in units, of the common packet header.                         */
#define RFID_PACKET_CMN_LEN         RFID_PACKET_UNIT_LEN(RFID_PACKET_COMMON)

/* When computing the unit length of a packet, the common header unit length  */
/* is subtracted as the unit length is the number of bytes after the common   */
/* header.                                                                    */
#define RFID_PACKET_ADDITIONAL_UNIT_LEN(pkt)                                  \
    ((RFID_PACKET_UNIT_LEN(pkt)) - (RFID_PACKET_CMN_LEN))


/*
  host_reg_req struct
  defines format for register access requests from host to mac
  NOTE - these are only for *MAC REGISTER ACCESS*
         these are not command packets
  See also : MAC EDS, Host Interface Section
*/
struct host_reg_req
{
  INT16U access_flg;
  INT16U reg_addr;
  INT32U reg_data;
};
#define HOST_REG_REQ_SIZE   sizeof(struct host_reg_req)

/* Constants for the access_flg field */
#define HOST_REG_REQ_ACCESS_TYPE    ((INT16U) 0x0001)

/* Constants for the register access types */
#define HOST_REG_REQ_ACCESS_READ    ((INT16U) 0x0000)
#define HOST_REG_REQ_ACCESS_WRITE   ((INT16U) 0x0001)

/*
  host_reg_resp struct
  defines format for register access responses from mac to host
  NOTE - these are only for *MAC REGISTER ACCESS*
         these are not command packets
  See also : MAC EDS, Host Interface Section
*/
struct host_reg_resp
{
  INT16U rfu0;
  INT16U reg_addr;
  INT32U reg_data;
};
#define HOST_REG_RESP_SIZE  sizeof(struct host_reg_resp)

/*
  hostpkt_mbp_read_reg

  MAC Bypass Read Register Packet

  See RFID Common Packet Format Specification for details
*/
typedef struct hostpkt_mbp_read_reg
{
  /* common preamble - part of every packet! */
  struct hostpkt_cmn cmn;
  /* address that was being read via MBPREADREG command*/
  INT16U addr;
  /* the data returned for the read*/
  INT16U data;
}RFID_PACKET_MBP_READ;

/*
  hostpkt_gpio_read

  GPIO Read Packet

  See RFID Common Packet Format Specification for details
*/
typedef struct hostpkt_gpio_read
{
  /* common preamble - part of every packet! */
  struct hostpkt_cmn cmn;

  /* the data returned for the read*/
  INT32U data;
}RFID_PACKET_GPIO_READ;

/*
  hostpkt_oemcfg_read

  Oem Configuration Area Read Packet

  See RFID Common Packet Format Specification for details
*/
typedef struct hostpkt_oemcfg_read
{
  /* common preamble - part of every packet! */
  struct hostpkt_cmn cmn;

  /* the address*/
  INT32U addr;

  /* the data returned for the read*/
  INT32U data;
}RFID_PACKET_OEMCFG_READ;


/*
  eng_rssi_data
  RSSI data struct - N of these are included in payload of hostpkt_eng_rssi
*/
#define ENGRSSI_FLAG_EXTFB_VALID      0x00000001
#define ENGRSSI_FLAG_RSSI_VALID       0x00000002
#define ENGRSSI_FLAG_LNA_VALID        0x00000004
#define ENGRSSI_FLAG_IFLNA_I_Q_VALID  0x00000008
#define ENGRSSI_FLAGS_MASK            0x0000000F
#define ENGRSSI_FLAGS_ARG1_SHIFT      16

/*
  hostpkt_eng_rssi

  RSSI engineering packet

  See RFID Common Packet Format Specification for details
*/
typedef struct hostpkt_eng_rssi
{
  /* common preamble - part of every packet! */
  struct hostpkt_cmn cmn;

  /* echo back what data is enabled */
  unsigned int command_flags;
  unsigned int msctr;

  /* not ready yet */
  unsigned int ext_fb;

  /* lna high in 31:16, lna low in 15:0 */
  unsigned int lna_high_low;

  /* IF LNA Peak I in 31:16, IF LNA Peak Q in 15:0 */
  unsigned int iflna_peakI_peakQ;

  /* nb rssi*/
  unsigned short nb_rssi;

  /* wb rssi*/
  unsigned short wb_rssi;
}RFID_PACKET_ENG_RSSI;

/*
  hostpkt_eng_invstats

  Inventory statistics packet
*/
typedef struct hostpkt_eng_invstats
{
  /* common preamble - part of every packet! */
  struct hostpkt_cmn cmn;

  INT32U queries;
  INT32U rn16_to;
  INT32U rn16_rcv;
  INT32U epc_crcerr;
  INT32U epc_rcvto;
  INT32U epc_good;
}RFID_PACKET_ENG_INVSTATS;

/*
  hostpkt_eng_bertest_results

  BER Test packet
*/
typedef struct hostpkt_eng_bertest_result
{
  /* common preamble - part of every packet! */
  struct hostpkt_cmn cmn;

  INT16U requested_bit_len;
  INT16U received_bit_len;
  INT8U rx_buffer[64]; /* Up to 512 bits of response data */
  INT32U rfu0;
  INT32U rfu1;
} RFID_PACKET_ENG_BERTSTRESULT;

/*
  hostpkt_nvmemupdcfg
  Non-volatile memory update configuration parameters packet
  Sent after the mac receives a CMD_NV_MEM_UPDATE but before
  it enters NV MEM UPDATE MODE
*/
typedef struct hostpkt_nvmemupdcfg
{
  /* common preamble - part of every packet! */
  struct hostpkt_cmn cmn;
  /* the maximum length of any packet sent while in nv mem update mode*/
  INT32U max_pkt_len;
  /* the version of nv mem update protocol */
  INT16U nv_upd_ver;
  /* reserved field */
  INT16U res0;
} RFID_PACKET_NVMEMUPDCFG;

/*
  hostpkt_engtstpat

  Engineering Test Pattern Packets
  See RFID Common Packet Format Specification for details
*/
typedef struct hostpkt_engtestpat
{
  /* common preamble - part of every packet!*/
  struct hostpkt_cmn cmn;

  /* data - variable length!*/
  INT32U data[1];
}RFID_PACKET_ENGTESTPAT_READ;


/*
  hostpkt_lprof_read_reg

  MAC Link-Profile Read Register Packet

  See RFID Common Packet Format Specification for details
*/
typedef struct hostpkt_lprof_read_reg
{
  /* common preamble - part of every packet! */
  struct hostpkt_cmn cmn;
  /* selector of the link profile that was being read via LPROFREADREG command*/
  INT16U sel;
  /* address that was being read via LPROFREADREG command*/
  INT16U addr;
  /* the data returned for the read*/
  INT16U data;
  /* reserved for future use */
  INT16U rfu;
}RFID_PACKET_LPROF_READ;

/*
  hostpkt_sjc_scanresult

  Packet for reporting the results of SJC scanning

  See RFID Common Packet Format Specification for details
*/
typedef struct hostpkt_sjc_scanresult
{
  /* common preamble - part of every packet! */
  struct hostpkt_cmn cmn;
  INT32U ms_ctr;
  INT32U algo_time;
  INT8U origin_I;
  INT8U origin_Q;
  INT8U step_size;
  INT8U scan_size;
  INT8U scale_out_I;
  INT8U scale_out_Q;
  INT16U rssi_pre_sjc;
  INT16U rssi_post_sjc;
  INT8U external_lo;
  INT8U rfu;
}RFID_PACKET_SJC_SCANRESULT;

/*
  hostpkt_csm_protsched_sm_status

  Packet for reporting the state machine status of the Protocol Scheduler.

  See RFID Common Packet Format Specification for details
*/
typedef struct hostpkt_csm_protsched_sm_status
{
  /* common preamble - part of every packet! */
  struct hostpkt_cmn cmn;

  /* Firmware ms counter when packet is sent out */
  INT32U ms_ctr;

  /* Current Protocol Scheduler State */
  INT8U protsched_state;

  /* Current Frequency Index */
  INT8U curr_freq_idx;

  /* Current Antenna Index */
  INT8U curr_ant_idx;

  /* Reserved 0 */
  INT8U rfu0;

  /* Reserved 1 */
  INT32U rfu1;

}RFID_PACKET_CSM_PROTSCHED_SM_STATUS;

/*
  hostpkt_csm_protsched_lbt_status

  Packet for reporting Protocol Scheduler LBT status.

  See RFID Common Packet Format Specification for details
*/
typedef struct hostpkt_csm_protsched_lbt_status
{
  /* common preamble - part of every packet! */
  struct hostpkt_cmn cmn;

  /* Firmware ms counter when packet is sent out */
  INT32U ms_ctr;

  /* flag to determine if an interferer is present */
  INT8U intf_present;

  /* RSSI level as detected by the LBT mechanism */
  INT8U rssi;

  /* Current Frequency Index */
  INT8U curr_freq_idx;

  /* Current Antenna Index */
  INT8U curr_ant_idx;

  /* Gain Settings */
  INT16U ana_ctrl;

  /* Offset at which interferer is detected */
  INT8U lbt_chans_offset;

  /* Reserved 0 */
  INT8U rfu0;

  /* Reserved 1 */
  INT32U rfu1;

}RFID_PACKET_CSM_PROTSCHED_LBT_STATUS;

/*
  hostpkt_eng_xy

  Eng XY packet

  See RFID Common Packet Format Specification for details
*/
typedef struct hostpkt_eng_xy
{
  /* common preamble - part of every packet! */
  struct hostpkt_cmn cmn;

  /* Calculation Id */
  unsigned int id;

  /* Calculation Time us */
  unsigned int calculationTimeUs;

  /* x */
  unsigned int x;

  /* y */
  unsigned int y;

  /* Reserved 0 */
  unsigned int res0;

  /* Reserved 1 */
  unsigned int res1;

  /* Reserved 2 */
  unsigned int res2;
  
  /* Reserved 3 */
  unsigned int res3;

}RFID_PACKET_ENG_XY;

/* TODO - Add packets for MACSNAP and TILSNAP register snapshot commands */


/* The length of the individual packets in the unit of measure that is present*/
/* in the common packet format preamble.                                      */
#define RFID_PACKET_LEN_COMMAND_BEGIN                                         \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_COMMAND_BEGIN)
#define RFID_PACKET_LEN_COMMAND_END                                           \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_COMMAND_END)
#define RFID_PACKET_LEN_CYCLE_BEGIN                                           \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_ANTENNA_CYCLE_BEGIN)
#define RFID_PACKET_LEN_CYCLE_END                                             \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_ANTENNA_CYCLE_END)
#define RFID_PACKET_LEN_CYCLE_BEGIN_DIAGS                                     \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_ANTENNA_CYCLE_BEGIN_DIAGS)
#define RFID_PACKET_LEN_CYCLE_END_DIAGS                                       \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_ANTENNA_CYCLE_END_DIAGS)
#define RFID_PACKET_LEN_ANTENNA_BEGIN                                         \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_ANTENNA_BEGIN)
#define RFID_PACKET_LEN_ANTENNA_END                                           \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_ANTENNA_END)
#define RFID_PACKET_LEN_ANTENNA_BEGIN_DIAGS                                   \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_ANTENNA_BEGIN_DIAGS)
#define RFID_PACKET_LEN_ANTENNA_END_DIAGS                                     \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_ANTENNA_END_DIAGS)
#define RFID_PACKET_LEN_INVENTORY_CYCLE_BEGIN                                 \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_INVENTORY_CYCLE_BEGIN)
#define RFID_PACKET_LEN_INVENTORY_CYCLE_END                                   \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_INVENTORY_CYCLE_END)
#define RFID_PACKET_LEN_INVENTORY_CYCLE_END_DIAGS                             \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_INVENTORY_CYCLE_END_DIAGS)
#define RFID_PACKET_LEN_INVENTORY_ROUND_BEGIN                                 \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_18K6C_INVENTORY_ROUND_BEGIN)
#define RFID_PACKET_LEN_INVENTORY_ROUND_END                                   \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_18K6C_INVENTORY_ROUND_END)
#define RFID_PACKET_LEN_INVENTORY_ROUND_BEGIN_DIAGS                           \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_18K6C_INVENTORY_ROUND_BEGIN_DIAGS)
#define RFID_PACKET_LEN_INVENTORY_ROUND_END_DIAGS                             \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_18K6C_INVENTORY_ROUND_END_DIAGS)
/* Since this is a variable-length packet, have to subtract off 1 unit for the*/
/* single-element array at the end                                            */
#define RFID_PACKET_LEN_MIN_18K6C_INVENTORY                                   \
    (RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_18K6C_INVENTORY) - 1)
#define RFID_PACKET_LEN_18K6C_INVENTORY_DIAGS                                 \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_18K6C_INVENTORY_DIAGS)
/* Since this is a variable-length packet, have to subtract off 1 unit for the*/
/* single-element array at the end                                            */
#define RFID_PACKET_LEN_MIN_18K6C_TAG_ACCESS                                  \
    (RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_18K6C_TAG_ACCESS) - 1)
#define RFID_PACKET_LEN_FREQUENCY_HOP_DIAGS                                   \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_FREQUENCY_HOP_DIAGS)
#define RFID_PACKET_LEN_NONCRITICAL_FAULT                                     \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_NONCRITICAL_FAULT)
#define RFID_PACKET_LEN_MBP_READ_REG                                          \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_MBP_READ)
#define RFID_PACKET_LEN_GPIO_READ                                             \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_GPIO_READ)
#define RFID_PACKET_LEN_OEMCFG_READ                                           \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_OEMCFG_READ)
#define RFID_PACKET_LEN_ENG_RSSI                                          \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_ENG_RSSI)
#define RFID_PACKET_LEN_ENG_INVSTATS                                          \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_ENG_INVSTATS)
#define RFID_PACKET_LEN_MIN_ENGTESTPAT                                         \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_ENGTESTPAT)
#define RFID_PACKET_LEN_ENG_BERTSTRESULT                                      \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_ENG_BERTSTRESULT)
#define RFID_PACKET_LEN_CARRIER_INFO                                          \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_CARRIER_INFO)
#define RFID_PACKET_LEN_COMMAND_ACTIVE                                          \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_COMMAND_ACTIVE)
#define RFID_PACKET_LEN_NVMEMUPDCFG                                           \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_NVMEMUPDCFG)
#define RFID_PACKET_LEN_LPROF_READ_REG                                        \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_LPROF_READ)
#define RFID_PACKET_LEN_TX_RANDOM_DATA_STATUS                                      \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_TX_RANDOM_DATA_STATUS)
#define RFID_PACKET_LEN_SJC_SCANRESULT                                        \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_SJC_SCANRESULT)
#define RFID_PACKET_LEN_CSM_PROTSCHED_SM_STATUS                               \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_CSM_PROTSCHED_SM_STATUS)
#define RFID_PACKET_LEN_CSM_PROTSCHED_LBT_STATUS                              \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_CSM_PROTSCHED_LBT_STATUS)
#define RFID_PACKET_LEN_ENG_XY                                          \
    RFID_PACKET_ADDITIONAL_UNIT_LEN(RFID_PACKET_ENG_XY)

/* The version nubmers to put in the common packet headers for different      */
/* packets.                                                                   */
#define RFID_PACKET_VER_COMMAND_BEGIN               0x01
#define RFID_PACKET_VER_COMMAND_END                 0x01
#define RFID_PACKET_VER_CYCLE_BEGIN                 0x01
#define RFID_PACKET_VER_CYCLE_END                   0x01
#define RFID_PACKET_VER_CYCLE_BEGIN_DIAGS           0x01
#define RFID_PACKET_VER_CYCLE_END_DIAGS             0x01
#define RFID_PACKET_VER_ANTENNA_BEGIN               0x01
#define RFID_PACKET_VER_ANTENNA_END                 0x01
#define RFID_PACKET_VER_ANTENNA_BEGIN_DIAGS         0x01
#define RFID_PACKET_VER_ANTENNA_END_DIAGS           0x01
#define RFID_PACKET_VER_INVENTORY_CYCLE_BEGIN       0x01
#define RFID_PACKET_VER_INVENTORY_CYCLE_END         0x01
#define RFID_PACKET_VER_INVENTORY_CYCLE_END_DIAGS   0x01
#define RFID_PACKET_VER_INVENTORY_ROUND_BEGIN       0x01
#define RFID_PACKET_VER_INVENTORY_ROUND_END         0x01
#define RFID_PACKET_VER_INVENTORY_ROUND_BEGIN_DIAGS 0x01
#define RFID_PACKET_VER_INVENTORY_ROUND_END_DIAGS   0x01
#define RFID_PACKET_VER_18K6C_INVENTORY             0x01
#define RFID_PACKET_VER_18K6C_INVENTORY_DIAGS       0x01
#define RFID_PACKET_VER_18K6C_TAG_ACCESS            0x01
#define RFID_PACKET_VER_FREQUENCY_HOP_DIAGS         0x01
#define RFID_PACKET_VER_NONCRITICAL_FAULT           0x01
#define RFID_PACKET_VER_MBP_READ_REG                0x01
#define RFID_PACKET_VER_GPIO_READ                   0x01
#define RFID_PACKET_VER_OEMCFG_READ                 0x01
#define RFID_PACKET_VER_ENG_RSSI                    0x01
#define RFID_PACKET_VER_ENG_INVSTATS                0x01
#define RFID_PACKET_VER_ENGTESTPAT                  0x01
#define RFID_PACKET_VER_ENG_BERTSTRESULT            0x01
#define RFID_PACKET_VER_CARRIER_INFO                0x01
#define RFID_PACKET_VER_COMMAND_ACTIVE              0x01
#define RFID_PACKET_VER_NVMEMUPDCFG                 0x01
#define RFID_PACKET_VER_LPROF_READ_REG              0x01
#define RFID_PACKET_VER_CYCCFG_EVT                  0x01
#define RFID_PACKET_VER_DEBUG                       0x01
#define RFID_PACKET_VER_TX_RANDOM_DATA_STATUS       0x01
#define RFID_PACKET_VER_SJC_SCANRESULT              0x01
#define RFID_PACKET_VER_CSM_PROTSCHED_SM_STATUS     0x01
#define RFID_PACKET_VER_CSM_PROTSCHED_LBT_STATUS    0x01
#define RFID_PACKET_VER_ENG_XY                      0x01

/* Macro to get the complete TX byte length of a packet including the common  */
/* fields                                                                     */
#define RFID_PACKET_FULL_BYTE_LEN(pktlen)                                    \
    ((INT32U) ((RFID_PACKET_CMN_LEN * RFID_PACKET_BYTES_PER_UNIT) +                  \
               ((pktlen) * RFID_PACKET_BYTES_PER_UNIT)))

/* Macro to convert packet specific length to a byte len                      */
#define RFID_PACKET_PKT_BYTE_LEN(pktlen)                                     \
    ((INT32U) ((pktlen) * RFID_PACKET_BYTES_PER_UNIT))


/* Host Packet Helper Functions */
extern int SendPacket_ENG_XY(struct macops*, unsigned int id, unsigned int calculationTimeUs, 
                             unsigned int x, unsigned int y, 
                             unsigned int res0, unsigned int res1, unsigned int res2, unsigned int res3);


#endif /* H_HOSTPKTS_H */
