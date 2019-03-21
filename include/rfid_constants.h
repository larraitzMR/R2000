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
 * $Id: rfid_constants.h 67084 2011-03-24 21:50:15Z dshaheen $
 * 
 * Description:
 *     This is the RFID Library header file that specifies the enumeration
 *     constants.
 *     
 *
 *****************************************************************************
 */

#ifndef RFID_CONSTANTS_H_INCLUDED
#define RFID_CONSTANTS_H_INCLUDED

#include "rfid_types.h"
#include "rfid_library_export.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The constant for an invalid radio handle                                   */
#define RFID_INVALID_RADIO_HANDLE ((RFID_RADIO_HANDLE)0)

/* Timeout in seconds for detecting the radio has stopped responding.
 * This is set to twice the rate at which the CMD_ACTIVE packet should be 
 * output during command operations.
*/
#define RFID_CMD_RESPONSE_TIMEOUT (CMD_ACTIVE_TIMEOUT * 2)   

/* Timeout in milliseconds for detecting a failed response.  This applies 
 * when we know a response (possibly a register read response, or the rest of
 * a command response packet) is coming, so it is defined independent of the
 * CMD_ACTIVE_TIMEOUT.
*/
#define RFID_READ_TIMEOUT 3000

/******************************************************************************
 * Name: RFID_RADIO_OPERATION_MODE - The operation mode for the radio
 ******************************************************************************/
enum {
    RFID_RADIO_OPERATION_MODE_CONTINUOUS,
    RFID_RADIO_OPERATION_MODE_NONCONTINUOUS
};
typedef INT32U  RFID_RADIO_OPERATION_MODE;

/******************************************************************************
 * Name: RFID_RADIO_POWER_STATE - The power state for a radio.
 ******************************************************************************/
enum {
    RFID_RADIO_POWER_STATE_FULL,
    RFID_RADIO_POWER_STATE_STANDBY
};
typedef INT32U  RFID_RADIO_POWER_STATE;

/******************************************************************************
 * Name: RFID_ANTENNA_PORT_STATE - The state of an antenna port.
 ******************************************************************************/
enum {
    RFID_ANTENNA_PORT_STATE_DISABLED,
    RFID_ANTENNA_PORT_STATE_ENABLED
};
typedef INT32U  RFID_ANTENNA_PORT_STATE;

/******************************************************************************
 * Name: RFID_18K6C_MODULATION_TYPE - The ISO 18000-6C modulation types.
 ******************************************************************************/
enum {
    RFID_18K6C_MODULATION_TYPE_DSB_ASK,
    RFID_18K6C_MODULATION_TYPE_SSB_ASK,
    RFID_18K6C_MODULATION_TYPE_PR_ASK
};
typedef INT32U  RFID_18K6C_MODULATION_TYPE;

/******************************************************************************
 * Name: RFID_18K6C_DATA_0_1_DIFFERENCE - The Tari difference between data zero
 *       and data one.
 ******************************************************************************/
enum {
    RFID_18K6C_DATA_0_1_DIFFERENCE_HALF_TARI,
    RFID_18K6C_DATA_0_1_DIFFERENCE_ONE_TARI
};
typedef INT32U  RFID_18K6C_DATA_0_1_DIFFERENCE;

/******************************************************************************
 * Name: RFID_18K6C_DIVIDE_RATIO - The ISO 18000-6C divide ratios.
 ******************************************************************************/
enum {
    RFID_18K6C_DIVIDE_RATIO_8,
    RFID_18K6C_DIVIDE_RATIO_64DIV3
};
typedef INT32U  RFID_18K6C_DIVIDE_RATIO;

/******************************************************************************
 * Name: RFID_18K6C_MILLER_NUMBER - The ISO 18000-6C Miller encoding subcarrier
 *       cycles.
 ******************************************************************************/
enum {
    RFID_18K6C_MILLER_NUMBER_FM0,
    RFID_18K6C_MILLER_NUMBER_2,
    RFID_18K6C_MILLER_NUMBER_4,
    RFID_18K6C_MILLER_NUMBER_8
};
typedef INT32U  RFID_18K6C_MILLER_NUMBER;

/******************************************************************************
 * Name: RFID_RADIO_PROTOCOL_ISO18K6C - The tag protocols.9
 ******************************************************************************/
enum {
    RFID_RADIO_PROTOCOL_ISO18K6C
};
typedef INT32U RFID_RADIO_PROTOCOL;

/******************************************************************************
 * Name: RFID_18K6C_MEMORY_BANK - The RFID tag's memory bank.
 ******************************************************************************/
enum {
    RFID_18K6C_MEMORY_BANK_RESERVED,
    RFID_18K6C_MEMORY_BANK_EPC,
    RFID_18K6C_MEMORY_BANK_TID,
    RFID_18K6C_MEMORY_BANK_USER
};
typedef INT32U  RFID_18K6C_MEMORY_BANK;

/******************************************************************************
 * Name:  RFID_18K6C_TARGET - Which one of a tag's flags that will be modified
 *        by a select operation.
 ******************************************************************************/
enum {
    RFID_18K6C_TARGET_INVENTORY_S0,
    RFID_18K6C_TARGET_INVENTORY_S1,
    RFID_18K6C_TARGET_INVENTORY_S2,
    RFID_18K6C_TARGET_INVENTORY_S3,
    RFID_18K6C_TARGET_SELECTED 
};
typedef INT32U  RFID_18K6C_TARGET;

/******************************************************************************
 * Name:  RFID_18K6C_ACTION - The action which will be performed upon the tag
 *        populations (i.e, matching and non-matching) during the select.  The
 *        constants are named RFID_18K6C_ACTION_xxx_yyy where "xxx" is the
 *        action to be applied to matching tags and "yyy" is the action to be
 *        applied to non-matching tags.  Actions are:
 *        ASL - Assert SL
 *        INVA - Set inventoried flag to A
 *        DSL - Deassert SL
 *        INVB - Set inventoried flag to B
 *        NSL - Negate SL
 *        INVS - Switch inventoried flag (A -> B, B -> A)
 *        NOTHING - Do nothing
 ******************************************************************************/
enum {
    /* Match – Assert SL or inventoried -> A       */
    /* Non-Match – Deassert SL or inventoried -> B */
    RFID_18K6C_ACTION_ASLINVA_DSLINVB,
    /* Match – Assert SL or inventoried -> A       */
    /* Non-Match – Nothing                         */
    RFID_18K6C_ACTION_ASLINVA_NOTHING,
    /* Match – Nothing                             */
    /* Non-Match – Deassert SL or inventoried -> B */
    RFID_18K6C_ACTION_NOTHING_DSLINVB, 
    /* Match – Negate SL or (A -> B, B -> A)       */
    /* Non-Match – Nothing                         */
    RFID_18K6C_ACTION_NSLINVS_NOTHING,
    /* Match – Deassert SL or inventoried -> B     */
    /* Non-Match – Assert SL or inventoried -> A   */
    RFID_18K6C_ACTION_DSLINVB_ASLINVA,
    /* Match - Deassert SL or inventoried -> B     */
    /* Non-Match - Nothing                         */
    RFID_18K6C_ACTION_DSLINVB_NOTHING,
    /* Match – Nothing                             */
    /* Non-Match – Assert SL or inventoried -> A   */
    RFID_18K6C_ACTION_NOTHING_ASLINVA,
    /* Match – Nothing                             */
    /* Non-Match – Negate SL or (A -> B, B -> A)   */
    RFID_18K6C_ACTION_NOTHING_NSLINVS
};
typedef INT32U  RFID_18K6C_ACTION;

/******************************************************************************
 * Name:  RFID_18K6C_SELECTED - The states for a tag's SL flag.
 ******************************************************************************/
enum {
    RFID_18K6C_SELECT_ALL        = 1,
    RFID_18K6C_SELECT_DEASSERTED = 2,
    RFID_18K6C_SELECT_ASSERTED   = 3 
};
typedef INT32U  RFID_18K6C_SELECTED;

/******************************************************************************
 * Name:  RFID_18K6C_INVENTORY_SESSION - The ISO 18000-6C inventory session
 *        flags that are available.
 ******************************************************************************/
enum {
    RFID_18K6C_INVENTORY_SESSION_S0,
    RFID_18K6C_INVENTORY_SESSION_S1,
    RFID_18K6C_INVENTORY_SESSION_S2,
    RFID_18K6C_INVENTORY_SESSION_S3
};
typedef INT32U  RFID_18K6C_INVENTORY_SESSION;

/******************************************************************************
 * Name:  RFID_18K6C_INVENTORY_SESSION_TARGET - The valid states for a tag's ISO
 *        18000-6C inventory flags.
 ******************************************************************************/
enum {
    RFID_18K6C_INVENTORY_SESSION_TARGET_A,
    RFID_18K6C_INVENTORY_SESSION_TARGET_B 
};
typedef INT32U  RFID_18K6C_INVENTORY_SESSION_TARGET;

/******************************************************************************
 * Name:  RFID_18K6C_SINGULATION_ALGORITHM - The valid singulation algorithms.
 ******************************************************************************/
enum {
    RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ             = 0,
    RFID_18K6C_SINGULATION_ALGORITHM_DYNAMICQ           = 1
};
typedef INT32U  RFID_18K6C_SINGULATION_ALGORITHM;

/******************************************************************************
 * Name:  RFID_18K6C_WRITE_TYPE - The type of tag write operation that will be
 *        performed.
 ******************************************************************************/
enum {
    RFID_18K6C_WRITE_TYPE_SEQUENTIAL,
    RFID_18K6C_WRITE_TYPE_RANDOM
};
typedef INT32U RFID_18K6C_WRITE_TYPE;

/******************************************************************************
 * Name:  RFID_18K6C_TAG_PWD_PERM - The ISO 18000-6C tag password permission
 *        values.
 ******************************************************************************/
enum {
    RFID_18K6C_TAG_PWD_PERM_ACCESSIBLE,
    RFID_18K6C_TAG_PWD_PERM_ALWAYS_ACCESSIBLE,
    RFID_18K6C_TAG_PWD_PERM_SECURED_ACCESSIBLE,
    RFID_18K6C_TAG_PWD_PERM_ALWAYS_NOT_ACCESSIBLE,
    RFID_18K6C_TAG_PWD_PERM_NO_CHANGE
};
typedef INT32U  RFID_18K6C_TAG_PWD_PERM;

/******************************************************************************
 * Name:  RFID_18K6C_TAG_PWD_PERM - The ISO 18000-6C tag memory bank 
 *        permission values.
 ******************************************************************************/
enum {
    RFID_18K6C_TAG_MEM_PERM_WRITEABLE,
    RFID_18K6C_TAG_MEM_PERM_ALWAYS_WRITEABLE,
    RFID_18K6C_TAG_MEM_PERM_SECURED_WRITEABLE,
    RFID_18K6C_TAG_MEM_PERM_ALWAYS_NOT_WRITEABLE,
    RFID_18K6C_TAG_MEM_PERM_NO_CHANGE
};
typedef INT32U  RFID_18K6C_TAG_MEM_PERM;


/******************************************************************************
 * Name:  RFID_18K6C_QT_OPT_CMD_TYPE - The optional ISO 18000-6C tag access
 *        command that may accompany a QT command
 ******************************************************************************/
enum {
    RFID_18K6C_QT_OPT_NONE,
    RFID_18K6C_QT_OPT_READ,
    RFID_18K6C_QT_OPT_WRITE_TYPE_SEQUENTIAL,
    RFID_18K6C_QT_OPT_WRITE_TYPE_RANDOM
};
typedef INT32U  RFID_18K6C_QT_OPT_CMD_TYPE;


/******************************************************************************
 * Name:  RFID_18K6C_QT_CTRL_TYPE - The type of operation to be performed on 
 *        the QT control data
 ******************************************************************************/
enum {
    RFID_18K6C_QT_CTRL_READ,
    RFID_18K6C_QT_CTRL_WRITE
};
typedef INT32U  RFID_18K6C_QT_CTRL_TYPE;

/******************************************************************************
 * Name:  RFID_18K6C_QT_PERSISTENCE_TYPE - The persistence designation of the 
 *        QT control data, temporary is volatile, permanent is non-volatile
 ******************************************************************************/
enum {
    RFID_18K6C_QT_PERSISTENCE_TEMPORARY,
    RFID_18K6C_QT_PERSISTENCE_PERMANENT
};
typedef INT32U  RFID_18K6C_QT_PERSISTENCE_TYPE;


/******************************************************************************
 * Name:  RFID_18K6C_QT_SR_TYPE - The QT short range control type
 ******************************************************************************/
enum {
    RFID_18K6C_QT_SR_DISABLE,
    RFID_18K6C_QT_SR_ENABLE
};
typedef INT32U  RFID_18K6C_QT_SR_TYPE;

/******************************************************************************
 * Name:  RFID_18K6C_QT_MEMMAP_TYPE - The QT memory map control type
 ******************************************************************************/
enum {
    RFID_18K6C_QT_MEMMAP_PRIVATE,
    RFID_18K6C_QT_MEMMAP_PUBLIC
};
typedef INT32U  RFID_18K6C_QT_MEMMAP_TYPE;

/******************************************************************************
 * Name: RFID_RESPONSE_TYPE - The tag-access operation response type.
 ******************************************************************************/
enum {
    RFID_RESPONSE_TYPE_DATA = 0xFFFFFFFF
};
typedef INT32U RFID_RESPONSE_TYPE;

/******************************************************************************
 * Name: RFID_RESPONSE_MODE - The tag-access operation data-reporting mode.
 ******************************************************************************/
enum {
    RFID_RESPONSE_MODE_COMPACT     = 0x00000001,
    RFID_RESPONSE_MODE_NORMAL      = 0x00000003,
    RFID_RESPONSE_MODE_EXTENDED    = 0x00000007
};
typedef INT32U RFID_RESPONSE_MODE;

/******************************************************************************
 * Name:  RFID_MAC_RESET_TYPE - The types of resets available on the MAC.
 ******************************************************************************/
enum {
    RFID_MAC_RESET_TYPE_SOFT,
    RFID_MAC_RESET_TYPE_TO_BOOTLOADER
};
typedef INT32U  RFID_MAC_RESET_TYPE;

/******************************************************************************
 * Name:  RFID_MAC_REGION - The regulatory mode regions.
 ******************************************************************************/
enum {
    RFID_MAC_REGION_FCC_GENERIC,
    RFID_MAC_REGION_ETSI_GENERIC,
    RFID_MAC_REGION_ETSI_LBT
};
typedef INT32U  RFID_MAC_REGION;


/******************************************************************************
 * Name: RFID_REGISTER_TYPE - The type of register.
 ******************************************************************************/
enum {
    RFID_REGISTER_NORMAL = 0,
    RFID_REGISTER_BANKED,
    RFID_REGISTER_SELECTOR,
    RFID_REGISTER_RESERVED
};
typedef INT32U RFID_REGISTER_TYPE;

/******************************************************************************
 * Name: RFID_REGISTER_ACCESS_TYPE - The type of access for the 
 *       register (read/write, readonly, writeonly).
 ******************************************************************************/
enum {
    RFID_REGISTER_ACCESS_RW = 0,
    RFID_REGISTER_ACCESS_RO,
    RFID_REGISTER_ACCESS_WO
};
typedef INT32U RFID_REGISTER_ACCESS_TYPE;


/******************************************************************************
 * Name:  RFID_RADIO_GPIO_PIN - The bit mask values for the radio module GPIO
 *        pins
 ******************************************************************************/
#define SET_BIT(location) (((INT32U) 0x00000001) << location)
enum {
    RFID_RADIO_GPIO_PIN_0   = SET_BIT(0),
    RFID_RADIO_GPIO_PIN_1   = SET_BIT(1),
    RFID_RADIO_GPIO_PIN_2   = SET_BIT(2),
    RFID_RADIO_GPIO_PIN_3   = SET_BIT(3),
    RFID_RADIO_GPIO_PIN_4   = SET_BIT(4),
    RFID_RADIO_GPIO_PIN_5   = SET_BIT(5),
    RFID_RADIO_GPIO_PIN_6   = SET_BIT(6),
    RFID_RADIO_GPIO_PIN_7   = SET_BIT(7),
    RFID_RADIO_GPIO_PIN_8   = SET_BIT(8),
    RFID_RADIO_GPIO_PIN_9   = SET_BIT(9),
    RFID_RADIO_GPIO_PIN_10  = SET_BIT(10),
    RFID_RADIO_GPIO_PIN_11  = SET_BIT(11),
    RFID_RADIO_GPIO_PIN_12  = SET_BIT(12),
    RFID_RADIO_GPIO_PIN_13  = SET_BIT(13),
    RFID_RADIO_GPIO_PIN_14  = SET_BIT(14),
    RFID_RADIO_GPIO_PIN_15  = SET_BIT(15),
    RFID_RADIO_GPIO_PIN_16  = SET_BIT(16),
    RFID_RADIO_GPIO_PIN_17  = SET_BIT(17),
    RFID_RADIO_GPIO_PIN_18  = SET_BIT(18),
    RFID_RADIO_GPIO_PIN_19  = SET_BIT(19),
    RFID_RADIO_GPIO_PIN_20  = SET_BIT(20),
    RFID_RADIO_GPIO_PIN_21  = SET_BIT(21),
    RFID_RADIO_GPIO_PIN_22  = SET_BIT(22),
    RFID_RADIO_GPIO_PIN_23  = SET_BIT(23),
    RFID_RADIO_GPIO_PIN_24  = SET_BIT(24),
    RFID_RADIO_GPIO_PIN_25  = SET_BIT(25),
    RFID_RADIO_GPIO_PIN_26  = SET_BIT(26),
    RFID_RADIO_GPIO_PIN_27  = SET_BIT(27),
    RFID_RADIO_GPIO_PIN_28  = SET_BIT(28),
    RFID_RADIO_GPIO_PIN_29  = SET_BIT(29),
    RFID_RADIO_GPIO_PIN_30  = SET_BIT(30),
    RFID_RADIO_GPIO_PIN_31  = SET_BIT(31)
};
typedef INT32U  RFID_RADIO_GPIO_PIN;

/******************************************************************************
 * Flags for the RFID_18K6CTag* functions
 ******************************************************************************/
enum {
    RFID_FLAG_PERFORM_SELECT        = 0x00000001,
    RFID_FLAG_PERFORM_POST_MATCH    = 0x00000002
};

/******************************************************************************
 * Flags for the RFID_MacUpdateNonvolatileMemory function
 ******************************************************************************/
enum {
    RFID_FLAG_NVMEM_UPDATE_TEST = 0x00000001
};


/******************************************************************************
 * Name:  RFID_BLOCKWRITE_WORDS - The max words per BlockWrite transaction
 ******************************************************************************/
enum {
    RFID_BLOCKWRITE_MODE_AUTO        = 0,
    RFID_BLOCKWRITE_MODE_FORCE_ONE   = 1,
    RFID_BLOCKWRITE_MODE_FORCE_TWO   = 2 
};
typedef INT32U  RFID_BLOCKWRITE_MODE;


/******************************************************************************
 * Name: RFID_TAG_FOCUS  - Enables/Disables tag focus feature.
 ******************************************************************************/
enum {
    RFID_TAG_FOCUS_DISABLED,
    RFID_TAG_FOCUS_ENABLED
};
typedef INT32U   RFID_TAG_FOCUS;

/******************************************************************************
 * Name: RFID_FAST_ID  - Enables/Disables FastID feature.
 ******************************************************************************/
enum {
    RFID_FAST_ID_DISABLED,
    RFID_FAST_ID_ENABLED
};
typedef INT32U   RFID_FAST_ID;

#ifdef __cplusplus
}
#endif

#endif  /* #ifndef RFID_CONSTANTS_H_INCLUDED */
