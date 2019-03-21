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
 * $Id: macregs.h 67080 2011-03-24 20:15:21Z dshaheen $
 * 
 * Description:
 *
 *   header file defines the MAC <-> Host CPU Interface registers,
 *   bit access macros etc
 *   See also hostif.c - register memory is actually defined and accessed there
 *
 *****************************************************************************
 */
#ifndef H_MACREGS_H
#define H_MACREGS_H

#include <hostifregs.h>

/* special reg defines used by RFTC - point to other auto gend regs */
#define HST_RFTC_WS_TX_I_Q_BASE         HST_RFTC_WS_TX_I_Q_00
#define HST_RFTC_WS_TX_SD_H_USR_BASE    HST_RFTC_WS_TX_SD_H_USR_00

/* special defines related to antenna descriptor regs */
#define FIRST_ANT_DESC 0x0
#define LAST_ANT_DESC  0xF

/* special defines for freq chans */
#define FIRST_CHAN 0x0
#define LAST_CHAN  0x31

/* HST_INVALRD is a special register and doesn't actually have a register
   configured in physical memory - this address is returned by the host
   interface whenever a read to and invalid register is requested by the
   host */
#define HST_INVALRD         0xFFFF /* do not include in the 0xFF block! */

/* ***************************************************************************
  MACROS for register bit access
*****************************************************************************/

/* Macro for creating a mask of a supplied number of bits                   */
/* si - the number of bits in the mask                                      */
#define CREATE_MASK(si)         (((INT32U) 0xFFFFFFFF) >> (32 - (si)))

/* Macro for defining the mask for a field within a register (i.e., a mask  */
/* that is shifted to the proper location for the field.                    */
/* si - the number of bits in the bit field                                 */
/* sh - the number of bits the field is shifted left from the register LSb  */
#define BIT_FIELD_MASK(si, sh)  (CREATE_MASK(si) << (sh))

/* Macro for extracting a bit field from a register value.  The extracted   */
/* value will be shifted into the low-order bits of the resulting INT32U.   */
/* rv - the register value from which bit field will be extracted           */
/* si - the number of bits in the bit field                                 */
/* sh - the number of bits the field is shifted left from the register LSb  */
#define BIT_FIELD_EXTRACT(rv, si, sh)                                       \
    ((INT32U) ((((INT32U) (rv)) >> (sh)) & CREATE_MASK(si)))

/* Macro for creating an appopriate bit field value (i.e., takes a value    */
/* and shifts it into the INT32U so that it is in the proper location for   */
/* a register.                                                              */
/* va - the value to use for the bit field                                  */
/* si - the number of bits in the bit field                                 */
/* sh - the number of bits the field is shifted left from the register LSb  */
#define BIT_FIELD_CREATE(va, si, sh)                                        \
    ((INT32U) (((va) & CREATE_MASK(si)) << (sh)))

/* Macro for, given a current register value, creating a new register value */
/* in which only the requested bit field is changed (i.e., other bits are   */
/* left alone).                                                             */
/* rv - original register value                                             */
/* va - value to be set in bit field                                        */
/* si - the number of bits in the bit field                                 */
/* sh - the number of bits the field is shifted left from the register LSb  */
#define REGISTER_CREATE_VALUE(rv, va, si, sh)                               \
    (((rv) & ~BIT_FIELD_MASK(si,sh)) | BIT_FIELD_CREATE(va,si,sh))

/* Macro for, given a current register value, changing the value so that the*/
/* requested bit field is changed (i.e., other bits are left alone).        */
/* Note that modifies the register value in place.                          */
/* rv - register value which will have bit-field value set                  */
/* va - value to be set in bit field                                        */
/* si - the number of bits in the bit field                                 */
/* sh - the number of bits the field is shifted left from the register LSb  */
#define REGISTER_SET_VALUE(rv, va, si, sh)                                  \
    ((rv &= ~BIT_FIELD_MASK(si,sh)), (rv |= BIT_FIELD_CREATE(va,si,sh)))

/* Macros suitable for single-bit fields.                                   */
/* Macro for creating a single-bit mask (we could use the generic macro, but*/
/* will special-case single-bit fields.                                     */
/* sh - the number of bits the field is shifted left from the register LSb  */
#define CREATE_1BIT_MASK(sh)    ((INT32U) (0x00000001 << (sh)))

/* Macro for setting the bit in a register value                            */
/* rv - the register value that is to be modified                           */
/* sh - the number of bits the field is shifted left from the register LSb  */
#define REGISTER_SET_BIT(rv, sh) (rv |= CREATE_1BIT_MASK(sh))

/* Macro for clearing the bit in a register value                           */
/* rv - the register value that is to be modified                           */
/* sh - the number of bits the field is shifted left from the register LSb  */
#define REGISTER_CLR_BIT(rv, sh) (rv &= ~CREATE_1BIT_MASK(sh))

/* Macro for testing a bit in a register value                              */
/* rv - the register value that is to be tested                             */
/* sh  - the number of bits the field is shifted left from the register LSb */
#define REGISTER_TEST_BIT(rv, sh) ((rv) & CREATE_1BIT_MASK(sh))

/* MAC_VER register helper macros                                           */
/* The size of the bit fields in the MAC_VER register.                      */
#define MAC_VER_REL_SIZE    8
#define MAC_VER_MAINT_SIZE  8
#define MAC_VER_MIN_SIZE    8
#define MAC_VER_MAJ_SIZE    8
/* The number of bits that fields are left-shifted in the MAC_VER register. */
#define MAC_VER_REL_SHIFT   0
#define MAC_VER_MAINT_SHIFT (MAC_VER_REL_SHIFT   + MAC_VER_REL_SIZE)
#define MAC_VER_MIN_SHIFT   (MAC_VER_MAINT_SHIFT + MAC_VER_MAINT_SIZE)
#define MAC_VER_MAJ_SHIFT   (MAC_VER_MIN_SHIFT + MAC_VER_MIN_SIZE)
/* MAC_VER bit-field retrieval macros                                       */
/* rv - the value from the MAC_VER register                                 */
#define MAC_VER_GET_REL(rv)                                                 \
    BIT_FIELD_EXTRACT(rv, MAC_VER_REL_SIZE, MAC_VER_REL_SHIFT)
#define MAC_VER_GET_MAINT(rv)                                               \
    BIT_FIELD_EXTRACT(rv, MAC_VER_MAINT_SIZE, MAC_VER_MAINT_SHIFT)
#define MAC_VER_GET_MIN(rv)                                                 \
    BIT_FIELD_EXTRACT(rv, MAC_VER_MIN_SIZE, MAC_VER_MIN_SHIFT)
#define MAC_VER_GET_MAJ(rv)                                                 \
    BIT_FIELD_EXTRACT(rv, MAC_VER_MAJ_SIZE, MAC_VER_MAJ_SHIFT)

/* HST_DBG1 bit position defines, access and check macros */
#define HST_DBG1_REGDMP             (0x1 << 0x0)
#define HST_DBG1_SET_REGDMP(regval) (regval |= HST_DBG1_REGDMP)
#define HST_DBG1_CLR_REGDMP(regval) (regval &= (~HST_DBG1_REGDMP))
#define HST_DBG1_IS_REGDMP(regval)  (regval & HST_DBG1_REGDMP)

/* HST_TEST_INVENTORY_CFG register bit access macros */
#define HST_TEST_INVENTORY_CFG_CONTINUOUSTX_ENABLED     0x00000001
#define HST_TEST_INVENTORY_CFG_CONTINUOUSTX_DISABLED    0x00000000

#define HST_TEST_INVENTORY_CFG_IS_CONTINUOUSTX(regval)                  \
    ((regval) & HST_TEST_INVENTORY_CFG_CONTINUOUSTX_ENABLED)

/* HST_PROTSCH_LBTCFG register bit access macros */
#define HST_PROTSCH_LBTCFG_LBT_ENABLED        0x00000001
#define HST_PROTSCH_LBTCFG_LBT_DISABLED       0x00000000
#define HST_PROTSCH_LBTCFG_SCANMODE_ENABLED   0x00000002
#define HST_PROTSCH_LBTCFG_SCANMODE_DISABLED  0x00000000
#define HST_PROTSCH_LBTCFG_LBT_CHANS_ENABLED  0x00000030
#define HST_PROTSCH_LBTCFG_LBT_CHANS_DISABLED 0x00000000
#define HST_PROTSCH_LBTCFG_LBT_GAIN_ENABLED   0x00000040
#define HST_PROTSCH_LBTCFG_LBT_GAIN_DISABLED  0x00000000

#define HST_PROTSCH_LBTCFG_RFU1(val)                                    \
    ((INT32U) (((val) & 0x00000001) << 2))
#define HST_PROTSCH_LBTCFG_RFU2(val)                                    \
    ((INT32U) (((val) & 0x00000001) << 3))
#define HST_PROTSCH_LBTCFG_RFU3(val)                                    \
    ((INT32U) (((val) & 0x01FFFFFF) << 7))
#define HST_PROTSCH_LBTCFG_GET_RFU1(regval)                             \
    (((regval) >> 2) & 0x00000001)
#define HST_PROTSCH_LBTCFG_GET_RFU2(regval)                             \
    (((regval) >> 3) & 0x00000001)
#define HST_PROTSCH_LBTCFG_GET_RFU3(regval)                             \
    (((regval) >> 7) & 0x01FFFFFF)
#define HST_PROTSCH_LBTCFG_SET_RFU1(regval,val)                         \
    (regval = ((regval) & 0xFFFFFFFB) | HST_PROTSCH_LBTCFG_RFU1(val))
#define HST_PROTSCH_LBTCFG_SET_RFU2(regval,val)                         \
    (regval = ((regval) & 0xFFFFFFF7) | HST_PROTSCH_LBTCFG_RFU2(val))
#define HST_PROTSCH_LBTCFG_SET_RFU3(regval,val)                         \
    (regval = ((regval) & 0x0000007F) | HST_PROTSCH_LBTCFG_RFU3(val))

#define HST_PROTSCH_LBTCFG_IS_LBT(regval)                               \
    ((regval) & HST_PROTSCH_LBTCFG_LBT_ENABLED)
#define HST_PROTSCH_LBTCFG_IS_SCANMODE(regval)                          \
    ((regval) & HST_PROTSCH_LBTCFG_SCANMODE_ENABLED)
#define HST_PROTSCH_LBTCFG_GET_LBT_CHANS(regval)                        \
    ((regval & HST_PROTSCH_LBTCFG_LBT_CHANS_ENABLED) >> 4)
#define HST_PROTSCH_LBTCFG_GET_LBT_GAIN(regval)                         \
    ((regval & HST_PROTSCH_LBTCFG_LBT_GAIN_ENABLED) >> 6)
#define HST_PROTSCH_LBTCFG_SET_LBT_ENABLED(regval)                      \
    (regval |= HST_PROTSCH_LBTCFG_LBT_ENABLED)
#define HST_PROTSCH_LBTCFG_SET_LBT_DISABLED(regval)                     \
    (regval &= ~HST_PROTSCH_LBTCFG_LBT_ENABLED)
#define HST_PROTSCH_LBTCFG_SET_SCANMODE_ENABLED(regval)                 \
    (regval |= HST_PROTSCH_LBTCFG_SCANMODE_ENABLED)
#define HST_PROTSCH_LBTCFG_SET_SCANMODE_DISABLED(regval)                \
    (regval &= ~HST_PROTSCH_LBTCFG_SCANMODE_ENABLED)
#define HST_PROTSCH_LBTCFG_SET_LBT_CHANS(regval,ch)                     \
    (regval = ((regval & ~HST_PROTSCH_LBTCFG_LBT_CHANS_ENABLED) | (ch << 4)))
#define HST_PROTSCH_LBTCFG_SET_LBT_GAIN_ENABLED(regval)                 \
    (regval |= HST_PROTSCH_LBTCFG_LBT_GAIN_ENABLED)
#define HST_PROTSCH_LBTCFG_SET_LBT_GAIN_DISABLED(regval)                \
    (regval &= ~HST_PROTSCH_LBTCFG_LBT_GAIN_ENABLED)

/* HST_PROTSCH_LBTRSSI register bit access macros */
#define HST_PROTSCH_LBTRSSI_LBTTHR(val)                                  \
    ((INT32U) ((val) & 0x000000FF))
#define HST_PROTSCH_LBTRSSI_GET_LBTTHR(regval)                           \
    ((regval) & 0x000000FF)
#define HST_PROTSCH_LBTRSSI_SET_LBTTHR(regval,val)                       \
    (regval = ((regval) & 0xFFFFFF00) | HST_PROTSCH_LBTRSSI_LBTTHR(val))

/* HST_CTR_GCFG register bit access & defs */
#define HST_CTR1 0x1
#define HST_CTR2 0x2
#define HST_CTR3 0x4
#define HST_CTR4 0x8
#define HST_ALLCTRS 0xF
#define HST_CTR_GCFG_IS_ANY_CTR_EN(regval)           (regval & HST_ALLCTRS)
#define HST_CTR_GCFG_IS_CTR1_EN(regval)              (regval & 0x1)
#define HST_CTR_GCFG_IS_CTR2_EN(regval)              (regval & 0x2)
#define HST_CTR_GCFG_IS_CTR3_EN(regval)              (regval & 0x4)
#define HST_CTR_GCFG_IS_CTR4_EN(regval)              (regval & 0x8)
#define HST_CTR_GCFG_SET_CTR1_EN(regval)             (regval |= 0x1)
#define HST_CTR_GCFG_SET_CTR2_EN(regval)             (regval |= 0x2)
#define HST_CTR_GCFG_SET_CTR3_EN(regval)             (regval |= 0x4)
#define HST_CTR_GCFG_SET_CTR4_EN(regval)             (regval |= 0x8)
#define HST_CTR_GCFG_SET_CTR1_DIS(regval)            (regval &= ~0x1)
#define HST_CTR_GCFG_SET_CTR2_DIS(regval)            (regval &= ~0x2)
#define HST_CTR_GCFG_SET_CTR3_DIS(regval)            (regval &= ~0x4)
#define HST_CTR_GCFG_SET_CTR4_DIS(regval)            (regval &= ~0x8)

/* Macros for accessing HST_CTRx_CFG regs */
/* type value is currently in the bottom nibble*/
#define HST_CTRx_GET_TYPE(regval)  (regval & 0xF)
#define HST_CTRx_SET_TYPE(regval,type)  (regval = (regval &= ~0xF) | type)

/* TYPE values that must be used HST_CTRx_CFG regs */
/* for packet types - the packet type value is in the upper 16 bits of
   the HST_CTRx_CFG register */
#define HST_CTRx_TYPE_RFID_PKT     0x1

/* HST_PWRMGMT register helper macros                                       */
/* The size of the bit fields in the HST_PWRMGMT register.                  */
#define HST_PWRMGMT_MODE_SIZE   3
#define HST_PWRMGMT_RFU1_SIZE   29
/* The number of bits that fields are left-shifted in the HST_PWRMGMT       */
/* register.                                                                */
#define HST_PWRMGMT_MODE_SHIFT  0
#define HST_PWRMGMT_RFU1_SHIFT  (HST_PWRMGMT_MODE_SHIFT + HST_PWRMGMT_MODE_SIZE)
/* Constants for HST_PWRMGMT register bit fields (note that the values are  */
/* already shifted into the low-order bits of the constant.                 */
#define HST_PWRMGMT_MODE_NORMAL             0x0
#define HST_PWRMGMT_MODE_LOWPOWER_STANDBY   0x1
#define HST_PWRMGMT_MODE_MAXVALID HST_PWRMGMT_MODE_LOWPOWER_STANDBY

/* HST_PWRMGMT bit-field retrieval and testing macros                       */
/* rv - the value from the HST_PWRMGMT register                             */
#define HST_PWRMGMT_GET_MODE(rv)                                            \
    BIT_FIELD_EXTRACT(rv, HST_PWRMGMT_MODE_SIZE, HST_PWRMGMT_MODE_SHIFT)
#define HST_PWRMGMT_GET_RFU1(rv)                                            \
    BIT_FIELD_EXTRACT(rv, HST_PWRMGMT_RFU1_SIZE, HST_PWRMGMT_RFU1_SHIFT)
/* HST_PWRMGMT bit-field value creation macros                              */
/* va - the value to put into the bit field                                 */
#define HST_PWRMGMT_MODE(va)                                                \
    BIT_FIELD_CREATE(va, HST_PWRMGMT_MODE_SIZE, HST_PWRMGMT_MODE_SHIFT)
#define HST_PWRMGMT_RFU1(va)                                                \
    BIT_FIELD_CREATE(va, HST_PWRMGMT_RFU1_SIZE, HST_PWRMGMT_RFU1_SHIFT)
/* HST_PWRMGMT register value modification macros (i.e., will modify the    */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_PWRMGMT_SET_MODE(rv, va)                                        \
    REGISTER_SET_VALUE(rv, va, HST_PWRMGMT_MODE_SIZE, HST_PWRMGMT_MODE_SHIFT)
#define HST_PWRMGMT_SET_RFU1(rv, va)                                        \
    REGISTER_SET_VALUE(rv, va, HST_PWRMGMT_RFU1_SIZE, HST_PWRMGMT_RFU1_SHIFT)

/* HST_CMNDIAGS register helper macros                                      */


// packet control flags
#define DIAG_PACKETS_ENABLED             0x03 
#define STATUS_PACKETS_ENABLED           0x02 
#define INV_CYCLE_DIAG_PACKETS_ENABLED   0x05  
#define INV_ROUND_DIAG_PACKETS_ENABLED   0x09  
#define INV_PACKETS_WITH_ACCESS_ENABLED  0x10
#define CARRIER_DIAG_PACKETS_ENABLED     0x21 
#define ACCESS_DIAG_PACKETS_ENABLED      0x41
#define RANDOMCW_DIAG_PACKETS_ENABLED    0x81
#define CSM_DIAG_PACKETS_ENABLED         0x101
#define CMD_ACTIVE_PACKETS_ENABLED       0x200

#define PACKET_ENABLED(macops, controlFlags)  \
           (controlFlags == (macops->hst_cmndiags & controlFlags))

/* The size of the bit fields in the HST_CMNDIAGS register.                 */
#define HST_CMNDIAGS_EN_DIAGS_SIZE          1
#define HST_CMNDIAGS_EN_STATUS_SIZE         1
#define HST_CMNDIAGS_EN_CYCLE_SIZE          1
#define HST_CMNDIAGS_EN_ROUND_SIZE          1
#define HST_CMNDIAGS_EN_INVRESP_SIZE        1
#define HST_CMNDIAGS_EN_CWDIAGS_SIZE        1
#define HST_CMNDIAGS_EN_ACCESSDIAGS_SIZE    1
#define HST_CMNDIAGS_EN_RANDOMCWDIAGS_SIZE  1
#define HST_CMNDIAGS_EN_CSMDIAGS_SIZE       1
#define HST_CMNDIAGS_EN_COMMANDACTIVE_SIZE  1
#define HST_CMNDIAGS_RFU2_SIZE              22

/* The number of bits that fields are left-shifted in the HST_CMNDIAGS      */
/* register.                                                                */
#define HST_CMNDIAGS_EN_DIAGS_SHIFT 0
#define HST_CMNDIAGS_EN_STATUS_SHIFT                                                   \
            (HST_CMNDIAGS_EN_DIAGS_SHIFT + HST_CMNDIAGS_EN_DIAGS_SIZE)
#define HST_CMNDIAGS_EN_CYCLE_SHIFT                                                    \
            (HST_CMNDIAGS_EN_STATUS_SHIFT + HST_CMNDIAGS_EN_STATUS_SIZE)
#define HST_CMNDIAGS_EN_ROUND_SHIFT                                                    \
            (HST_CMNDIAGS_EN_CYCLE_SHIFT + HST_CMNDIAGS_EN_CYCLE_SIZE)
#define HST_CMNDIAGS_EN_INVRESP_SHIFT                                                  \
            (HST_CMNDIAGS_EN_ROUND_SHIFT + HST_CMNDIAGS_EN_ROUND_SIZE)    
#define HST_CMNDIAGS_EN_CWDIAGS_SHIFT                                                  \
            (HST_CMNDIAGS_EN_INVRESP_SHIFT + HST_CMNDIAGS_EN_INVRESP_SIZE)    
#define HST_CMNDIAGS_EN_ACCESSDIAGS_SHIFT                                              \
            (HST_CMNDIAGS_EN_CWDIAGS_SHIFT + HST_CMNDIAGS_EN_CWDIAGS_SIZE)    
#define HST_CMNDIAGS_EN_RANDOMCWDIAGS_SHIFT                                            \
            (HST_CMNDIAGS_EN_ACCESSDIAGS_SHIFT + HST_CMNDIAGS_EN_ACCESSDIAGS_SIZE)    
#define HST_CMNDIAGS_EN_CSMDIAGS_SHIFT                                                 \
            (HST_CMNDIAGS_EN_RANDOMCWDIAGS_SHIFT + HST_CMNDIAGS_EN_RANDOMCWDIAGS_SIZE)    
#define HST_CMNDIAGS_EN_COMMANDACTIVE_SHIFT                                            \
            (HST_CMNDIAGS_EN_CSMDIAGS_SHIFT + HST_CMNDIAGS_EN_CSMDIAGS_SIZE)                
#define HST_CMNDIAGS_RFU2_SHIFT                                                        \
            (HST_CMNDIAGS_EN_COMMANDACTIVE_SHIFT + HST_CMNDIAGS_EN_COMMANDACTIVE_SIZE)
    
/* HST_CMNDIAGS bit-field retrieval and testing macros                      */
/* rv - the value from the HST_CMNDIAGS register                            */
#define HST_CMNDIAGS_DIAGS_ARE_ENABLED(rv)                                        \
    REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_DIAGS_SHIFT)
#define HST_CMNDIAGS_DIAGS_ARE_DISABLED(rv)                                       \
    !(REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_DIAGS_SHIFT))
#define HST_CMNDIAGS_STATUS_ARE_ENABLED(rv)                                       \
    REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_STATUS_SHIFT)
#define HST_CMNDIAGS_STATUS_ARE_DISABLED(rv)                                      \
    !(REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_STATUS_SHIFT))
#define HST_CMNDIAGS_CYCLE_ARE_ENABLED(rv)                                        \
    REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_CYCLE_SHIFT)
#define HST_CMNDIAGS_CYCLE_ARE_DISABLED(rv)                                       \
    !(REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_CYCLE_SHIFT))
#define HST_CMNDIAGS_ROUND_ARE_ENABLED(rv)                                        \
    REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_ROUND_SHIFT)
#define HST_CMNDIAGS_ROUND_ARE_DISABLED(rv)                                       \
    !(REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_ROUND_SHIFT))
#define HST_CMNDIAGS_INVRESP_ARE_ENABLED(rv)                                      \
            REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_INVRESP_SHIFT) 
#define HST_CMNDIAGS_INVRESP_ARE_DISABLED(rv)                                     \
           !(REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_INVRESP_SHIFT))
#define HST_CMNDIAGS_CWDIAGS_ARE_ENABLED(rv)                                      \
            REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_CWDIAGS_SHIFT)
#define HST_CMNDIAGS_CWDIAGS_ARE_DISABLED(rv)                                     \
           !(REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_CWDIAGS_SHIFT))
#define HST_CMNDIAGS_ACCESSDIAGS_ARE_ENABLED(rv)                                  \
            REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_ACCESSDIAGS_SHIFT)
#define HST_CMNDIAGS_ACCESSDIAGS_ARE_DISABLED(rv)                                 \
           !(REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_ACCESSDIAGS_SHIFT))
#define HST_CMNDIAGS_RANDOMCWDIAGS_ARE_ENABLED(rv)                                \
            REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_RANDOMCWDIAGS_SHIFT)
#define HST_CMNDIAGS_RANDOMCWDIAGS_ARE_DISABLED(rv)                               \
           !(REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_RANDOMCWDIAGS_SHIFT))
#define HST_CMNDIAGS_CSMDIAGS_ARE_ENABLED(rv)                                     \
            REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_CSMDIAGS_SHIFT)
#define HST_CMNDIAGS_CSMDIAGS_ARE_DISABLED(rv)                                    \
           !(REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_CSMDIAGS_SHIFT))
#define HST_CMNDIAGS_COMMANDACTIVE_ARE_ENABLED(rv)                                \
            REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_COMMANDACTIVE_SHIFT)
#define HST_CMNDIAGS_COMMANDACTIVE_ARE_DISABLED(rv)                               \
           !(REGISTER_TEST_BIT(rv, HST_CMNDIAGS_EN_COMMANDACTIVE_SHIFT))          
#define HST_CMNDIAGS_GET_RFU2(rv)                                                 \
            BIT_FIELD_EXTRACT(rv, HST_CMNDIAGS_RFU2_SIZE, HST_CMNDIAGS_RFU2_SHIFT)
            
/* HST_CMNDIAGS bit-field value creation macros                             */
/* va - the value to put into the bit field                                 */
#define HST_CMNDIAGS_DIAGS_ENABLED                                                \
    CREATE_1BIT_MASK(HST_CMNDIAGS_EN_DIAGS_SHIFT)
#define HST_CMNDIAGS_DIAGS_DISABLED     0
#define HST_CMNDIAGS_STATUS_ENABLED                                               \
    CREATE_1BIT_MASK(HST_CMNDIAGS_EN_STATUS_SHIFT)
#define HST_CMNDIAGS_STATUS_DISABLED    0
#define HST_CMNDIAGS_CYCLE_ENABLED                                                \
    CREATE_1BIT_MASK(HST_CMNDIAGS_EN_CYCLE_SHIFT)
#define HST_CMNDIAGS_CYCLE_DISABLED    0
#define HST_CMNDIAGS_ROUND_ENABLED                                                \
    CREATE_1BIT_MASK(HST_CMNDIAGS_EN_ROUND_SHIFT)
#define HST_CMNDIAGS_ROUND_DISABLED    0
#define HST_CMNDIAGS_INVRESP_ENABLED                                              \
           CREATE_1BIT_MASK(HST_CMNDIAGS_EN_INVRESP_SHIFT)
#define HST_CMNDIAGS_INVRESP_DISABLED    0
#define HST_CMNDIAGS_CWDIAGS_ENABLED                                              \
           CREATE_1BIT_MASK(HST_CMNDIAGS_EN_CWDIAGS_SHIFT)
#define HST_CMNDIAGS_CWDIAGS_DISABLED    0
#define HST_CMNDIAGS_ACCESSDIAGS_ENABLED                                          \
           CREATE_1BIT_MASK(HST_CMNDIAGS_EN_ACCESSDIAGS_SHIFT)
#define HST_CMNDIAGS_ACCESSDIAGS_DISABLED    0
#define HST_CMNDIAGS_RANDOMCWDIAGS_ENABLED                                        \
           CREATE_1BIT_MASK(HST_CMNDIAGS_EN_RANDOMCWDIAGS_SHIFT)
#define HST_CMNDIAGS_RANDOMCWDIAGS_DISABLED    0
#define HST_CMNDIAGS_CSMDIAGS_ENABLED                                             \
           CREATE_1BIT_MASK(HST_CMNDIAGS_EN_CSMDIAGS_SHIFT)
#define HST_CMNDIAGS_CSMDIAGS_DISABLED    0
#define HST_CMNDIAGS_COMMANDACTIVE_ENABLED                                        \
           CREATE_1BIT_MASK(HST_CMNDIAGS_EN_COMMANDACTIVE_SHIFT)
#define HST_CMNDIAGS_COMMANDACTIVE_DISABLED    0
#define HST_CMNDIAGS_RFU2(va)                                                     \
           BIT_FIELD_CREATE(va, HST_CMNDIAGS_RFU2_SIZE, HST_CMNDIAGS_RFU2_SHIFT)    
    
/* HST_CMNDIAGS register value modification macros (i.e., will modify the   */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_CMNDIAGS_SET_DIAGS_ENABLED(rv)                                       \
    REGISTER_SET_BIT(rv, HST_CMNDIAGS_EN_DIAGS_SHIFT)
#define HST_CMNDIAGS_SET_DIAGS_DISABLED(rv)                                      \
    REGISTER_CLR_BIT(rv, HST_CMNDIAGS_EN_DIAGS_SHIFT)
#define HST_CMNDIAGS_SET_STATUS_ENABLED(rv)                                      \
    REGISTER_SET_BIT(rv, HST_CMNDIAGS_EN_STATUS_SHIFT)
#define HST_CMNDIAGS_SET_STATUS_DISABLED(rv)                                     \
    REGISTER_CLR_BIT(rv, HST_CMNDIAGS_EN_STATUS_SHIFT)
#define HST_CMNDIAGS_SET_CYCLE_ENABLED(rv)                                       \
    REGISTER_SET_BIT(rv, HST_CMNDIAGS_EN_CYCLE_SHIFT)
#define HST_CMNDIAGS_SET_CYCLE_DISABLED(rv)                                      \
    REGISTER_CLR_BIT(rv, HST_CMNDIAGS_EN_CYCLE_SHIFT)
#define HST_CMNDIAGS_SET_ROUND_ENABLED(rv)                                       \
    REGISTER_SET_BIT(rv, HST_CMNDIAGS_EN_ROUND_SHIFT)
#define HST_CMNDIAGS_SET_ROUND_DISABLED(rv)                                      \
    REGISTER_CLR_BIT(rv, HST_CMNDIAGS_EN_ROUND_SHIFT)
#define HST_CMNDIAGS_SET_ROUND_ENABLED(rv)                                       \
    REGISTER_SET_BIT(rv, HST_CMNDIAGS_EN_ROUND_SHIFT)
#define HST_CMNDIAGS_SET_ROUND_DISABLED(rv)                                      \
    REGISTER_CLR_BIT(rv, HST_CMNDIAGS_EN_ROUND_SHIFT)
#define HST_CMNDIAGS_SET_INVRESP_ENABLED(rv)                                     \
    REGISTER_SET_BIT(rv, HST_CMNDIAGS_EN_INVRESP_SHIFT)
#define HST_CMNDIAGS_SET_INVRESP_DISABLED(rv)                                    \
    REGISTER_CLR_BIT(rv, HST_CMNDIAGS_EN_INVRESP_SHIFT)
#define HST_CMNDIAGS_SET_CWDIAGS_ENABLED(rv)                                     \
    REGISTER_SET_BIT(rv, HST_CMNDIAGS_EN_CWDIAGS_SHIFT)
#define HST_CMNDIAGS_SET_CWDIAGS_DISABLED(rv)                                    \
    REGISTER_CLR_BIT(rv, HST_CMNDIAGS_EN_CWDIAGS_SHIFT)
#define HST_CMNDIAGS_SET_ACCESSDIAGS_ENABLED(rv)                                 \
    REGISTER_SET_BIT(rv, HST_CMNDIAGS_EN_ACCESSDIAGS_SHIFT)
#define HST_CMNDIAGS_SET_ACCESSDIAGS_DISABLED(rv)                                \
    REGISTER_CLR_BIT(rv, HST_CMNDIAGS_EN_ACCESSDIAGS_SHIFT)
#define HST_CMNDIAGS_SET_RANDOMCWDIAGS_ENABLED(rv)                               \
    REGISTER_SET_BIT(rv, HST_CMNDIAGS_EN_RANDOMCWDIAGS_SHIFT)
#define HST_CMNDIAGS_SET_RANDOMCWDIAGS_DISABLED(rv)                              \
    REGISTER_CLR_BIT(rv, HST_CMNDIAGS_EN_RANDOMCWDIAGS_SHIFT)
#define HST_CMNDIAGS_SET_CSMDIAGS_ENABLED(rv)                                    \
    REGISTER_SET_BIT(rv, HST_CMNDIAGS_EN_CSMDIAGS_SHIFT)
#define HST_CMNDIAGS_SET_CSMDIAGS_DISABLED(rv)                                   \
    REGISTER_CLR_BIT(rv, HST_CMNDIAGS_EN_CSMDIAGS_SHIFT)
#define HST_CMNDIAGS_SET_COMMANDACTIVE_ENABLED(rv)                               \
    REGISTER_SET_BIT(rv, HST_CMNDIAGS_EN_COMMANDACTIVE_SHIFT)
#define HST_CMNDIAGS_SET_COMMANDACTIVE_DISABLED(rv)                              \
    REGISTER_CLR_BIT(rv, HST_CMNDIAGS_EN_COMMANDACTIVE_SHIFT)    
#define HST_CMNDIAGS_SET_RFU2(rv, va)                                            \
    REGISTER_SET_VALUE(rv, va, HST_CMNDIAGS_RFU2_SIZE, HST_CMNDIAGS_RFU2_SHIFT)


/* HST_PROTSCH_TXTIME_ON register helper macros                             */
/* The size of the bit fields in the HST_PROTSCH_TXTIME_ON register.        */
#define HST_PROTSCH_TXTIME_ON_TIME_SIZE     16
#define HST_PROTSCH_TXTIME_ON_RFU1_SIZE     16
/* The number of bits that fields are left-shifted in the                   */
/* HST_PROTSCH_TXTIME_ON register.                                          */
#define HST_PROTSCH_TXTIME_ON_TIME_SHIFT    0
#define HST_PROTSCH_TXTIME_ON_RFU1_SHIFT                                    \
    (HST_PROTSCH_TXTIME_ON_TIME_SHIFT + HST_PROTSCH_TXTIME_ON_TIME_SIZE)
/* HST_PROTSCH_TXTIME_ON bit-field retrieval and testing macros             */
/* rv - the value from the HST_PROTSCH_TXTIME_ON register                   */
#define HST_PROTSCH_TXTIME_ON_GET_TIME(rv)                                  \
    BIT_FIELD_EXTRACT(                                                      \
        rv,                                                                 \
        HST_PROTSCH_TXTIME_ON_TIME_SIZE,                                    \
        HST_PROTSCH_TXTIME_ON_TIME_SHIFT)
#define HST_PROTSCH_TXTIME_ON_GET_RFU1(rv)                                  \
    BIT_FIELD_EXTRACT(                                                      \
        rv,                                                                 \
        HST_PROTSCH_TXTIME_ON_RFU1_SIZE,                                    \
        HST_PROTSCH_TXTIME_ON_RFU1_SHIFT)
/* HST_PROTSCH_TXTIME_ON bit-field value creation macros                    */
/* va - the value to put into the bit field                                 */
#define HST_PROTSCH_TXTIME_ON_TIME(va)                                      \
    BIT_FIELD_CREATE(                                                       \
        va,                                                                 \
        HST_PROTSCH_TXTIME_ON_TIME_SIZE,                                    \
        HST_PROTSCH_TXTIME_ON_TIME_SHIFT)
#define HST_PROTSCH_TXTIME_ON_RFU1(va)                                      \
    BIT_FIELD_CREATE(                                                       \
        va,                                                                 \
        HST_PROTSCH_TXTIME_ON_RFU1_SIZE,                                    \
        HST_PROTSCH_TXTIME_ON_RFU1_SHIFT)
/* HST_PROTSCH_TXTIME_ON register value modification macros (i.e., will     */
/* modify the field specified within an already existing register value).   */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_PROTSCH_TXTIME_ON_SET_TIME(rv, va)                              \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_PROTSCH_TXTIME_ON_TIME_SIZE,                     \
                       HST_PROTSCH_TXTIME_ON_TIME_SHIFT)
#define HST_PROTSCH_TXTIME_ON_SET_RFU1(rv, va)                              \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_PROTSCH_TXTIME_ON_RFU1_SIZE,                     \
                       HST_PROTSCH_TXTIME_ON_RFU1_SHIFT)

/* HST_PROTSCH_TXTIME_OFF register helper macros                            */
/* The size of the bit fields in the HST_PROTSCH_TXTIME_OFF register.       */
#define HST_PROTSCH_TXTIME_OFF_TIME_SIZE     16
#define HST_PROTSCH_TXTIME_OFF_RFU1_SIZE     16
/* The number of bits that fields are left-shifted in the                   */
/* HST_PROTSCH_TXTIME_OFF register.                                         */
#define HST_PROTSCH_TXTIME_OFF_TIME_SHIFT    0
#define HST_PROTSCH_TXTIME_OFF_RFU1_SHIFT                                   \
    (HST_PROTSCH_TXTIME_OFF_TIME_SHIFT + HST_PROTSCH_TXTIME_OFF_TIME_SIZE)
/* HST_PROTSCH_TXTIME_OFF bit-field retrieval and testing macros            */
/* rv - the value from the HST_PROTSCH_TXTIME_OFF register                  */
#define HST_PROTSCH_TXTIME_OFF_GET_TIME(rv)                                 \
    BIT_FIELD_EXTRACT(                                                      \
        rv,                                                                 \
        HST_PROTSCH_TXTIME_OFF_TIME_SIZE,                                   \
        HST_PROTSCH_TXTIME_OFF_TIME_SHIFT)
#define HST_PROTSCH_TXTIME_OFF_GET_RFU1(rv)                                 \
    BIT_FIELD_EXTRACT(                                                      \
        rv,                                                                 \
        HST_PROTSCH_TXTIME_OFF_RFU1_SIZE,                                   \
        HST_PROTSCH_TXTIME_OFF_RFU1_SHIFT)
/* HST_PROTSCH_TXTIME_OFF bit-field value creation macros                   */
/* va - the value to put into the bit field                                 */
#define HST_PROTSCH_TXTIME_OFF_TIME(va)                                     \
    BIT_FIELD_CREATE(                                                       \
        va,                                                                 \
        HST_PROTSCH_TXTIME_OFF_TIME_SIZE,                                   \
        HST_PROTSCH_TXTIME_OFF_TIME_SHIFT)
#define HST_PROTSCH_TXTIME_OFF_RFU1(va)                                     \
    BIT_FIELD_CREATE(                                                       \
        va,                                                                 \
        HST_PROTSCH_TXTIME_OFF_RFU1_SIZE,                                   \
        HST_PROTSCH_TXTIME_OFF_RFU1_SHIFT)
/* HST_PROTSCH_TXTIME_OFF register value modification macros (i.e., will    */
/* modify the field specified within an already existing register value).   */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_PROTSCH_TXTIME_OFF_SET_TIME(rv, va)                             \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_PROTSCH_TXTIME_OFF_TIME_SIZE,                    \
                       HST_PROTSCH_TXTIME_OFF_TIME_SHIFT)
#define HST_PROTSCH_TXTIME_OFF_SET_RFU1(rv, va)                             \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_PROTSCH_TXTIME_OFF_RFU1_SIZE,                    \
                       HST_PROTSCH_TXTIME_OFF_RFU1_SHIFT)

/* HST_MBP_ADDR register helper macros                                      */
/* The size of the bit fields in the HST_MBP_ADDR register.                 */
#define HST_MBP_ADDR_REGADDR_SIZE   12
#define HST_MBP_ADDR_RFU1_SIZE      20
/* The number of bits that fields are left-shifted in the HST_MBP_ADDR      */
/* register.                                                                */
#define HST_MBP_ADDR_REGADDR_SHIFT  0
#define HST_MBP_ADDR_RFU1_SHIFT                                             \
    (HST_MBP_ADDR_REGADDR_SHIFT + HST_MBP_ADDR_REGADDR_SIZE)
/* HST_MBP_ADDR bit-field retrieval and testing macros                      */
/* rv - the value from the HST_MBP_ADDR register                            */
#define HST_MBP_ADDR_GET_REGADDR(rv)                                        \
    BIT_FIELD_EXTRACT(rv, HST_MBP_ADDR_REGADDR_SIZE, HST_MBP_ADDR_REGADDR_SHIFT)
#define HST_MBP_ADDR_GET_RFU1(rv)                                            \
    BIT_FIELD_EXTRACT(rv, HST_MBP_ADDR_RFU1_SIZE, HST_MBP_ADDR_RFU1_SHIFT)
/* HST_MBP_ADDR bit-field value creation macros                             */
/* va - the value to put into the bit field                                 */
#define HST_MBP_ADDR_REGADDR(va)                                            \
    BIT_FIELD_CREATE(va, HST_MBP_ADDR_REGADDR_SIZE, HST_MBP_ADDR_REGADDR_SHIFT)
#define HST_MBP_ADDR_RFU1(va)                                            \
    BIT_FIELD_CREATE(va, HST_MBP_ADDR_RFU1_SIZE, HST_MBP_ADDR_RFU1_SHIFT)
/* HST_MBP_ADDR register value modification macros (i.e., will modify the   */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_MBP_ADDR_SET_REGADDR(rv, va)                                    \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_MBP_ADDR_REGADDR_SIZE,                           \
                       HST_MBP_ADDR_REGADDR_SHIFT)
#define HST_MBP_ADDR_SET_RFU1(rv, va)                                       \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_MBP_ADDR_RFU1_SIZE,                              \
                       HST_MBP_ADDR_RFU1_SHIFT)

/* HST_MBP_DATA register helper macros                                      */
/* The size of the bit fields in the HST_MBP_DATA register.                 */
#define HST_MBP_DATA_REGDATA_SIZE   16
#define HST_MBP_DATA_RFU1_SIZE      16
/* The number of bits that fields are left-shifted in the HST_MBP_DATA      */
/* register.                                                                */
#define HST_MBP_DATA_REGDATA_SHIFT  0
#define HST_MBP_DATA_RFU1_SHIFT                                             \
    (HST_MBP_DATA_REGDATA_SHIFT + HST_MBP_DATA_REGDATA_SIZE)
/* HST_MBP_DATA bit-field retrieval and testing macros                      */
/* rv - the value from the HST_MBP_DATA register                            */
#define HST_MBP_DATA_GET_REGDATA(rv)                                        \
    BIT_FIELD_EXTRACT(rv, HST_MBP_DATA_REGDATA_SIZE, HST_MBP_DATA_REGDATA_SHIFT)
#define HST_MBP_DATA_GET_RFU1(rv)                                           \
    BIT_FIELD_EXTRACT(rv, HST_MBP_DATA_RFU1_SIZE, HST_MBP_DATA_RFU1_SHIFT)
/* HST_MBP_DATA bit-field value creation macros                             */
/* va - the value to put into the bit field                                 */
#define HST_MBP_DATA_REGDATA(va)                                            \
    BIT_FIELD_CREATE(va, HST_MBP_DATA_REGDATA_SIZE, HST_MBP_DATA_REGDATA_SHIFT)
#define HST_MBP_DATA_RFU1(va)                                               \
    BIT_FIELD_CREATE(va, HST_MBP_DATA_RFU1_SIZE, HST_MBP_DATA_RFU1_SHIFT)
/* HST_MBP_DATA register value modification macros (i.e., will modify the   */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_MBP_DATA_SET_REGDATA(rv, va)                                    \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_MBP_DATA_REGDATA_SIZE,                           \
                       HST_MBP_DATA_REGDATA_SHIFT)
#define HST_MBP_DATA_SET_RFU1(rv, va)                                       \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_MBP_DATA_RFU1_SIZE,                              \
                       HST_MBP_DATA_RFU1_SHIFT)

/* HST_LPROF_SEL register helper macros                                     */
/* The size of the bit fields in the HST_LPROF_SEL register.                */
#define HST_LPROF_SEL_SELECTOR_SIZE     8
#define HST_LPROF_SEL_RFU1_SIZE         24
/* The number of bits that fields are left-shifted in the HST_LPROF_SEL     */
/* register.                                                                */
#define HST_LPROF_SEL_SELECTOR_SHIFT    0
#define HST_LPROF_SEL_RFU1_SHIFT                                            \
    (HST_LPROF_SEL_SELECTOR_SHIFT + HST_LPROF_SEL_SELECTOR_SIZE)
/* HST_LPROF_SEL bit-field retrieval and testing macros                    */
/* rv - the value from the HST_LPROF_SEL register                          */
#define HST_LPROF_SEL_GET_SELECTOR(rv)                                      \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_LPROF_SEL_SELECTOR_SIZE,                          \
                      HST_LPROF_SEL_SELECTOR_SHIFT)
#define HST_LPROF_SEL_GET_RFU1(rv)                                          \
    BIT_FIELD_EXTRACT(rv, HST_LPROF_SEL_RFU1_SIZE, HST_LPROF_SEL_RFU1_SHIFT)
/* HST_LPROF_SEL bit-field value creation macros                            */
/* va - the value to put into the bit field                                 */
#define HST_LPROF_SEL_SELECTOR(va)                                          \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_LPROF_SEL_SELECTOR_SIZE,                           \
                     HST_LPROF_SEL_SELECTOR_SHIFT)
#define HST_LPROF_SEL_RFU1(va)                                              \
    BIT_FIELD_CREATE(va, HST_LPROF_SEL_RFU1_SIZE, HST_LPROF_SEL_RFU1_SHIFT)
/* HST_LPROF_SEL register value modification macros (i.e., will modify the  */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_LPROF_SEL_SET_SELECTOR(rv, va)                                  \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_LPROF_SEL_SELECTOR_SIZE,                         \
                       HST_LPROF_SEL_SELECTOR_SHIFT)
#define HST_LPROF_SEL_SET_RFU1(rv, va)                                      \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_LPROF_SEL_RFU1_SIZE,                             \
                       HST_LPROF_SEL_RFU1_SHIFT)

/* HST_LPROF_ADDR register helper macros                                    */
/* The size of the bit fields in the HST_LPROF_ADDR register.               */
#define HST_LPROF_ADDR_REGADDR_SIZE   12
#define HST_LPROF_ADDR_RFU1_SIZE      20
/* The number of bits that fields are left-shifted in the HST_LPROF_ADDR    */
/* register.                                                                */
#define HST_LPROF_ADDR_REGADDR_SHIFT  0
#define HST_LPROF_ADDR_RFU1_SHIFT                                           \
    (HST_LPROF_ADDR_REGADDR_SHIFT + HST_LPROF_ADDR_REGADDR_SIZE)
/* HST_LPROF_ADDR bit-field retrieval and testing macros                    */
/* rv - the value from the HST_LPROF_ADDR register                          */
#define HST_LPROF_ADDR_GET_REGADDR(rv)                                      \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_LPROF_ADDR_REGADDR_SIZE,                          \
                      HST_LPROF_ADDR_REGADDR_SHIFT)
#define HST_LPROF_ADDR_GET_RFU1(rv)                                         \
    BIT_FIELD_EXTRACT(rv, HST_LPROF_ADDR_RFU1_SIZE, HST_LPROF_ADDR_RFU1_SHIFT)
/* HST_LPROF_ADDR bit-field value creation macros                           */
/* va - the value to put into the bit field                                 */
#define HST_LPROF_ADDR_REGADDR(va)                                          \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_LPROF_ADDR_REGADDR_SIZE,                           \
                     HST_LPROF_ADDR_REGADDR_SHIFT)
#define HST_LPROF_ADDR_RFU1(va)                                             \
    BIT_FIELD_CREATE(va, HST_LPROF_ADDR_RFU1_SIZE, HST_LPROF_ADDR_RFU1_SHIFT)
/* HST_LPROF_ADDR register value modification macros (i.e., will modify the */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_LPROF_ADDR_SET_REGADDR(rv, va)                                  \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_LPROF_ADDR_REGADDR_SIZE,                         \
                       HST_LPROF_ADDR_REGADDR_SHIFT)
#define HST_LPROF_ADDR_SET_RFU1(rv, va)                                     \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_LPROF_ADDR_RFU1_SIZE,                            \
                       HST_LPROF_ADDR_RFU1_SHIFT)

/* HST_LPROF_DATA register helper macros                                    */
/* The size of the bit fields in the HST_LPROF_DATA register.               */
#define HST_LPROF_DATA_REGDATA_SIZE   16
#define HST_LPROF_DATA_RFU1_SIZE      16
/* The number of bits that fields are left-shifted in the HST_LPROF_DATA    */
/* register.                                                                */
#define HST_LPROF_DATA_REGDATA_SHIFT  0
#define HST_LPROF_DATA_RFU1_SHIFT                                           \
    (HST_LPROF_DATA_REGDATA_SHIFT + HST_LPROF_DATA_REGDATA_SIZE)
/* HST_LPROF_DATA bit-field retrieval and testing macros                    */
/* rv - the value from the HST_LPROF_DATA register                          */
#define HST_LPROF_DATA_GET_REGDATA(rv)                                      \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_LPROF_DATA_REGDATA_SIZE,                          \
                      HST_LPROF_DATA_REGDATA_SHIFT)
#define HST_LPROF_DATA_GET_RFU1(rv)                                         \
    BIT_FIELD_EXTRACT(rv, HST_LPROF_DATA_RFU1_SIZE, HST_LPROF_DATA_RFU1_SHIFT)
/* HST_LPROF_DATA bit-field value creation macros                           */
/* va - the value to put into the bit field                                 */
#define HST_LPROF_DATA_REGDATA(va)                                          \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_LPROF_DATA_REGDATA_SIZE,                           \
                     HST_LPROF_DATA_REGDATA_SHIFT)
#define HST_LPROF_DATA_RFU1(va)                                             \
    BIT_FIELD_CREATE(va, HST_LPROF_DATA_RFU1_SIZE, HST_LPROF_DATA_RFU1_SHIFT)
/* HST_LPROF_DATA register value modification macros (i.e., will modify the */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_LPROF_DATA_SET_REGDATA(rv, va)                                  \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_LPROF_DATA_REGDATA_SIZE,                         \
                       HST_LPROF_DATA_REGDATA_SHIFT)
#define HST_LPROF_DATA_SET_RFU1(rv, va)                                     \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_LPROF_DATA_RFU1_SIZE,                            \
                       HST_LPROF_DATA_RFU1_SHIFT)

/* HST_OEM_ADDR register helper macros                                      */
/* The size of the bit fields in the HST_OEM_ADDR register.                 */
#define HST_OEM_ADDR_OEMADDR_SIZE   16
#define HST_OEM_ADDR_RFU1_SIZE      16
/* The number of bits that fields are left-shifted in the HST_OEM_ADDR      */
/* register.                                                                */
#define HST_OEM_ADDR_OEMADDR_SHIFT  0
#define HST_OEM_ADDR_RFU1_SHIFT                                             \
    (HST_OEM_ADDR_OEMADDR_SHIFT + HST_OEM_ADDR_OEMADDR_SIZE)
/* HST_OEM_ADDR bit-field retrieval and testing macros                      */
/* rv - the value from the HST_OEM_ADDR register                            */
#define HST_OEM_ADDR_GET_OEMADDR(rv)                                        \
    BIT_FIELD_EXTRACT(rv, HST_OEM_ADDR_OEMADDR_SIZE, HST_OEM_ADDR_OEMADDR_SHIFT)
#define HST_OEM_ADDR_GET_RFU1(rv)                                           \
    BIT_FIELD_EXTRACT(rv, HST_OEM_ADDR_RFU1_SIZE, HST_OEM_ADDR_RFU1_SHIFT)
/* HST_OEM_ADDR bit-field value creation macros                             */
/* va - the value to put into the bit field                                 */
#define HST_OEM_ADDR_OEMADDR(va)                                            \
    BIT_FIELD_CREATE(va, HST_OEM_ADDR_OEMADDR_SIZE, HST_OEM_ADDR_OEMADDR_SHIFT)
#define HST_OEM_ADDR_RFU1(va)                                            \
    BIT_FIELD_CREATE(va, HST_OEM_ADDR_RFU1_SIZE, HST_OEM_ADDR_RFU1_SHIFT)
/* HST_OEM_ADDR register value modification macros (i.e., will modify the   */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_OEM_ADDR_SET_OEMADDR(rv, va)                                    \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_OEM_ADDR_OEMADDR_SIZE,                           \
                       HST_OEM_ADDR_OEMADDR_SHIFT)
#define HST_OEM_ADDR_SET_RFU1(rv, va)                                       \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_OEM_ADDR_RFU1_SIZE,                              \
                       HST_OEM_ADDR_RFU1_SHIFT)

/* HST_GPIO_INMSK register helper macros                                    */
/* The size of the bit fields in the HST_GPIO_INMSK register.               */
#define HST_GPIO_INMSK_MASK_SIZE    4
#define HST_GPIO_INMSK_RFU1_SIZE    28
/* The number of bits that fields are left-shifted in the HST_GPIO_INMSK    */
/* register.                                                                */
#define HST_GPIO_INMSK_MASK_SHIFT   0
#define HST_GPIO_INMSK_RFU1_SHIFT                                           \
    (HST_GPIO_INMSK_MASK_SHIFT + HST_GPIO_INMSK_MASK_SIZE)
#define HST_GPIO_INMSK_CYCLES_INFINITE  0xFFFF
/* HST_GPIO_INMSK bit-field retrieval and testing macros                    */
/* rv - the value from the HST_GPIO_INMSK register                          */
#define HST_GPIO_INMSK_GET_MASK(rv)                                         \
    BIT_FIELD_EXTRACT(rv, HST_GPIO_INMSK_MASK_SIZE, HST_GPIO_INMSK_MASK_SHIFT)
#define HST_GPIO_INMSK_GET_RFU1(rv)                                         \
    BIT_FIELD_EXTRACT(rv, HST_GPIO_INMSK_RFU1_SIZE, HST_GPIO_INMSK_RFU1_SHIFT)
/* HST_GPIO_INMSK bit-field value creation macros                           */
/* va - the value to put into the bit field                                 */
#define HST_GPIO_INMSK_MASK(va)                                             \
    BIT_FIELD_CREATE(va, HST_GPIO_INMSK_MASK_SIZE, HST_GPIO_INMSK_MASK_SHIFT)
#define HST_GPIO_INMSK_RFU1(va)                                             \
    BIT_FIELD_CREATE(va, HST_GPIO_INMSK_RFU1_SIZE, HST_GPIO_INMSK_RFU1_SHIFT)
/* HST_GPIO_INMSK register value modification macros (i.e., will modify the */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_GPIO_INMSK_SET_MASK(rv, va)                                     \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_GPIO_INMSK_MASK_SIZE,                            \
                       HST_GPIO_INMSK_MASK_SHIFT)
#define HST_GPIO_INMSK_SET_RFU1(rv, va)                                     \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_GPIO_INMSK_RFU1_SIZE,                            \
                       HST_GPIO_INMSK_RFU1_SHIFT)

/* HST_GPIO_OUTMSK register helper macros                                   */
/* The size of the bit fields in the HST_GPIO_OUTMSK register.              */
#define HST_GPIO_OUTMSK_MASK_SIZE    4
#define HST_GPIO_OUTMSK_RFU1_SIZE    28
/* The number of bits that fields are left-shifted in the HST_GPIO_OUTMSK   */
/* register.                                                                */
#define HST_GPIO_OUTMSK_MASK_SHIFT   0
#define HST_GPIO_OUTMSK_RFU1_SHIFT                                          \
    (HST_GPIO_OUTMSK_MASK_SHIFT + HST_GPIO_OUTMSK_MASK_SIZE)
#define HST_GPIO_OUTMSK_CYCLES_INFINITE  0xFFFF
/* HST_GPIO_OUTMSK bit-field retrieval and testing macros                   */
/* rv - the value from the HST_GPIO_OUTMSK register                         */
#define HST_GPIO_OUTMSK_GET_MASK(rv)                                        \
    BIT_FIELD_EXTRACT(rv, HST_GPIO_OUTMSK_MASK_SIZE, HST_GPIO_OUTMSK_MASK_SHIFT)
#define HST_GPIO_OUTMSK_GET_RFU1(rv)                                        \
    BIT_FIELD_EXTRACT(rv, HST_GPIO_OUTMSK_RFU1_SIZE, HST_GPIO_OUTMSK_RFU1_SHIFT)
/* HST_GPIO_OUTMSK bit-field value creation macros                          */
/* va - the value to put into the bit field                                 */
#define HST_GPIO_OUTMSK_MASK(va)                                            \
    BIT_FIELD_CREATE(va, HST_GPIO_OUTMSK_MASK_SIZE, HST_GPIO_OUTMSK_MASK_SHIFT)
#define HST_GPIO_OUTMSK_RFU1(va)                                            \
    BIT_FIELD_CREATE(va, HST_GPIO_OUTMSK_RFU1_SIZE, HST_GPIO_OUTMSK_RFU1_SHIFT)
/* HST_GPIO_OUTMSK register value modification macros (i.e., will modify the*/
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_GPIO_OUTMSK_SET_MASK(rv, va)                                    \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_GPIO_OUTMSK_MASK_SIZE,                           \
                       HST_GPIO_OUTMSK_MASK_SHIFT)
#define HST_GPIO_OUTMSK_SET_RFU1(rv, va)                                    \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_GPIO_OUTMSK_RFU1_SIZE,                           \
                       HST_GPIO_OUTMSK_RFU1_SHIFT)

/* HST_GPIO_OUTVAL register helper macros                                   */
/* The size of the bit fields in the HST_GPIO_OUTVAL register.              */
#define HST_GPIO_OUTVAL_VALUE_SIZE   4
#define HST_GPIO_OUTVAL_RFU1_SIZE    28
/* The number of bits that fields are left-shifted in the HST_GPIO_OUTVAL   */
/* register.                                                                */
#define HST_GPIO_OUTVAL_VALUE_SHIFT  0
#define HST_GPIO_OUTVAL_RFU1_SHIFT                                          \
    (HST_GPIO_OUTVAL_VALUE_SHIFT + HST_GPIO_OUTVAL_VALUE_SIZE)
#define HST_GPIO_OUTVAL_CYCLES_INFINITE  0xFFFF
/* HST_GPIO_OUTVAL bit-field retrieval and testing macros                   */
/* rv - the value from the HST_GPIO_OUTVAL register                         */
#define HST_GPIO_OUTVAL_GET_VALUE(rv)                                       \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_GPIO_OUTVAL_VALUE_SIZE,                           \
                      HST_GPIO_OUTVAL_VALUE_SHIFT)
#define HST_GPIO_OUTVAL_GET_RFU1(rv)                                        \
    BIT_FIELD_EXTRACT(rv, HST_GPIO_OUTVAL_RFU1_SIZE, HST_GPIO_OUTVAL_RFU1_SHIFT)
/* HST_GPIO_OUTVAL bit-field value creation macros                          */
/* va - the value to put into the bit field                                 */
#define HST_GPIO_OUTVAL_VALUE(va)                                           \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_GPIO_OUTVAL_VALUE_SIZE,                            \
                     HST_GPIO_OUTVAL_VALUE_SHIFT)
#define HST_GPIO_OUTVAL_RFU1(va)                                            \
    BIT_FIELD_CREATE(va, HST_GPIO_OUTVAL_RFU1_SIZE, HST_GPIO_OUTVAL_RFU1_SHIFT)
/* HST_GPIO_OUTVAL register value modification macros (i.e., will modify the*/
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_GPIO_OUTVAL_SET_VALUE(rv, va)                                   \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_GPIO_OUTVAL_VALUE_SIZE,                          \
                       HST_GPIO_OUTVAL_VALUE_SHIFT)
#define HST_GPIO_OUTVAL_SET_RFU1(rv, va)                                    \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_GPIO_OUTVAL_RFU1_SIZE,                           \
                       HST_GPIO_OUTVAL_RFU1_SHIFT)

/* HST_ANT_CYCLES register helper macros                                    */
/* The size of the bit fields in the HST_ANT_CYCLES register.               */
#define HST_ANT_CYCLES_CYCLES_SIZE  16
#define HST_ANT_CYCLES_RFU1_SIZE    16
/* The number of bits that fields are left-shifted in the HST_ANT_CYCLES    */
/* register.                                                                */
#define HST_ANT_CYCLES_CYCLES_SHIFT  0
#define HST_ANT_CYCLES_RFU1_SHIFT                                           \
    (HST_ANT_CYCLES_CYCLES_SHIFT + HST_ANT_CYCLES_CYCLES_SIZE)
/* Constants for HST_ANT_CYCLES register bit fields (note that the values   */
/* are already shifted into the low-order bits of the constant.             */
#define HST_ANT_CYCLES_CYCLES_INFINITE  0xFFFF
/* HST_ANT_CYCLES bit-field retrieval and testing macros                    */
/* rv - the value from the HST_ANT_CYCLES register                          */
#define HST_ANT_CYCLES_GET_CYCLES(rv)                                       \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_ANT_CYCLES_CYCLES_SIZE,                           \
                      HST_ANT_CYCLES_CYCLES_SHIFT)
#define HST_ANT_CYCLES_GET_RFU1(rv)                                         \
    BIT_FIELD_EXTRACT(rv, HST_ANT_CYCLES_RFU1_SIZE, HST_ANT_CYCLES_RFU1_SHIFT)
/* HST_ANT_CYCLES bit-field value creation macros                           */
/* va - the value to put into the bit field                                 */
#define HST_ANT_CYCLES_CYCLES(va)                                           \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_ANT_CYCLES_CYCLES_SIZE,                            \
                     HST_ANT_CYCLES_CYCLES_SHIFT)
#define HST_ANT_CYCLES_RFU1(va)                                             \
    BIT_FIELD_CREATE(va, HST_ANT_CYCLES_RFU1_SIZE, HST_ANT_CYCLES_RFU1_SHIFT)
/* HST_ANT_CYCLES register value modification macros (i.e., will modify the */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_ANT_CYCLES_SET_CYCLES(rv, va)                                   \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_ANT_CYCLES_CYCLES_SIZE,                          \
                       HST_ANT_CYCLES_CYCLES_SHIFT)
#define HST_ANT_CYCLES_SET_RFU1(rv, va)                                     \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_ANT_CYCLES_RFU1_SIZE,                            \
                       HST_ANT_CYCLES_RFU1_SHIFT)

/* HST_ANT_DESC_CFG register helper macros                                  */
/* The size of the bit fields in the HST_ANT_DESC_CFG register.             */
#define HST_ANT_DESC_CFG_ENABLE_SIZE    1
#define HST_ANT_DESC_CFG_RFU1_SIZE      31
/* The number of bits that fields are left-shifted in the HST_ANT_DESC_CFG  */
/* register.                                                                */
#define HST_ANT_DESC_CFG_ENABLE_SHIFT   0
#define HST_ANT_DESC_CFG_RFU1_SHIFT                                         \
    (HST_ANT_DESC_CFG_ENABLE_SHIFT + HST_ANT_DESC_CFG_ENABLE_SIZE)
/* HST_ANT_DESC_CFG bit-field retrieval and testing macros                  */
/* rv - the value from the HST_ANT_DESC_CFG register                        */
#define HST_ANT_DESC_CFG_IS_ENABLED(rv)                                     \
    REGISTER_TEST_BIT(rv, HST_ANT_DESC_CFG_ENABLE_SHIFT)
#define HST_ANT_DESC_CFG_IS_DISABLED(rv)                                    \
    !(REGISTER_TEST_BIT(rv, HST_ANT_DESC_CFG_ENABLE_SHIFT))
#define HST_ANT_DESC_CFG_GET_RFU1(rv)                                       \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_ANT_DESC_CFG_RFU1_SIZE,                           \
                      HST_ANT_DESC_CFG_RFU1_SHIFT)
/* HST_ANT_DESC_CFG bit-field value creation macros                         */
/* va - the value to put into the bit field                                 */
#define HST_ANT_DESC_CFG_ENABLED                                            \
    CREATE_1BIT_MASK(HST_ANT_DESC_CFG_ENABLE_SHIFT)
#define HST_ANT_DESC_CFG_DISABLED   0
#define HST_ANT_DESC_CFG_RFU1(va)                                           \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_ANT_DESC_CFG_RFU1_SIZE,                            \
                     HST_ANT_DESC_CFG_RFU1_SHIFT)
/* HST_ANT_DESC_CFG register value modification macros (i.e., will modify   */
/* the field specified within an already existing register value).          */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_ANT_DESC_CFG_SET_ENABLED(rv)                                    \
    REGISTER_SET_BIT(rv, HST_ANT_DESC_CFG_ENABLE_SHIFT)
#define HST_ANT_DESC_CFG_SET_DISABLED(rv)                                   \
    REGISTER_CLR_BIT(rv, HST_ANT_DESC_CFG_ENABLE_SHIFT)
#define HST_ANT_DESC_CFG_SET_RFU1(rv, va)                                   \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_ANT_DESC_CFG_RFU1_SIZE,                          \
                       HST_ANT_DESC_CFG_RFU1_SHIFT)

/* MAC_ANT_DESC_STAT register helper macros                                 */
/* The size of the bit fields in the MAC_ANT_DESC_STAT register.            */
#define MAC_ANT_DESC_STAT_RESIST_SIZE   20
#define MAC_ANT_DESC_STAT_RFU1_SIZE     12
/* The number of bits that fields are left-shifted in the MAC_ANT_DESC_STAT */
/* register.                                                                */
#define MAC_ANT_DESC_STAT_RESIST_SHIFT  0
#define MAC_ANT_DESC_STAT_RFU1_SHIFT                                        \
    (MAC_ANT_DESC_STAT_RESIST_SHIFT + MAC_ANT_DESC_STAT_RESIST_SIZE)
/* MAC_ANT_DESC_STAT bit-field retrieval and testing macros                 */
/* rv - the value from the MAC_ANT_DESC_STAT register                       */
#define MAC_ANT_DESC_STAT_GET_RESIST(rv)                                    \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      MAC_ANT_DESC_STAT_RESIST_SIZE,                        \
                      MAC_ANT_DESC_STAT_RESIST_SHIFT)
#define MAC_ANT_DESC_STAT_GET_RFU1(rv)                                      \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      MAC_ANT_DESC_STAT_RFU1_SIZE,                          \
                      MAC_ANT_DESC_STAT_RFU1_SHIFT)
/* MAC_ANT_DESC_STAT bit-field value creation macros                        */
/* va - the value to put into the bit field                                 */
#define MAC_ANT_DESC_STAT_RESIST(va)                                        \
    BIT_FIELD_CREATE(va,                                                    \
                     MAC_ANT_DESC_STAT_RESIST_SIZE,                         \
                     MAC_ANT_DESC_STAT_RESIST_SHIFT)
#define MAC_ANT_DESC_STAT_RFU1(va)                                          \
    BIT_FIELD_CREATE(va,                                                    \
                     MAC_ANT_DESC_STAT_RFU1_SIZE,                           \
                     MAC_ANT_DESC_STAT_RFU1_SHIFT)
/* MAC_ANT_DESC_STAT register value modification macros (i.e., will modify  */
/* the field specified within an already existing register value).          */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define MAC_ANT_DESC_STAT_SET_RESIST(rv, va)                                \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       MAC_ANT_DESC_STAT_RESIST_SIZE,                       \
                       MAC_ANT_DESC_STAT_RESIST_SHIFT)
#define MAC_ANT_DESC_STAT_SET_RFU1(rv, va)                                  \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       MAC_ANT_DESC_STAT_RFU1_SIZE,                         \
                       MAC_ANT_DESC_STAT_RFU1_SHIFT)

/* HST_ANT_DESC_PORTDEF register helper macros                              */
#define MAX_TX_RX_PORTDEF 3
/* The size of the bit fields in the HST_ANT_DESC_PORTDEF register.         */
#define HST_ANT_DESC_PORTDEF_TXPORT_SIZE    2
#define HST_ANT_DESC_PORTDEF_RFU1_SIZE      14
#define HST_ANT_DESC_PORTDEF_RXPORT_SIZE    2
#define HST_ANT_DESC_PORTDEF_RFU2_SIZE      14
/* The number of bits that fields are left-shifted in the                   */
/* HST_ANT_DESC_PORTDEF register.                                           */
#define HST_ANT_DESC_PORTDEF_TXPORT_SHIFT   0
#define HST_ANT_DESC_PORTDEF_RFU1_SHIFT                                     \
    (HST_ANT_DESC_PORTDEF_TXPORT_SHIFT + HST_ANT_DESC_PORTDEF_TXPORT_SIZE)
#define HST_ANT_DESC_PORTDEF_RXPORT_SHIFT                                   \
    (HST_ANT_DESC_PORTDEF_RFU1_SHIFT + HST_ANT_DESC_PORTDEF_RFU1_SIZE)
#define HST_ANT_DESC_PORTDEF_RFU2_SHIFT                                     \
    (HST_ANT_DESC_PORTDEF_RXPORT_SHIFT + HST_ANT_DESC_PORTDEF_RXPORT_SIZE)
/* HST_ANT_DESC_PORTDEF bit-field retrieval and testing macros              */
/* rv - the value from the HST_ANT_DESC_PORTDEF register                    */
#define HST_ANT_DESC_PORTDEF_GET_TXPORT(rv)                                 \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_ANT_DESC_PORTDEF_TXPORT_SIZE,                     \
                      HST_ANT_DESC_PORTDEF_TXPORT_SHIFT)
#define HST_ANT_DESC_PORTDEF_GET_RFU1(rv)                                   \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_ANT_DESC_PORTDEF_RFU1_SIZE,                       \
                      HST_ANT_DESC_PORTDEF_RFU1_SHIFT)
#define HST_ANT_DESC_PORTDEF_GET_RXPORT(rv)                                 \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_ANT_DESC_PORTDEF_RXPORT_SIZE,                     \
                      HST_ANT_DESC_PORTDEF_RXPORT_SHIFT)
#define HST_ANT_DESC_PORTDEF_GET_RFU2(rv)                                   \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_ANT_DESC_PORTDEF_RFU2_SIZE,                       \
                      HST_ANT_DESC_PORTDEF_RFU2_SHIFT)
/* HST_ANT_DESC_PORTDEF bit-field value creation macros                     */
/* va - the value to put into the bit field                                 */
#define HST_ANT_DESC_PORTDEF_TXPORT(va)                                     \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_ANT_DESC_PORTDEF_TXPORT_SIZE,                      \
                     HST_ANT_DESC_PORTDEF_TXPORT_SHIFT)
#define HST_ANT_DESC_PORTDEF_RFU1(va)                                       \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_ANT_DESC_PORTDEF_RFU1_SIZE,                        \
                     HST_ANT_DESC_PORTDEF_RFU1_SHIFT)
#define HST_ANT_DESC_PORTDEF_RXPORT(va)                                     \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_ANT_DESC_PORTDEF_RXPORT_SIZE,                      \
                     HST_ANT_DESC_PORTDEF_RXPORT_SHIFT)
#define HST_ANT_DESC_PORTDEF_RFU2(va)                                       \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_ANT_DESC_PORTDEF_RFU2_SIZE,                        \
                     HST_ANT_DESC_PORTDEF_RFU2_SHIFT)
/* HST_ANT_DESC_PORTDEF register value modification macros (i.e., will      */
/* modify the field specified within an already existing register value).   */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_ANT_DESC_PORTDEF_SET_TXPORT(rv, va)                             \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_ANT_DESC_PORTDEF_TXPORT_SIZE,                    \
                       HST_ANT_DESC_PORTDEF_TXPORT_SHIFT)
#define HST_ANT_DESC_PORTDEF_SET_RFU1(rv, va)                               \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_ANT_DESC_PORTDEF_RFU1_SIZE,                      \
                       HST_ANT_DESC_PORTDEF_RFU1_SHIFT)
#define HST_ANT_DESC_PORTDEF_SET_RXPORT(rv, va)                             \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_ANT_DESC_PORTDEF_RXPORT_SIZE,                    \
                       HST_ANT_DESC_PORTDEF_RXPORT_SHIFT)
#define HST_ANT_DESC_PORTDEF_SET_RFU2(rv, va)                               \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_ANT_DESC_PORTDEF_RFU2_SIZE,                      \
                       HST_ANT_DESC_PORTDEF_RFU2_SHIFT)

/* HST_TAGMSK_DESC_CFG register helper macros                               */
/* The size of the bit fields in the HST_TAGMSK_DESC_CFG register.          */
#define HST_TAGMSK_DESC_CFG_ENABLE_SIZE     1
#define HST_TAGMSK_DESC_CFG_TARGET_SIZE     3
#define HST_TAGMSK_DESC_CFG_ACTION_SIZE     3
#define HST_TAGMSK_DESC_CFG_TRUNC_SIZE      1
#define HST_TAGMSK_DESC_CFG_RFU1_SIZE       24
/* The number of bits that fields are left-shifted in the                   */
/* HST_TAGMSK_DESC_CFG register.                                            */
#define HST_TAGMSK_DESC_CFG_ENABLE_SHIFT    0
#define HST_TAGMSK_DESC_CFG_TARGET_SHIFT                                    \
    (HST_TAGMSK_DESC_CFG_ENABLE_SHIFT + HST_TAGMSK_DESC_CFG_ENABLE_SIZE)
#define HST_TAGMSK_DESC_CFG_ACTION_SHIFT                                    \
    (HST_TAGMSK_DESC_CFG_TARGET_SHIFT + HST_TAGMSK_DESC_CFG_TARGET_SIZE)
#define HST_TAGMSK_DESC_CFG_TRUNC_SHIFT                                     \
    (HST_TAGMSK_DESC_CFG_ACTION_SHIFT + HST_TAGMSK_DESC_CFG_ACTION_SIZE)
#define HST_TAGMSK_DESC_CFG_RFU1_SHIFT                                      \
    (HST_TAGMSK_DESC_CFG_TRUNC_SHIFT + HST_TAGMSK_DESC_CFG_TRUNC_SIZE)
/* HST_TAGMSK_DESC_CFG bit-field retrieval and testing macros               */
/* rv - the value from the HST_TAGMSK_DESC_CFG register                     */
#define HST_TAGMSK_DESC_CFG_IS_ENABLED(rv)                                  \
    REGISTER_TEST_BIT(rv, HST_TAGMSK_DESC_CFG_ENABLE_SHIFT)
#define HST_TAGMSK_DESC_CFG_IS_DISABLED(rv)                                 \
    !(REGISTER_TEST_BIT(rv, HST_TAGMSK_DESC_CFG_ENABLE_SHIFT))
#define HST_TAGMSK_DESC_CFG_GET_TARGET(rv)                                  \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGMSK_DESC_CFG_TARGET_SIZE,                      \
                      HST_TAGMSK_DESC_CFG_TARGET_SHIFT)
#define HST_TAGMSK_DESC_CFG_GET_ACTION(rv)                                  \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGMSK_DESC_CFG_ACTION_SIZE,                      \
                      HST_TAGMSK_DESC_CFG_ACTION_SHIFT)
#define HST_TAGMSK_DESC_CFG_TRUNC_IS_ENABLED(rv)                            \
    REGISTER_TEST_BIT(rv, HST_TAGMSK_DESC_CFG_TRUNC_SHIFT)
#define HST_TAGMSK_DESC_CFG_TRUNC_IS_DISABLED(rv)                           \
    !(REGISTER_TEST_BIT(rv, HST_TAGMSK_DESC_CFG_TRUNC_SHIFT))
#define HST_TAGMSK_DESC_CFG_GET_RFU1(rv)                                    \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGMSK_DESC_CFG_RFU1_SIZE,                        \
                      HST_TAGMSK_DESC_CFG_RFU1_SHIFT)
/* HST_TAGMSK_DESC_CFG bit-field value creation macros                      */
/* va - the value to put into the bit field                                 */
#define HST_TAGMSK_DESC_CFG_ENABLED                                         \
    CREATE_1BIT_MASK(HST_TAGMSK_DESC_CFG_ENABLE_SHIFT)
#define HST_TAGMSK_DESC_CFG_DISABLED    0
#define HST_TAGMSK_DESC_CFG_TARGET(va)                                      \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGMSK_DESC_CFG_TARGET_SIZE,                       \
                     HST_TAGMSK_DESC_CFG_TARGET_SHIFT)
#define HST_TAGMSK_DESC_CFG_ACTION(va)                                      \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGMSK_DESC_CFG_ACTION_SIZE,                       \
                     HST_TAGMSK_DESC_CFG_ACTION_SHIFT)
#define HST_TAGMSK_DESC_CFG_TRUNC_ENABLED                                   \
    CREATE_1BIT_MASK(HST_TAGMSK_DESC_CFG_TRUNC_SHIFT)
#define HST_TAGMSK_DESC_CFG_TRUNC_DISABLED   0
#define HST_TAGMSK_DESC_CFG_RFU1(va)                                        \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGMSK_DESC_CFG_RFU1_SIZE,                         \
                     HST_TAGMSK_DESC_CFG_RFU1_SHIFT)
/* HST_TAGMSK_DESC_CFG register value modification macros (i.e., will modify*/
/* the field specified within an already existing register value).          */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_TAGMSK_DESC_CFG_SET_ENABLED(rv)                                 \
    REGISTER_SET_BIT(rv, HST_TAGMSK_DESC_CFG_ENABLE_SHIFT)
#define HST_TAGMSK_DESC_CFG_SET_DISABLED(rv)                                \
    REGISTER_CLR_BIT(rv, HST_TAGMSK_DESC_CFG_ENABLE_SHIFT)
#define HST_TAGMSK_DESC_CFG_SET_TARGET(rv, va)                              \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGMSK_DESC_CFG_TARGET_SIZE,                     \
                       HST_TAGMSK_DESC_CFG_TARGET_SHIFT)
#define HST_TAGMSK_DESC_CFG_SET_ACTION(rv, va)                              \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGMSK_DESC_CFG_ACTION_SIZE,                     \
                       HST_TAGMSK_DESC_CFG_ACTION_SHIFT)
#define HST_TAGMSK_DESC_CFG_SET_TRUNC_ENABLED(rv)                           \
    REGISTER_SET_BIT(rv, HST_TAGMSK_DESC_CFG_TRUNC_SHIFT)
#define HST_TAGMSK_DESC_CFG_SET_TRUNC_DISABLED(rv)                          \
    REGISTER_CLR_BIT(rv, HST_TAGMSK_DESC_CFG_TRUNC_SHIFT)
#define HST_TAGMSK_DESC_CFG_SET_RFU1(rv, va)                                \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGMSK_DESC_CFG_RFU1_SIZE,                       \
                       HST_TAGMSK_DESC_CFG_RFU1_SHIFT)

/* HST_TAGMSK_BANK register helper macros                                   */
/* The size of the bit fields in the HST_TAGMSK_BANK register.              */
#define HST_TAGMSK_BANK_BANK_SIZE   2
#define HST_TAGMSK_BANK_RFU1_SIZE   30
/* The number of bits that fields are left-shifted in the HST_TAGMSK_BANK   */
/* register.                                                                */
#define HST_TAGMSK_BANK_BANK_SHIFT  0
#define HST_TAGMSK_BANK_RFU1_SHIFT                                          \
    (HST_TAGMSK_BANK_BANK_SHIFT + HST_TAGMSK_BANK_BANK_SIZE)
/* Constants for HST_TAGMSK_BANK register bit fields (note that the values  */
/* are already shifted into the low-order bits of the constant.             */
#define HST_TAGMSK_BANK_BANK_RESERVED   0x0
#define HST_TAGMSK_BANK_BANK_EPC        0x1
#define HST_TAGMSK_BANK_BANK_TID        0x2
#define HST_TAGMSK_BANK_BANK_USER       0x3
/* HST_TAGMSK_BANK bit-field retrieval and testing macros                   */
/* rv - the value from the HST_TAGMSK_BANK register                         */
#define HST_TAGMSK_BANK_GET_BANK(rv)                                        \
    BIT_FIELD_EXTRACT(rv, HST_TAGMSK_BANK_BANK_SIZE, HST_TAGMSK_BANK_BANK_SHIFT)
#define HST_TAGMSK_BANK_GET_RFU1(rv)                                        \
    BIT_FIELD_EXTRACT(rv, HST_TAGMSK_BANK_RFU1_SIZE, HST_TAGMSK_BANK_RFU1_SHIFT)
/* HST_TAGMSK_BANK bit-field value creation macros                          */
/* va - the value to put into the bit field                                 */
#define HST_TAGMSK_BANK_BANK(va)                                            \
    BIT_FIELD_CREATE(va, HST_TAGMSK_BANK_BANK_SIZE, HST_TAGMSK_BANK_BANK_SHIFT)
#define HST_TAGMSK_BANK_RFU1(va)                                            \
    BIT_FIELD_CREATE(va, HST_TAGMSK_BANK_RFU1_SIZE, HST_TAGMSK_BANK_RFU1_SHIFT)
/* HST_TAGMSK_BANK register value modification macros (i.e., will modify the*/
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_TAGMSK_BANK_SET_BANK(rv, va)                                    \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGMSK_BANK_BANK_SIZE,                           \
                       HST_TAGMSK_BANK_BANK_SHIFT)
#define HST_TAGMSK_BANK_SET_RFU1(rv, va)                                    \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGMSK_BANK_RFU1_SIZE,                           \
                       HST_TAGMSK_BANK_RFU1_SHIFT)

/* HST_TAGMSK_LEN register helper macros                                    */
/* The size of the bit fields in the HST_TAGMSK_LEN register.               */
#define HST_TAGMSK_LEN_LEN_SIZE     8
#define HST_TAGMSK_LEN_RFU1_SIZE    24
/* The number of bits that fields are left-shifted in the HST_TAGMSK_LEN    */
/* register.                                                                */
#define HST_TAGMSK_LEN_LEN_SHIFT    0
#define HST_TAGMSK_LEN_RFU1_SHIFT                                           \
    (HST_TAGMSK_LEN_LEN_SHIFT + HST_TAGMSK_LEN_LEN_SIZE)
/* HST_TAGMSK_LEN bit-field retrieval and testing macros                    */
/* rv - the value from the HST_TAGMSK_LEN register                          */
#define HST_TAGMSK_LEN_GET_LEN(rv)                                          \
    BIT_FIELD_EXTRACT(rv, HST_TAGMSK_LEN_LEN_SIZE, HST_TAGMSK_LEN_LEN_SHIFT)
#define HST_TAGMSK_LEN_GET_RFU1(rv)                                         \
    BIT_FIELD_EXTRACT(rv, HST_TAGMSK_LEN_RFU1_SIZE, HST_TAGMSK_LEN_RFU1_SHIFT)
/* HST_TAGMSK_LEN bit-field value creation macros                           */
/* va - the value to put into the bit field                                 */
#define HST_TAGMSK_LEN_LEN(va)                                              \
    BIT_FIELD_CREATE(va, HST_TAGMSK_LEN_LEN_SIZE, HST_TAGMSK_LEN_LEN_SHIFT)
#define HST_TAGMSK_LEN_RFU1(va)                                             \
    BIT_FIELD_CREATE(va, HST_TAGMSK_LEN_RFU1_SIZE, HST_TAGMSK_LEN_RFU1_SHIFT)
/* HST_TAGMSK_LEN register value modification macros (i.e., will modify the */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_TAGMSK_LEN_SET_LEN(rv, va)                                      \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGMSK_LEN_LEN_SIZE,                             \
                       HST_TAGMSK_LEN_LEN_SHIFT)
#define HST_TAGMSK_LEN_SET_RFU1(rv, va)                                     \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGMSK_LEN_RFU1_SIZE,                            \
                       HST_TAGMSK_LEN_RFU1_SHIFT)

/* HST_TAGQTDAT register helper macros                               */
/* The size of the bit fields in the HST_TAGQTDAT register.          */
#define HST_TAGQTDAT_OPT_ACCESS_CMD_SIZE     4
#define HST_TAGQTDAT_QT_READ_WRITE_SIZE      1
#define HST_TAGQTDAT_QT_PERSISTENCE_SIZE        1
#define HST_TAGQTDAT_QT_MEM_MAP_SIZE         1
#define HST_TAGQTDAT_QT_SHORT_RANGE_SIZE     1
#define HST_TAGQTDAT_QT_RFU1_SIZE            7
#define HST_TAGQTDAT_QT_RAW_ENABLE_SIZE      1
#define HST_TAGQTDAT_QT_RAW_CTRL_SIZE       16
/* The number of bits that fields are left-shifted in the            */
/* HST_TAGQTDAT register.                                            */
#define HST_TAGQTDAT_OPT_ACCESS_CMD_SHIFT    0
#define HST_TAGQTDAT_QT_READ_WRITE_SHIFT                                    \
    (HST_TAGQTDAT_OPT_ACCESS_CMD_SHIFT + HST_TAGQTDAT_OPT_ACCESS_CMD_SIZE)
#define HST_TAGQTDAT_QT_PERSISTENCE_SHIFT                                      \
    (HST_TAGQTDAT_QT_READ_WRITE_SHIFT + HST_TAGQTDAT_QT_READ_WRITE_SIZE)
#define HST_TAGQTDAT_QT_MEM_MAP_SHIFT                                       \
    (HST_TAGQTDAT_QT_PERSISTENCE_SHIFT + HST_TAGQTDAT_QT_PERSISTENCE_SIZE)
#define HST_TAGQTDAT_QT_SHORT_RANGE_SHIFT                                   \
    (HST_TAGQTDAT_QT_MEM_MAP_SHIFT + HST_TAGQTDAT_QT_MEM_MAP_SIZE)    
#define HST_TAGQTDAT_QT_RFU1_SHIFT                                          \
    (HST_TAGQTDAT_QT_SHORT_RANGE_SHIFT + HST_TAGQTDAT_QT_SHORT_RANGE_SIZE)
#define HST_TAGQTDAT_QT_RAW_ENABLE_SHIFT                                    \
    (HST_TAGQTDAT_QT_RFU1_SHIFT + HST_TAGQTDAT_QT_RFU1_SIZE)
#define HST_TAGQTDAT_QT_RAW_CTRL_SHIFT                                      \
    (HST_TAGQTDAT_QT_RAW_ENABLE_SHIFT + HST_TAGQTDAT_QT_RAW_ENABLE_SIZE)
/* HST_TAGQTDAT bit-field retrieval and testing macros               */
/* rv - the value from the HST_TAGQTDAT register                     */
#define HST_TAGQTDAT_GET_OPT_ACCESS_CMD(rv)                                 \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGQTDAT_OPT_ACCESS_CMD_SIZE,                     \
                      HST_TAGQTDAT_OPT_ACCESS_CMD_SHIFT)
#define HST_TAGQTDAT_GET_QT_READ_WRITE(rv)                                  \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGQTDAT_QT_READ_WRITE_SIZE,                      \
                      HST_TAGQTDAT_QT_READ_WRITE_SHIFT)
#define HST_TAGQTDAT_GET_QT_PERSISTENCE(rv)                                 \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGQTDAT_QT_PERSISTENCE_SIZE,                     \
                      HST_TAGQTDAT_QT_PERSISTENCE_SHIFT)
#define HST_TAGQTDAT_GET_QT_MEM_MAP(rv)                                     \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGQTDAT_QT_MEM_MAP_SIZE,                         \
                      HST_TAGQTDAT_QT_MEM_MAP_SHIFT)
#define HST_TAGQTDAT_GET_QT_SHORT_RANGE(rv)                                 \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGQTDAT_QT_SHORT_RANGE_SIZE,                     \
                      HST_TAGQTDAT_QT_SHORT_RANGE_SHIFT)
#define HST_TAGQTDAT_IS_QT_RAW_ENABLED(rv)                                  \
    REGISTER_TEST_BIT(rv, HST_TAGQTDAT_QT_RAW_ENABLE_SHIFT)
#define HST_TAGQTDAT_GET_QT_RAW_CTRL(rv)                                    \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGQTDAT_QT_RAW_CTRL_SIZE,                        \
                      HST_TAGQTDAT_QT_RAW_CTRL_SHIFT)                     
/* HST_TAGQTDAT bit-field value creation macros                             */
/* va - the value to put into the bit field                                 */
#define HST_TAGQTDAT_OPT_ACCESS_CMD(va)                                     \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGQTDAT_OPT_ACCESS_CMD_SIZE,                      \
                     HST_TAGQTDAT_OPT_ACCESS_CMD_SHIFT)
#define HST_TAGQTDAT_QT_READ_WRITE(va)                                      \
    BIT_FIELD_CREATE(va,                                                    \
                      HST_TAGQTDAT_QT_READ_WRITE_SIZE,                      \
                      HST_TAGQTDAT_QT_READ_WRITE_SHIFT)
#define HST_TAGQTDAT_QT_PERSISTENCE(va)                                     \
    BIT_FIELD_CREATE(va,                                                    \
                      HST_TAGQTDAT_QT_PERSISTENCE_SIZE,                     \
                      HST_TAGQTDAT_QT_PERSISTENCE_SHIFT)
#define HST_TAGQTDAT_QT_MEM_MAP(va)                                         \
    BIT_FIELD_CREATE(va,                                                    \
                      HST_TAGQTDAT_QT_MEM_MAP_SIZE,                         \
                      HST_TAGQTDAT_QT_MEM_MAP_SHIFT)
#define HST_TAGQTDAT_QT_SHORT_RANGE(va)                                     \
    BIT_FIELD_CREATE(va,                                                    \
                      HST_TAGQTDAT_QT_SHORT_RANGE_SIZE,                     \
                      HST_TAGQTDAT_QT_SHORT_RANGE_SHIFT)
#define HST_TAGQTDAT_QT_RFU1(va)                                            \
    BIT_FIELD_CREATE(va,                                                    \
                      HST_TAGQTDAT_QT_RFU1_SSIZE,                           \
                      HST_TAGQTDAT_QT_RFU1_SHIFT)
#define HST_TAGQTDAT_QT_RAW_ENABLED(va)                                     \
    CREATE_1BIT_MASK(HST_TAGQTDAT_QT_RAW_ENABLE_SHIFT)   
#define HST_TAGQTDAT_QT_RAW_DISABLED   0
#define HST_TAGQTDAT_QT_RAW_CTRL(va)                                        \
    BIT_FIELD_CREATE(va,                                                    \
                      HST_TAGQTDAT_QT_RAW_CTRL_SIZE,                        \
                      HST_TAGQTDAT_QT_RAW_CTRL_SHIFT)
/* HST_TAGQTDAT register value modification macros (i.e., will modify*/
/* the field specified within an already existing register value).          */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_TAGQTDAT_SET_OPT_ACCESS_CMD(rv, va)                             \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGQTDAT_OPT_ACCESS_CMD_SIZE,                    \
                       HST_TAGQTDAT_OPT_ACCESS_CMD_SHIFT)
#define HST_TAGQTDAT_SET_QT_READ_WRITE(rv, va)                              \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGQTDAT_QT_READ_WRITE_SIZE,                     \
                       HST_TAGQTDAT_QT_READ_WRITE_SHIFT)
#define HST_TAGQTDAT_SET_QT_PERSISTENCE(rv, va)                             \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGQTDAT_QT_PERSISTENCE_SIZE,                    \
                       HST_TAGQTDAT_QT_PERSISTENCE_SHIFT)
#define HST_TAGQTDAT_SET_QT_MEM_MAP(rv, va)                                 \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGQTDAT_QT_MEM_MAP_SIZE,                        \
                       HST_TAGQTDAT_QT_MEM_MAP_SHIFT)
#define HST_TAGQTDAT_SET_QT_SHORT_RANGE(rv, va)                             \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGQTDAT_QT_SHORT_RANGE_SIZE,                    \
                       HST_TAGQTDAT_QT_SHORT_RANGE_SHIFT)
#define HST_TAGQTDAT_SET_QT_RFU1(rv, va)                                    \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGQTDAT_QT_RFU1_SIZE,                           \
                       HST_TAGQTDAT_QT_RFU1_SHIFT)
#define HST_TAGQTDAT_SET_QT_RAW_ENABLED(rv)                                 \
    REGISTER_SET_BIT(rv, HST_TAGQTDAT_QT_RAW_ENABLE_SHIFT)
#define HST_TAGQTDAT_SET_QT_RAW_DISABLED(rv)                                \
    REGISTER_CLR_BIT(rv, HST_TAGQTDAT_QT_RAW_ENABLE_SHIFT)
#define HST_TAGQTDAT_SET_QT_RAW_CTRL(rv,va)                                 \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGQTDAT_QT_RAW_CTRL_SIZE,                       \
                       HST_TAGQTDAT_QT_RAW_CTRL_SHIFT)
/* HST_TAGQTDAT OPT_ACCESS_CMD Values                                      */
#define HST_TAGQTDAT_OPT_ACCESS_CMD_NONE    0
#define HST_TAGQTDAT_OPT_ACCESS_CMD_READ    1
#define HST_TAGQTDAT_OPT_ACCESS_CMD_WRITE   2


/* HST_QUERY_CFG register helper macros                                     */
/* The size of the bit fields in the HST_QUERY_CFG register.                */
#define HST_QUERY_CFG_RFU1_SIZE     4
#define HST_QUERY_CFG_TARG_SIZE     1
#define HST_QUERY_CFG_SESS_SIZE     2
#define HST_QUERY_CFG_SEL_SIZE      2
#define HST_QUERY_CFG_RFU2_SIZE     23
/* The number of bits that fields are left-shifted in the HST_QUERY_CFG     */
/* register.                                                                */
#define HST_QUERY_CFG_RFU1_SHIFT    0
#define HST_QUERY_CFG_TARG_SHIFT                                            \
    (HST_QUERY_CFG_RFU1_SHIFT + HST_QUERY_CFG_RFU1_SIZE)
#define HST_QUERY_CFG_SESS_SHIFT                                            \
    (HST_QUERY_CFG_TARG_SHIFT + HST_QUERY_CFG_TARG_SIZE)
#define HST_QUERY_CFG_SEL_SHIFT                                             \
    (HST_QUERY_CFG_SESS_SHIFT + HST_QUERY_CFG_SESS_SIZE)
#define HST_QUERY_CFG_RFU2_SHIFT                                            \
    (HST_QUERY_CFG_SEL_SHIFT + HST_QUERY_CFG_SEL_SIZE)
/* Constants for HST_QUERY_CFG register bit fields (note that the values    */
/* are already shifted into the low-order bits of the constant.             */
#define HST_QUERY_CFG_TARG_A        0x0
#define HST_QUERY_CFG_TARG_B        0x1
#define HST_QUERY_CFG_SESS_S0       0x0
#define HST_QUERY_CFG_SESS_S1       0x1
#define HST_QUERY_CFG_SESS_S2       0x2
#define HST_QUERY_CFG_SESS_S3       0x3
#define HST_QUERY_CFG_SEL_ALL       0x0
#define HST_QUERY_CFG_SEL_NOTSL     0x2
#define HST_QUERY_CFG_SEL_SL        0x3
/* HST_QUERY_CFG bit-field retrieval and testing macros                     */
/* rv - the value from the HST_QUERY_CFG register                           */
#define HST_QUERY_CFG_GET_RFU1(rv)                                          \
    BIT_FIELD_EXTRACT(rv, HST_QUERY_CFG_RFU1_SIZE, HST_QUERY_CFG_RFU1_SHIFT)
#define HST_QUERY_CFG_GET_TARG(rv)                                          \
    BIT_FIELD_EXTRACT(rv, HST_QUERY_CFG_TARG_SIZE, HST_QUERY_CFG_TARG_SHIFT)
#define HST_QUERY_CFG_GET_SESS(rv)                                          \
    BIT_FIELD_EXTRACT(rv, HST_QUERY_CFG_SESS_SIZE, HST_QUERY_CFG_SESS_SHIFT)
#define HST_QUERY_CFG_GET_SEL(rv)                                           \
    BIT_FIELD_EXTRACT(rv, HST_QUERY_CFG_SEL_SIZE, HST_QUERY_CFG_SEL_SHIFT)
#define HST_QUERY_CFG_GET_RFU2(rv)                                          \
    BIT_FIELD_EXTRACT(rv, HST_QUERY_CFG_RFU2_SIZE, HST_QUERY_CFG_RFU2_SHIFT)
/* HST_QUERY_CFG bit-field value creation macros                            */
/* va - the value to put into the bit field                                 */
#define HST_QUERY_CFG_RFU1(rv)                                              \
    BIT_FIELD_CREATE(rv, HST_QUERY_CFG_RFU1_SIZE, HST_QUERY_CFG_RFU1_SHIFT)
#define HST_QUERY_CFG_TARG(rv)                                              \
    BIT_FIELD_CREATE(rv, HST_QUERY_CFG_TARG_SIZE, HST_QUERY_CFG_TARG_SHIFT)
#define HST_QUERY_CFG_SESS(rv)                                              \
    BIT_FIELD_CREATE(rv, HST_QUERY_CFG_SESS_SIZE, HST_QUERY_CFG_SESS_SHIFT)
#define HST_QUERY_CFG_SEL(rv)                                               \
    BIT_FIELD_CREATE(rv, HST_QUERY_CFG_SEL_SIZE, HST_QUERY_CFG_SEL_SHIFT)
#define HST_QUERY_CFG_RFU2(va)                                              \
    BIT_FIELD_CREATE(va, HST_QUERY_CFG_RFU2_SIZE, HST_QUERY_CFG_RFU2_SHIFT)
/* HST_QUERY_CFG register value modification macros (i.e., will modify the  */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_QUERY_CFG_SET_RFU1(rv, va)                                      \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_QUERY_CFG_RFU1_SIZE,                             \
                       HST_QUERY_CFG_RFU1_SHIFT)
#define HST_QUERY_CFG_SET_TARG(rv, va)                                      \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_QUERY_CFG_TARG_SIZE,                             \
                       HST_QUERY_CFG_TARG_SHIFT)
#define HST_QUERY_CFG_SET_SESS(rv, va)                                      \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_QUERY_CFG_SESS_SIZE,                             \
                       HST_QUERY_CFG_SESS_SHIFT)
#define HST_QUERY_CFG_SET_SEL(rv, va)                                       \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_QUERY_CFG_SEL_SIZE,                              \
                       HST_QUERY_CFG_SEL_SHIFT)
#define HST_QUERY_CFG_SET_RFU2(rv, va)                                      \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_QUERY_CFG_RFU2_SIZE,                             \
                       HST_QUERY_CFG_RFU2_SHIFT)

/* HST_INV_CFG register helper macros                                       */
/* The size of the bit fields in the HST_INV_CFG register.                  */
#define HST_INV_CFG_ALGO_SIZE           6
#define HST_INV_CFG_REP_SIZE            8
#define HST_INV_CFG_AUTOSEL_SIZE        1
#define HST_INV_CFG_RFU1_SIZE          17
/* The number of bits that fields are left-shifted in the HST_INV_CFG       */
/* register.                                                                */
#define HST_INV_CFG_ALGO_SHIFT          0
#define HST_INV_CFG_REP_SHIFT                                               \
    (HST_INV_CFG_ALGO_SHIFT + HST_INV_CFG_ALGO_SIZE)
#define HST_INV_CFG_AUTOSEL_SHIFT                                           \
    (HST_INV_CFG_REP_SHIFT + HST_INV_CFG_REP_SIZE)
#define HST_INV_CFG_RFU1_SHIFT                                              \
    (HST_INV_CFG_AUTOSEL_SHIFT + HST_INV_CFG_AUTOSEL_SIZE)
/* Constants for HST_INV_CFG register bit fields (note that the values are  */
/* already shifted into the low-order bits of the constant.                 */
#define HST_INV_CFG_ALGO_FIXED_Q          0x0
#define HST_INV_CFG_ALGO_ADAPTIVE_Q_TRSH  0x1
/* HST_INV_CFG bit-field retrieval and testing macros                       */
/* rv - the value from the HST_INV_CFG register                             */
#define HST_INV_CFG_GET_ALGO(rv)                                            \
    BIT_FIELD_EXTRACT(rv, HST_INV_CFG_ALGO_SIZE, HST_INV_CFG_ALGO_SHIFT)
#define HST_INV_CFG_GET_REP(rv)                                             \
    BIT_FIELD_EXTRACT(rv, HST_INV_CFG_REP_SIZE, HST_INV_CFG_REP_SHIFT)
#define HST_INV_CFG_AUTOSEL_IS_ENABLED(rv)                                  \
    REGISTER_TEST_BIT(rv, HST_INV_CFG_AUTOSEL_SHIFT)
#define HST_INV_CFG_AUTOSEL_IS_DISABLED(rv)                                 \
    !(REGISTER_TEST_BIT(rv, HST_INV_CFG_AUTOSEL_SHIFT))
#define HST_INV_CFG_GET_RFU1(rv)                                            \
    BIT_FIELD_EXTRACT(rv, HST_INV_CFG_RFU1_SIZE, HST_INV_CFG_RFU1_SHIFT)
/* HST_INV_CFG bit-field value creation macros                              */
/* va - the value to put into the bit field                                 */
#define HST_INV_CFG_ALGO(va)                                                \
    BIT_FIELD_CREATE(va, HST_INV_CFG_ALGO_SIZE, HST_INV_CFG_ALGO_SHIFT)
#define HST_INV_CFG_REP(va)                                                 \
    BIT_FIELD_CREATE(va, HST_INV_CFG_REP_SIZE, HST_INV_CFG_REP_SHIFT)
#define HST_INV_CFG_AUTOSEL_ENABLED                                         \
    CREATE_1BIT_MASK(HST_INV_CFG_AUTOSEL_SHIFT)
#define HST_INV_CFG_AUTOSEL_DISABLED    0
#define HST_INV_CFG_RFU1(va)                                                \
    BIT_FIELD_CREATE(va, HST_INV_CFG_RFU1_SIZE, HST_INV_CFG_RFU1_SHIFT)
/* HST_INV_CFG register value modification macros (i.e., will modify the    */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_INV_CFG_SET_ALGO(rv, va)                                        \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_CFG_ALGO_SIZE,                               \
                       HST_INV_CFG_ALGO_SHIFT)
#define HST_INV_CFG_SET_REP(rv, va)                                         \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_CFG_REP_SIZE,                                \
                       HST_INV_CFG_REP_SHIFT)
#define HST_INV_CFG_SET_AUTOSEL_ENABLED(rv)                                 \
    REGISTER_SET_BIT(rv, HST_INV_CFG_AUTOSEL_SHIFT)
#define HST_INV_CFG_SET_AUTOSEL_DISABLED(rv)                                \
    REGISTER_CLR_BIT(rv, HST_INV_CFG_AUTOSEL_SHIFT)
#define HST_INV_CFG_SET_RFU1(rv, va)                                        \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_CFG_RFU1_SIZE,                               \
                       HST_INV_CFG_RFU1_SHIFT)

/* HST_INV_ALG_PARM_0 register helper macros                                */
/* The size of the bit fields in the HST_INV_ALG_PARM_0 register.           */
/* Algorithm 0 bit field sizes                                              */
#define HST_INV_ALG_PARM_0_ALGO0_Q_SIZE         4
#define HST_INV_ALG_PARM_0_ALGO0_RFU1_SIZE      28
/* Algorithm 1 bit field sizes                                              */
#define HST_INV_ALG_PARM_0_ALGO1_STARTQ_SIZE    4
#define HST_INV_ALG_PARM_0_ALGO1_MAXQ_SIZE      4
#define HST_INV_ALG_PARM_0_ALGO1_MINQ_SIZE      4
#define HST_INV_ALG_PARM_0_ALGO1_TMULT_SIZE     8
#define HST_INV_ALG_PARM_0_ALGO1_RFU1_SIZE      12

/* The number of bits that fields are left-shifted in the HST_INV_ALG_PARM_0*/
/* register.                                                                */
/* Algorithm 0 bit field shift values                                       */
#define HST_INV_ALG_PARM_0_ALGO0_Q_SHIFT    0
#define HST_INV_ALG_PARM_0_ALGO0_RFU1_SHIFT                                 \
    (HST_INV_ALG_PARM_0_ALGO0_Q_SHIFT + HST_INV_ALG_PARM_0_ALGO0_Q_SIZE)
/* Algorithm 1 bit field shift values                                       */
#define HST_INV_ALG_PARM_0_ALGO1_STARTQ_SHIFT   0
#define HST_INV_ALG_PARM_0_ALGO1_MAXQ_SHIFT                                 \
    (HST_INV_ALG_PARM_0_ALGO1_STARTQ_SHIFT +                                \
     HST_INV_ALG_PARM_0_ALGO1_STARTQ_SIZE)
#define HST_INV_ALG_PARM_0_ALGO1_MINQ_SHIFT                                 \
    (HST_INV_ALG_PARM_0_ALGO1_MAXQ_SHIFT + HST_INV_ALG_PARM_0_ALGO1_MAXQ_SIZE)
#define HST_INV_ALG_PARM_0_ALGO1_TMULT_SHIFT                                \
    (HST_INV_ALG_PARM_0_ALGO1_MINQ_SHIFT + HST_INV_ALG_PARM_0_ALGO1_MINQ_SIZE)
#define HST_INV_ALG_PARM_0_ALGO1_RFU1_SHIFT                                 \
    (HST_INV_ALG_PARM_0_ALGO1_TMULT_SHIFT + HST_INV_ALG_PARM_0_ALGO1_TMULT_SIZE)
/* HST_INV_ALG_PARM_0_ALGO0 bit-field retrieval and testing macros          */
/* rv - the value from the HST_INV_ALG_PARM_0 register                      */
/* Algorithm 0 bit field retrieval and testing macros                       */
#define HST_INV_ALG_PARM_0_ALGO0_GET_Q(rv)                                  \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_ALG_PARM_0_ALGO0_Q_SIZE,                      \
                      HST_INV_ALG_PARM_0_ALGO0_Q_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO0_GET_RFU1(rv)                               \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_ALG_PARM_0_ALGO0_RFU1_SIZE,                   \
                      HST_INV_ALG_PARM_0_ALGO0_RFU1_SHIFT)
/* Algorithm 1 bit field retrieval and testing macros                       */
#define HST_INV_ALG_PARM_0_ALGO1_GET_STARTQ(rv)                             \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_ALG_PARM_0_ALGO1_STARTQ_SIZE,                 \
                      HST_INV_ALG_PARM_0_ALGO1_STARTQ_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO1_GET_MAXQ(rv)                               \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_ALG_PARM_0_ALGO1_MAXQ_SIZE,                   \
                      HST_INV_ALG_PARM_0_ALGO1_MAXQ_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO1_GET_MINQ(rv)                               \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_ALG_PARM_0_ALGO1_MINQ_SIZE,                   \
                      HST_INV_ALG_PARM_0_ALGO1_MINQ_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO1_GET_TMULT(rv)                              \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_ALG_PARM_0_ALGO1_TMULT_SIZE,                  \
                      HST_INV_ALG_PARM_0_ALGO1_TMULT_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO1_GET_RFU1(rv)                               \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_ALG_PARM_0_ALGO1_RFU1_SIZE,                   \
                      HST_INV_ALG_PARM_0_ALGO1_RFU1_SHIFT)
/* HST_INV_ALG_PARM_0 bit-field value creation macros                       */
/* va - the value to put into the bit field                                 */
/* Algorithm 0 bit field value creation macros                              */
#define HST_INV_ALG_PARM_0_ALGO0_Q(va)                                      \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_INV_ALG_PARM_0_ALGO0_Q_SIZE,                       \
                     HST_INV_ALG_PARM_0_ALGO0_Q_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO0_RFU1(va)                                   \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_INV_ALG_PARM_0_ALGO0_RFU1_SIZE,                    \
                     HST_INV_ALG_PARM_0_ALGO0_RFU1_SHIFT)
/* Algorithm 1 bit field value creation macros                              */
#define HST_INV_ALG_PARM_0_ALGO1_STARTQ(va)                                 \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_INV_ALG_PARM_0_ALGO1_STARTQ_SIZE,                  \
                     HST_INV_ALG_PARM_0_ALGO1_STARTQ_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO1_MAXQ(va)                                   \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_INV_ALG_PARM_0_ALGO1_MAXQ_SIZE,                    \
                     HST_INV_ALG_PARM_0_ALGO1_MAXQ_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO1_MINQ(va)                                   \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_INV_ALG_PARM_0_ALGO1_MINQ_SIZE,                    \
                     HST_INV_ALG_PARM_0_ALGO1_MINQ_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO1_TMULT(va)                                  \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_INV_ALG_PARM_0_ALGO1_TMULT_SIZE,                   \
                     HST_INV_ALG_PARM_0_ALGO1_TMULT_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO1_RFU1(va)                                   \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_INV_ALG_PARM_0_ALGO1_RFU1_SIZE,                    \
                     HST_INV_ALG_PARM_0_ALGO1_RFU1_SHIFT)
/* HST_INV_ALG_PARM_0 register value modification macros (i.e., will        */
/* modify the field specified within an already existing register value).   */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
/* Algorithm 0 register value modification macros                           */
#define HST_INV_ALG_PARM_0_ALGO0_SET_Q(rv, va)                              \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_ALG_PARM_0_ALGO0_Q_SIZE,                     \
                       HST_INV_ALG_PARM_0_ALGO0_Q_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO0_SET_RFU1(rv, va)                           \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_ALG_PARM_0_ALGO0_RFU1_SIZE,                  \
                       HST_INV_ALG_PARM_0_ALGO0_RFU1_SHIFT)
/* Algorithm 1 register value modification macros                           */
#define HST_INV_ALG_PARM_0_ALGO1_SET_STARTQ(rv, va)                         \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_ALG_PARM_0_ALGO1_STARTQ_SIZE,                \
                       HST_INV_ALG_PARM_0_ALGO1_STARTQ_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO1_SET_MAXQ(rv, va)                           \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_ALG_PARM_0_ALGO1_MAXQ_SIZE,                  \
                       HST_INV_ALG_PARM_0_ALGO1_MAXQ_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO1_SET_MINQ(rv, va)                           \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_ALG_PARM_0_ALGO1_MINQ_SIZE,                  \
                       HST_INV_ALG_PARM_0_ALGO1_MINQ_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO1_SET_TMULT(rv, va)                          \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_ALG_PARM_0_ALGO1_TMULT_SIZE,                 \
                       HST_INV_ALG_PARM_0_ALGO1_TMULT_SHIFT)
#define HST_INV_ALG_PARM_0_ALGO1_SET_RFU1(rv, va)                           \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_ALG_PARM_0_ALGO1_RFU1_SIZE,                  \
                       HST_INV_ALG_PARM_0_ALGO1_RFU1_SHIFT)

/* HST_INV_ALG_PARM_1 register helper macros                                */
/* The size of the bit fields in the HST_INV_ALG_PARM_1 register.           */
/* Algorithm 0 bit field sizes                                              */
#define HST_INV_ALG_PARM_1_ALGO0_RETRY_SIZE     8
#define HST_INV_ALG_PARM_1_ALGO0_RFU1_SIZE      24
/* Algorithm 1 bit field sizes                                              */
#define HST_INV_ALG_PARM_1_ALGO1_RETRY_SIZE     8
#define HST_INV_ALG_PARM_1_ALGO1_RFU1_SIZE      24
/* The number of bits that fields are left-shifted in the HST_INV_ALG_PARM_1*/
/* register.                                                                */
/* Algorithm 0 bit field shift values                                       */
#define HST_INV_ALG_PARM_1_ALGO0_RETRY_SHIFT    0
#define HST_INV_ALG_PARM_1_ALGO0_RFU1_SHIFT                                 \
    (HST_INV_ALG_PARM_1_ALGO0_RETRY_SHIFT +                                 \
     HST_INV_ALG_PARM_1_ALGO0_RETRY_SIZE)
/* Algorithm 1 bit field shift values                                       */
#define HST_INV_ALG_PARM_1_ALGO1_RETRY_SHIFT    0
#define HST_INV_ALG_PARM_1_ALGO1_RFU1_SHIFT                                 \
    (HST_INV_ALG_PARM_1_ALGO1_RETRY_SHIFT +                                 \
     HST_INV_ALG_PARM_1_ALGO1_RETRY_SIZE)
/* HST_INV_ALG_PARM_1 bit-field retrieval and testing macros                */
/* rv - the value from the HST_INV_ALG_PARM_1_ALGO0 register                */
/* Algorithm 0 bit field retrieval and testing macros                       */
#define HST_INV_ALG_PARM_1_ALGO0_GET_RETRY(rv)                              \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_ALG_PARM_1_ALGO0_RETRY_SIZE,                  \
                      HST_INV_ALG_PARM_1_ALGO0_RETRY_SHIFT)
#define HST_INV_ALG_PARM_1_ALGO0_GET_RFU1(rv)                               \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_ALG_PARM_1_ALGO0_RFU1_SIZE,                   \
                      HST_INV_ALG_PARM_1_ALGO0_RFU1_SHIFT)
/* Algorithm 1 bit field retrieval and testing macros                       */
#define HST_INV_ALG_PARM_1_ALGO1_GET_RETRY(rv)                              \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_ALG_PARM_1_ALGO1_RETRY_SIZE,                  \
                      HST_INV_ALG_PARM_1_ALGO1_RETRY_SHIFT)
#define HST_INV_ALG_PARM_1_ALGO1_GET_RFU1(rv)                               \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_ALG_PARM_1_ALGO1_RFU1_SIZE,                   \
                      HST_INV_ALG_PARM_1_ALGO1_RFU1_SHIFT)
/* HST_INV_ALG_PARM_1 bit-field value creation macros                       */
/* va - the value to put into the bit field                                 */
/* Algorithm 0 bit field value creation macros                              */
#define HST_INV_ALG_PARM_1_ALGO0_RETRY(va)                                  \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_INV_ALG_PARM_1_ALGO0_RETRY_SIZE,                   \
                     HST_INV_ALG_PARM_1_ALGO0_RETRY_SHIFT)
#define HST_INV_ALG_PARM_1_ALGO0_RFU1(va)                                   \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_INV_ALG_PARM_1_ALGO0_RFU1_SIZE,                    \
                     HST_INV_ALG_PARM_1_ALGO0_RFU1_SHIFT)
/* Algorithm 3 bit field value creation macros                              */
#define HST_INV_ALG_PARM_1_ALGO1_RETRY(va)                                  \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_INV_ALG_PARM_1_ALGO1_RETRY_SIZE,                   \
                     HST_INV_ALG_PARM_1_ALGO1_RETRY_SHIFT)
#define HST_INV_ALG_PARM_1_ALGO1_RFU1(va)                                   \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_INV_ALG_PARM_1_ALGO1_RFU1_SIZE,                    \
                     HST_INV_ALG_PARM_1_ALGO1_RFU1_SHIFT)
/* HST_INV_ALG_PARM_1 register value modification macros (i.e., will        */
/* modify the field specified within an already existing register value).   */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
/* Algorithm 0 register value modification macros                           */
#define HST_INV_ALG_PARM_1_ALGO0_SET_RETRY(rv, va)                          \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_ALG_PARM_1_ALGO0_RETRY_SIZE,                 \
                       HST_INV_ALG_PARM_1_ALGO0_RETRY_SHIFT)
#define HST_INV_ALG_PARM_1_ALGO0_SET_RFU1(rv, va)                           \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_ALG_PARM_1_ALGO0_RFU1_SIZE,                  \
                       HST_INV_ALG_PARM_1_ALGO0_RFU1_SHIFT)
/* Algorithm 3 register value modification macros                           */
#define HST_INV_ALG_PARM_1_ALGO1_SET_RETRY(rv, va)                          \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_ALG_PARM_1_ALGO1_RETRY_SIZE,                 \
                       HST_INV_ALG_PARM_1_ALGO1_RETRY_SHIFT)
#define HST_INV_ALG_PARM_1_ALGO1_SET_RFU1(rv, va)                           \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_ALG_PARM_1_ALGO1_RFU1_SIZE,                  \
                       HST_INV_ALG_PARM_1_ALGO1_RFU1_SHIFT)

/* HST_INV_ALG_PARM_2 register helper macros                                */
/* The size of the bit fields in the HST_INV_ALG_PARM_2 register.           */
/* Algorithm 0 bit field sizes                                              */
#define HST_INV_ALG_PARM_2_ALGO0_TOGGLE_SIZE    1
#define HST_INV_ALG_PARM_2_ALGO0_REPEAT_SIZE    1
#define HST_INV_ALG_PARM_2_ALGO0_RFU1_SIZE      30
/* Algorithm 1 bit field sizes                                              */
#define HST_INV_ALG_PARM_2_ALGO1_TOGGLE_SIZE    1
#define HST_INV_ALG_PARM_2_ALGO1_RFU1_SIZE      31
/* The number of bits that fields are left-shifted in the HST_INV_ALG_PARM_2*/
/* register.                                                                */
/* Algorithm 0 bit field shift values                                       */
#define HST_INV_ALG_PARM_2_ALGO0_TOGGLE_SHIFT   0
#define HST_INV_ALG_PARM_2_ALGO0_REPEAT_SHIFT                               \
    (HST_INV_ALG_PARM_2_ALGO0_TOGGLE_SHIFT +                                \
     HST_INV_ALG_PARM_2_ALGO0_TOGGLE_SIZE)
#define HST_INV_ALG_PARM_2_ALGO0_RFU1_SHIFT                                 \
    (HST_INV_ALG_PARM_2_ALGO0_REPEAT_SHIFT +                                \
     HST_INV_ALG_PARM_2_ALGO0_REPEAT_SIZE)
/* Algorithm 1 bit field shift values                                       */
#define HST_INV_ALG_PARM_2_ALGO1_TOGGLE_SHIFT   0
#define HST_INV_ALG_PARM_2_ALGO1_RFU1_SHIFT                                 \
    (HST_INV_ALG_PARM_2_ALGO1_TOGGLE_SHIFT +                                \
     HST_INV_ALG_PARM_2_ALGO1_TOGGLE_SIZE)
/* HST_INV_ALG_PARM_2 bit-field retrieval and testing macros                */
/* rv - the value from the HST_INV_ALG_PARM_2_ALGO0 register                */
/* Algorithm 0 bit field retrieval and testing macros                       */
#define HST_INV_ALG_PARM_2_ALGO0_TOGGLE_IS_ENABLED(rv)                      \
    REGISTER_TEST_BIT(rv, HST_INV_ALG_PARM_2_ALGO0_TOGGLE_SHIFT)
#define HST_INV_ALG_PARM_2_ALGO0_TOGGLE_IS_DISABLED(rv)                     \
    !(REGISTER_TEST_BIT(rv, HST_INV_ALG_PARM_2_ALGO0_TOGGLE_SHIFT))
#define HST_INV_ALG_PARM_2_ALGO0_REPEAT_IS_ENABLED(rv)                      \
    REGISTER_TEST_BIT(rv, HST_INV_ALG_PARM_2_ALGO0_REPEAT_SHIFT)
#define HST_INV_ALG_PARM_2_ALGO0_REPEAT_IS_DISABLED(rv)                     \
    !(REGISTER_TEST_BIT(rv, HST_INV_ALG_PARM_2_ALGO0_REPEAT_SHIFT))
#define HST_INV_ALG_PARM_2_ALGO0_GET_RFU1(rv)                               \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_ALG_PARM_2_ALGO0_RFU1_SIZE,                   \
                      HST_INV_ALG_PARM_2_ALGO0_RFU1_SHIFT)
/* Algorithm 1 bit field retrieval and testing macros                       */
#define HST_INV_ALG_PARM_2_ALGO1_TOGGLE_IS_ENABLED(rv)                      \
    REGISTER_TEST_BIT(rv, HST_INV_ALG_PARM_2_ALGO1_TOGGLE_SHIFT)
#define HST_INV_ALG_PARM_2_ALGO1_TOGGLE_IS_DISABLED(rv)                     \
    !(REGISTER_TEST_BIT(rv, HST_INV_ALG_PARM_2_ALGO1_TOGGLE_SHIFT))
#define HST_INV_ALG_PARM_2_ALGO1_GET_RFU1(rv)                               \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_ALG_PARM_2_ALGO1_RFU1_SIZE,                   \
                      HST_INV_ALG_PARM_2_ALGO1_RFU1_SHIFT)
/* HST_INV_ALG_PARM_2 bit-field value creation macros                       */
/* va - the value to put into the bit field                                 */
/* Algorithm 0 bit field value creation macros                              */
#define HST_INV_ALG_PARM_2_ALGO0_TOGGLE_ENABLED                             \
    CREATE_1BIT_MASK(HST_INV_ALG_PARM_2_ALGO0_TOGGLE_SHIFT)
#define HST_INV_ALG_PARM_2_ALGO0_TOGGLE_DISABLED    0
#define HST_INV_ALG_PARM_2_ALGO0_REPEAT_ENABLED                             \
    CREATE_1BIT_MASK(HST_INV_ALG_PARM_2_ALGO0_REPEAT_SHIFT)
#define HST_INV_ALG_PARM_2_ALGO0_REPEAT_DISABLED    0
#define HST_INV_ALG_PARM_2_ALGO0_RFU1(va)                                   \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_INV_ALG_PARM_2_ALGO0_RFU1_SIZE,                    \
                     HST_INV_ALG_PARM_2_ALGO0_RFU1_SHIFT)
/* Algorithm 1 bit field value creation macros                              */
#define HST_INV_ALG_PARM_2_ALGO1_TOGGLE_ENABLED                             \
    CREATE_1BIT_MASK(HST_INV_ALG_PARM_2_ALGO1_TOGGLE_SHIFT)
#define HST_INV_ALG_PARM_2_ALGO1_TOGGLE_DISABLED    0
#define HST_INV_ALG_PARM_2_ALGO1_RFU1(va)                                   \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_INV_ALG_PARM_2_ALGO1_RFU1_SIZE,                    \
                     HST_INV_ALG_PARM_2_ALGO1_RFU1_SHIFT)
/* HST_INV_ALG_PARM_2 register value modification macros (i.e., will        */
/* modify the field specified within an already existing register value).   */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
/* Algorithm 0 register value modification macros                           */
#define HST_INV_ALG_PARM_2_ALGO0_SET_TOGGLE_ENABLED(rv)                     \
    REGISTER_SET_BIT(rv, HST_INV_ALG_PARM_2_ALGO0_TOGGLE_SHIFT)
#define HST_INV_ALG_PARM_2_ALGO0_SET_TOGGLE_DISABLED(rv)                    \
    REGISTER_CLR_BIT(rv, HST_INV_ALG_PARM_2_ALGO0_TOGGLE_SHIFT)
#define HST_INV_ALG_PARM_2_ALGO0_SET_REPEAT_ENABLED(rv)                     \
    REGISTER_SET_BIT(rv, HST_INV_ALG_PARM_2_ALGO0_REPEAT_SHIFT)
#define HST_INV_ALG_PARM_2_ALGO0_SET_REPEAT_DISABLED(rv)                    \
    REGISTER_CLR_BIT(rv, HST_INV_ALG_PARM_2_ALGO0_REPEAT_SHIFT)
#define HST_INV_ALG_PARM_2_ALGO0_SET_RFU1(rv, va)                           \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_ALG_PARM_2_ALGO0_RFU1_SIZE,                  \
                       HST_INV_ALG_PARM_2_ALGO0_RFU1_SHIFT)
/* Algorithm 1 register value modification macros                           */
#define HST_INV_ALG_PARM_2_ALGO1_SET_TOGGLE_ENABLED(rv)                     \
    REGISTER_SET_BIT(rv, HST_INV_ALG_PARM_2_ALGO1_TOGGLE_SHIFT)
#define HST_INV_ALG_PARM_2_ALGO1_SET_TOGGLE_DISABLED(rv)                    \
    REGISTER_CLR_BIT(rv, HST_INV_ALG_PARM_2_ALGO1_TOGGLE_SHIFT)
#define HST_INV_ALG_PARM_2_ALGO1_SET_RFU1(rv, va)                           \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_ALG_PARM_2_ALGO1_RFU1_SIZE,                  \
                       HST_INV_ALG_PARM_2_ALGO1_RFU1_SHIFT)

/* HST_INV_EPC_MATCH_CFG register helper macros                             */
/* The size of the bit fields in the HST_INV_EPC_MATCH_CFG register.        */
#define HST_INV_EPC_MATCH_CFG_ENABLE_SIZE   1
#define HST_INV_EPC_MATCH_CFG_MATCH_SIZE    1
#define HST_INV_EPC_MATCH_CFG_LEN_SIZE      9
#define HST_INV_EPC_MATCH_CFG_OFF_SIZE      9
#define HST_INV_EPC_MATCH_CFG_RFU1_SIZE     12
/* The number of bits that fields are left-shifted in the                   */
/* HST_INV_EPC_MATCH_CFG register.                                          */
#define HST_INV_EPC_MATCH_CFG_ENABLE_SHIFT  0
#define HST_INV_EPC_MATCH_CFG_MATCH_SHIFT                                   \
    (HST_INV_EPC_MATCH_CFG_ENABLE_SHIFT + HST_INV_EPC_MATCH_CFG_ENABLE_SIZE)
#define HST_INV_EPC_MATCH_CFG_LEN_SHIFT                                     \
    (HST_INV_EPC_MATCH_CFG_MATCH_SHIFT + HST_INV_EPC_MATCH_CFG_MATCH_SIZE)
#define HST_INV_EPC_MATCH_CFG_OFF_SHIFT                                     \
    (HST_INV_EPC_MATCH_CFG_LEN_SHIFT + HST_INV_EPC_MATCH_CFG_LEN_SIZE)
#define HST_INV_EPC_MATCH_CFG_RFU1_SHIFT                                    \
    (HST_INV_EPC_MATCH_CFG_OFF_SHIFT + HST_INV_EPC_MATCH_CFG_OFF_SIZE)
/* Constants for HST_INV_EPC_MATCH_CFG register bit fields (note that the   */
/* values are already shifted into the low-order bits of the constant.      */
#define HST_INV_EPC_MATCH_CFG_MATCH_YES 0x0
#define HST_INV_EPC_MATCH_CFG_MATCH_NO  0x1
/* HST_INV_EPC_MATCH_CFG bit-field retrieval and testing macros             */
/* rv - the value from the HST_INV_EPC_MATCH_CFG register                   */
#define HST_INV_EPC_MATCH_CFG_IS_ENABLED(rv)                                \
    REGISTER_TEST_BIT(rv, HST_INV_EPC_MATCH_CFG_ENABLE_SHIFT)
#define HST_INV_EPC_MATCH_CFG_IS_DISABLED(rv)                               \
    !(REGISTER_TEST_BIT(rv, HST_INV_EPC_MATCH_CFG_ENABLE_SHIFT))
#define HST_INV_EPC_MATCH_CFG_GET_MATCH(rv)                                 \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_EPC_MATCH_CFG_MATCH_SIZE,                     \
                      HST_INV_EPC_MATCH_CFG_MATCH_SHIFT)
#define HST_INV_EPC_MATCH_CFG_GET_LEN(rv)                                   \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_EPC_MATCH_CFG_LEN_SIZE,                       \
                      HST_INV_EPC_MATCH_CFG_LEN_SHIFT)
#define HST_INV_EPC_MATCH_CFG_GET_OFF(rv)                                   \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_EPC_MATCH_CFG_OFF_SIZE,                       \
                      HST_INV_EPC_MATCH_CFG_OFF_SHIFT)
#define HST_INV_EPC_MATCH_CFG_GET_RFU1(rv)                                  \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_INV_EPC_MATCH_CFG_RFU1_SIZE,                      \
                      HST_INV_EPC_MATCH_CFG_RFU1_SHIFT)
/* HST_INV_EPC_MATCH_CFG bit-field value creation macros                    */
/* va - the value to put into the bit field                                 */
#define HST_INV_EPC_MATCH_CFG_ENABLED                                       \
    CREATE_1BIT_MASK(HST_INV_EPC_MATCH_CFG_ENABLE_SHIFT)
#define HST_INV_EPC_MATCH_CFG_DISABLED  0
#define HST_INV_EPC_MATCH_CFG_MATCH(rv)                                     \
    BIT_FIELD_CREATE(rv,                                                    \
                     HST_INV_EPC_MATCH_CFG_MATCH_SIZE,                      \
                     HST_INV_EPC_MATCH_CFG_MATCH_SHIFT)
#define HST_INV_EPC_MATCH_CFG_LEN(rv)                                       \
    BIT_FIELD_CREATE(rv,                                                    \
                     HST_INV_EPC_MATCH_CFG_LEN_SIZE,                        \
                     HST_INV_EPC_MATCH_CFG_LEN_SHIFT)
#define HST_INV_EPC_MATCH_CFG_OFF(rv)                                       \
    BIT_FIELD_CREATE(rv,                                                    \
                     HST_INV_EPC_MATCH_CFG_OFF_SIZE,                        \
                     HST_INV_EPC_MATCH_CFG_OFF_SHIFT)
#define HST_INV_EPC_MATCH_CFG_RFU1(va)                                      \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_INV_EPC_MATCH_CFG_RFU1_SIZE,                       \
                     HST_INV_EPC_MATCH_CFG_RFU1_SHIFT)
/* HST_INV_EPC_MATCH_CFG register value modification macros (i.e., will     */
/* modify the field specified within an already existing register value).   */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_INV_EPC_MATCH_CFG_SET_ENABLED(rv)                               \
    REGISTER_SET_BIT(rv, HST_INV_EPC_MATCH_CFG_ENABLE_SHIFT)
#define HST_INV_EPC_MATCH_CFG_SET_DISABLED(rv)                              \
    REGISTER_CLR_BIT(rv, HST_INV_EPC_MATCH_CFG_ENABLE_SHIFT)
#define HST_INV_EPC_MATCH_CFG_SET_MATCH(rv, va)                             \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_EPC_MATCH_CFG_MATCH_SIZE,                    \
                       HST_INV_EPC_MATCH_CFG_MATCH_SHIFT)
#define HST_INV_EPC_MATCH_CFG_SET_LEN(rv, va)                               \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_EPC_MATCH_CFG_LEN_SIZE,                      \
                       HST_INV_EPC_MATCH_CFG_LEN_SHIFT)
#define HST_INV_EPC_MATCH_CFG_SET_OFF(rv, va)                               \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_EPC_MATCH_CFG_OFF_SIZE,                      \
                       HST_INV_EPC_MATCH_CFG_OFF_SHIFT)
#define HST_INV_EPC_MATCH_CFG_SET_RFU1(rv, va)                              \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_INV_EPC_MATCH_CFG_RFU1_SIZE,                     \
                       HST_INV_EPC_MATCH_CFG_RFU1_SHIFT)

/* HST_TAGACC_DESC_CFG register helper macros                               */
/* The size of the bit fields in the HST_TAGACC_DESC_CFG register.          */
/* NOTE:  The HST_TAGACC_DESC_CFG_VERIFY has been deprecated and will be    */
/*        redefined in the next release.  It is retained here for the       */
/*        current release, only for backward compatibility with             */
#define HST_TAGACC_DESC_CFG_VERIFY_SIZE 1
#define HST_TAGACC_DESC_CFG_RETRY_SIZE  3
#define HST_TAGACC_DESC_CFG_RFU1_SIZE   28
/* The number of bits that fields are left-shifted in the                   */
/* HST_TAGACC_DESC_CFG register.                                            */
#define HST_TAGACC_DESC_CFG_VERIFY_SHIFT    0
#define HST_TAGACC_DESC_CFG_RETRY_SHIFT                                     \
    (HST_TAGACC_DESC_CFG_VERIFY_SHIFT + HST_TAGACC_DESC_CFG_VERIFY_SIZE)
#define HST_TAGACC_DESC_CFG_RFU1_SHIFT                                      \
    (HST_TAGACC_DESC_CFG_RETRY_SHIFT + HST_TAGACC_DESC_CFG_RETRY_SIZE)
/* HST_TAGACC_DESC_CFG bit-field retrieval and testing macros               */
/* rv - the value from the HST_TAGACC_DESC_CFG register                     */
#define HST_TAGACC_DESC_CFG_VERIFY_IS_ENABLED(rv)                           \
    REGISTER_TEST_BIT(rv, HST_TAGACC_DESC_CFG_VERIFY_SHIFT)
#define HST_TAGACC_DESC_CFG_VERIFY_IS_DISABLED(rv)                          \
    !(REGISTER_TEST_BIT(rv, HST_TAGACC_DESC_CFG_VERIFY_SHIFT))
#define HST_TAGACC_DESC_CFG_GET_RETRY(rv)                                   \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGACC_DESC_CFG_RETRY_SIZE,                       \
                      HST_TAGACC_DESC_CFG_RETRY_SHIFT)
#define HST_TAGACC_DESC_CFG_GET_RFU1(rv)                                    \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGACC_DESC_CFG_RFU1_SIZE,                        \
                      HST_TAGACC_DESC_CFG_RFU1_SHIFT)
/* HST_TAGACC_DESC_CFG bit-field value creation macros                      */
/* va - the value to put into the bit field                                 */
#define HST_TAGACC_DESC_CFG_VERIFY_ENABLED                                  \
    CREATE_1BIT_MASK(HST_TAGACC_DESC_CFG_VERIFY_SHIFT)
#define HST_TAGACC_DESC_CFG_VERIFY_DISABLED   0
#define HST_TAGACC_DESC_CFG_RETRY(va)                                       \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGACC_DESC_CFG_RETRY_SIZE,                        \
                     HST_TAGACC_DESC_CFG_RETRY_SHIFT)
#define HST_TAGACC_DESC_CFG_RFU1(va)                                        \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGACC_DESC_CFG_RFU1_SIZE,                         \
                     HST_TAGACC_DESC_CFG_RFU1_SHIFT)
/* HST_TAGACC_DESC_CFG register value modification macros (i.e., will modify*/
/* the field specified within an already existing register value).          */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_TAGACC_DESC_CFG_SET_VERIFY_ENABLED(rv)                          \
    REGISTER_SET_BIT(rv, HST_TAGACC_DESC_CFG_VERIFY_SHIFT)
#define HST_TAGACC_DESC_CFG_SET_VERIFY_DISABLED(rv)                         \
    REGISTER_CLR_BIT(rv, HST_TAGACC_DESC_CFG_VERIFY_SHIFT)
#define HST_TAGACC_DESC_CFG_SET_RETRY(rv, va)                               \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_DESC_CFG_RETRY_SIZE,                      \
                       HST_TAGACC_DESC_CFG_RETRY_SHIFT)
#define HST_TAGACC_DESC_CFG_SET_RFU1(rv, va)                                \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_DESC_CFG_RFU1_SIZE,                       \
                       HST_TAGACC_DESC_CFG_RFU1_SHIFT)

/* HST_TAGACC_BANK register helper macros                                   */
/* The size of the bit fields in the HST_TAGACC_BANK register.              */
#define HST_TAGACC_BANK_BANK_SIZE   2
#define HST_TAGACC_BANK_RFU1_SIZE   30
/* The number of bits that fields are left-shifted in the HST_TAGACC_BANK   */
/* register.                                                                */
#define HST_TAGACC_BANK_BANK_SHIFT  0
#define HST_TAGACC_BANK_RFU1_SHIFT                                          \
    (HST_TAGACC_BANK_BANK_SHIFT + HST_TAGACC_BANK_BANK_SIZE)
/* Constants for HST_TAGACC_BANK register bit fields (note that the values  */
/* are already shifted into the low-order bits of the constant.             */
#define HST_TAGACC_BANK_BANK_RESERVED   0x0
#define HST_TAGACC_BANK_BANK_EPC        0x1
#define HST_TAGACC_BANK_BANK_TID        0x2
#define HST_TAGACC_BANK_BANK_USER       0x3
/* HST_TAGACC_BANK bit-field retrieval and testing macros                   */
/* rv - the value from the HST_TAGACC_BANK register                         */
#define HST_TAGACC_BANK_GET_BANK(rv)                                        \
    BIT_FIELD_EXTRACT(rv, HST_TAGACC_BANK_BANK_SIZE, HST_TAGACC_BANK_BANK_SHIFT)
#define HST_TAGACC_BANK_GET_RFU1(rv)                                        \
    BIT_FIELD_EXTRACT(rv, HST_TAGACC_BANK_RFU1_SIZE, HST_TAGACC_BANK_RFU1_SHIFT)
/* HST_TAGACC_BANK bit-field value creation macros                          */
/* va - the value to put into the bit field                                 */
#define HST_TAGACC_BANK_BANK(va)                                            \
    BIT_FIELD_CREATE(va, HST_TAGACC_BANK_BANK_SIZE, HST_TAGACC_BANK_BANK_SHIFT)
#define HST_TAGACC_BANK_RFU1(va)                                            \
    BIT_FIELD_CREATE(va, HST_TAGACC_BANK_RFU1_SIZE, HST_TAGACC_BANK_RFU1_SHIFT)
/* HST_TAGACC_BANK register value modification macros (i.e., will modify the*/
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_TAGACC_BANK_SET_BANK(rv, va)                                    \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_BANK_BANK_SIZE,                           \
                       HST_TAGACC_BANK_BANK_SHIFT)
#define HST_TAGACC_BANK_SET_RFU1(rv, va)                                    \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_BANK_RFU1_SIZE,                           \
                       HST_TAGACC_BANK_RFU1_SHIFT)

/* HST_TAGACC_CNT register helper macros                                    */
/* The size of the bit fields in the HST_TAGACC_CNT register.               */
#define HST_TAGACC_CNT_LEN_SIZE     16
#define HST_TAGACC_CNT_RFU1_SIZE    16
/* The number of bits that fields are left-shifted in the HST_TAGACC_CNT    */
/* register.                                                                */
#define HST_TAGACC_CNT_LEN_SHIFT    0
#define HST_TAGACC_CNT_RFU1_SHIFT                                           \
    (HST_TAGACC_CNT_LEN_SHIFT + HST_TAGACC_CNT_LEN_SIZE)
/* HST_TAGACC_CNT bit-field retrieval and testing macros                    */
/* rv - the value from the HST_TAGACC_CNT register                          */
#define HST_TAGACC_CNT_GET_LEN(rv)                                          \
    BIT_FIELD_EXTRACT(rv, HST_TAGACC_CNT_LEN_SIZE, HST_TAGACC_CNT_LEN_SHIFT)
#define HST_TAGACC_CNT_GET_RFU1(rv)                                         \
    BIT_FIELD_EXTRACT(rv, HST_TAGACC_CNT_RFU1_SIZE, HST_TAGACC_CNT_RFU1_SHIFT)
/* HST_TAGACC_CNT bit-field value creation macros                           */
/* va - the value to put into the bit field                                 */
#define HST_TAGACC_CNT_LEN(va)                                              \
    BIT_FIELD_CREATE(va, HST_TAGACC_CNT_LEN_SIZE, HST_TAGACC_CNT_LEN_SHIFT)
#define HST_TAGACC_CNT_RFU1(va)                                             \
    BIT_FIELD_CREATE(va, HST_TAGACC_CNT_RFU1_SIZE, HST_TAGACC_CNT_RFU1_SHIFT)
/* HST_TAGACC_CNT register value modification macros (i.e., will modify the */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_TAGACC_CNT_SET_LEN(rv, va)                                      \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_CNT_LEN_SIZE,                             \
                       HST_TAGACC_CNT_LEN_SHIFT)
#define HST_TAGACC_CNT_SET_RFU1(rv, va)                                     \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_CNT_RFU1_SIZE,                            \
                       HST_TAGACC_CNT_RFU1_SHIFT)

/* HST_TAGACC_LOCKCFG register helper macros                                */
/* The size of the bit fields in the HST_TAGACC_LOCKCFG register.           */
#define HST_TAGACC_LOCKCFG_ACTION_USER_SIZE 2
#define HST_TAGACC_LOCKCFG_ACTION_TID_SIZE  2
#define HST_TAGACC_LOCKCFG_ACTION_EPC_SIZE  2
#define HST_TAGACC_LOCKCFG_ACTION_ACC_SIZE  2
#define HST_TAGACC_LOCKCFG_ACTION_KILL_SIZE 2
#define HST_TAGACC_LOCKCFG_MASK_USER_SIZE   2
#define HST_TAGACC_LOCKCFG_MASK_TID_SIZE    2
#define HST_TAGACC_LOCKCFG_MASK_EPC_SIZE    2
#define HST_TAGACC_LOCKCFG_MASK_ACC_SIZE    2
#define HST_TAGACC_LOCKCFG_MASK_KILL_SIZE   2
#define HST_TAGACC_LOCKCFG_RFU1_SIZE        12
/* The number of bits that fields are left-shifted in the HST_TAGACC_LOCKCFG*/
/* register.                                                                */
#define HST_TAGACC_LOCKCFG_ACTION_USER_SHIFT    0
#define HST_TAGACC_LOCKCFG_ACTION_TID_SHIFT                                 \
    (HST_TAGACC_LOCKCFG_ACTION_USER_SHIFT + HST_TAGACC_LOCKCFG_ACTION_USER_SIZE)
#define HST_TAGACC_LOCKCFG_ACTION_EPC_SHIFT                                 \
    (HST_TAGACC_LOCKCFG_ACTION_TID_SHIFT + HST_TAGACC_LOCKCFG_ACTION_TID_SIZE)
#define HST_TAGACC_LOCKCFG_ACTION_ACC_SHIFT                                 \
    (HST_TAGACC_LOCKCFG_ACTION_EPC_SHIFT + HST_TAGACC_LOCKCFG_ACTION_EPC_SIZE)
#define HST_TAGACC_LOCKCFG_ACTION_KILL_SHIFT                                \
    (HST_TAGACC_LOCKCFG_ACTION_ACC_SHIFT + HST_TAGACC_LOCKCFG_ACTION_ACC_SIZE)
#define HST_TAGACC_LOCKCFG_MASK_USER_SHIFT                                  \
    (HST_TAGACC_LOCKCFG_ACTION_KILL_SHIFT + HST_TAGACC_LOCKCFG_ACTION_KILL_SIZE)
#define HST_TAGACC_LOCKCFG_MASK_TID_SHIFT                                   \
    (HST_TAGACC_LOCKCFG_MASK_USER_SHIFT + HST_TAGACC_LOCKCFG_MASK_USER_SIZE)
#define HST_TAGACC_LOCKCFG_MASK_EPC_SHIFT                                   \
    (HST_TAGACC_LOCKCFG_MASK_TID_SHIFT + HST_TAGACC_LOCKCFG_MASK_TID_SIZE)
#define HST_TAGACC_LOCKCFG_MASK_ACC_SHIFT                                   \
    (HST_TAGACC_LOCKCFG_MASK_EPC_SHIFT + HST_TAGACC_LOCKCFG_MASK_EPC_SIZE)
#define HST_TAGACC_LOCKCFG_MASK_KILL_SHIFT                                  \
    (HST_TAGACC_LOCKCFG_MASK_ACC_SHIFT + HST_TAGACC_LOCKCFG_MASK_ACC_SIZE)
#define HST_TAGACC_LOCKCFG_RFU1_SHIFT                                       \
    (HST_TAGACC_LOCKCFG_MASK_KILL_SHIFT + HST_TAGACC_LOCKCFG_MASK_KILL_SIZE)
/* Constants for HST_TAGACC_LOCKCFG register bit fields (note that the      */
/* values are already shifted into the low-order bits of the constant.      */
#define HST_TAGACC_LOCKCFG_ACTION_MEM_WRITE         0x0
#define HST_TAGACC_LOCKCFG_ACTION_MEM_PERM_WRITE    0x1
#define HST_TAGACC_LOCKCFG_ACTION_MEM_SEC_WRITE     0x2
#define HST_TAGACC_LOCKCFG_ACTION_MEM_NO_WRITE      0x3
#define HST_TAGACC_LOCKCFG_ACTION_PWD_ACCESS        0x0
#define HST_TAGACC_LOCKCFG_ACTION_PWD_PERM_ACCESS   0x1
#define HST_TAGACC_LOCKCFG_ACTION_PWD_SEC_ACCESS    0x2
#define HST_TAGACC_LOCKCFG_ACTION_PWD_NO_ACCESS     0x3
#define HST_TAGACC_LOCKCFG_MASK_USE_NO_ACTION       0x0
#define HST_TAGACC_LOCKCFG_MASK_USE_PWD_ACTION      0x1
#define HST_TAGACC_LOCKCFG_MASK_USE_PERMA_ACTION    0x2
#define HST_TAGACC_LOCKCFG_MASK_USE_BOTH_ACTION                             \
    (HST_TAGACC_LOCKCFG_MASK_USE_PWD_ACTION |                               \
     HST_TAGACC_LOCKCFG_MASK_USE_PERMA_ACTION)
/* HST_TAGACC_LOCKCFG bit-field retrieval and testing macros                */
/* rv - the value from the HST_TAGACC_LOCKCFG register                      */
#define HST_TAGACC_LOCKCFG_GET_ACTION_USER(rv)                              \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGACC_LOCKCFG_ACTION_USER_SIZE,                  \
                      HST_TAGACC_LOCKCFG_ACTION_USER_SHIFT)
#define HST_TAGACC_LOCKCFG_GET_ACTION_TID(rv)                               \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGACC_LOCKCFG_ACTION_TID_SIZE,                   \
                      HST_TAGACC_LOCKCFG_ACTION_TID_SHIFT)
#define HST_TAGACC_LOCKCFG_GET_ACTION_EPC(rv)                               \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGACC_LOCKCFG_ACTION_EPC_SIZE,                   \
                      HST_TAGACC_LOCKCFG_ACTION_EPC_SHIFT)
#define HST_TAGACC_LOCKCFG_GET_ACTION_ACC(rv)                               \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGACC_LOCKCFG_ACTION_ACC_SIZE,                   \
                      HST_TAGACC_LOCKCFG_ACTION_ACC_SHIFT)
#define HST_TAGACC_LOCKCFG_GET_ACTION_KILL(rv)                              \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGACC_LOCKCFG_ACTION_KILL_SIZE,                  \
                      HST_TAGACC_LOCKCFG_ACTION_KILL_SHIFT)
#define HST_TAGACC_LOCKCFG_GET_MASK_USER(rv)                                \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGACC_LOCKCFG_MASK_USER_SIZE,                    \
                      HST_TAGACC_LOCKCFG_MASK_USER_SHIFT)
#define HST_TAGACC_LOCKCFG_GET_MASK_TID(rv)                                 \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGACC_LOCKCFG_MASK_TID_SIZE,                     \
                      HST_TAGACC_LOCKCFG_MASK_TID_SHIFT)
#define HST_TAGACC_LOCKCFG_GET_MASK_EPC(rv)                                 \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGACC_LOCKCFG_MASK_EPC_SIZE,                     \
                      HST_TAGACC_LOCKCFG_MASK_EPC_SHIFT)
#define HST_TAGACC_LOCKCFG_GET_MASK_ACC(rv)                                 \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGACC_LOCKCFG_MASK_ACC_SIZE,                     \
                      HST_TAGACC_LOCKCFG_MASK_ACC_SHIFT)
#define HST_TAGACC_LOCKCFG_GET_MASK_KILL(rv)                                \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGACC_LOCKCFG_MASK_KILL_SIZE,                    \
                      HST_TAGACC_LOCKCFG_MASK_KILL_SHIFT)
#define HST_TAGACC_LOCKCFG_GET_RFU1(rv)                                     \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_TAGACC_LOCKCFG_RFU1_SIZE,                         \
                      HST_TAGACC_LOCKCFG_RFU1_SHIFT)
/* HST_TAGACC_LOCKCFG bit-field value creation macros                       */
/* va - the value to put into the bit field                                 */
#define HST_TAGACC_LOCKCFG_ACTION_USER(va)                                  \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGACC_LOCKCFG_ACTION_USER_SIZE,                   \
                     HST_TAGACC_LOCKCFG_ACTION_USER_SHIFT)
#define HST_TAGACC_LOCKCFG_ACTION_TID(va)                                   \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGACC_LOCKCFG_ACTION_TID_SIZE,                    \
                     HST_TAGACC_LOCKCFG_ACTION_TID_SHIFT)
#define HST_TAGACC_LOCKCFG_ACTION_EPC(va)                                   \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGACC_LOCKCFG_ACTION_EPC_SIZE,                    \
                     HST_TAGACC_LOCKCFG_ACTION_EPC_SHIFT)
#define HST_TAGACC_LOCKCFG_ACTION_ACC(va)                                   \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGACC_LOCKCFG_ACTION_ACC_SIZE,                    \
                     HST_TAGACC_LOCKCFG_ACTION_ACC_SHIFT)
#define HST_TAGACC_LOCKCFG_ACTION_KILL(va)                                  \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGACC_LOCKCFG_ACTION_KILL_SIZE,                   \
                     HST_TAGACC_LOCKCFG_ACTION_KILL_SHIFT)
#define HST_TAGACC_LOCKCFG_MASK_USER(va)                                    \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGACC_LOCKCFG_MASK_USER_SIZE,                     \
                     HST_TAGACC_LOCKCFG_MASK_USER_SHIFT)
#define HST_TAGACC_LOCKCFG_MASK_TID(va)                                     \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGACC_LOCKCFG_MASK_TID_SIZE,                      \
                     HST_TAGACC_LOCKCFG_MASK_TID_SHIFT)
#define HST_TAGACC_LOCKCFG_MASK_EPC(va)                                     \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGACC_LOCKCFG_MASK_EPC_SIZE,                      \
                     HST_TAGACC_LOCKCFG_MASK_EPC_SHIFT)
#define HST_TAGACC_LOCKCFG_MASK_ACC(va)                                     \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGACC_LOCKCFG_MASK_ACC_SIZE,                      \
                     HST_TAGACC_LOCKCFG_MASK_ACC_SHIFT)
#define HST_TAGACC_LOCKCFG_MASK_KILL(va)                                    \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGACC_LOCKCFG_MASK_KILL_SIZE,                     \
                     HST_TAGACC_LOCKCFG_MASK_KILL_SHIFT)
#define HST_TAGACC_LOCKCFG_RFU1(va)                                         \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_TAGACC_LOCKCFG_RFU1_SIZE,                          \
                     HST_TAGACC_LOCKCFG_RFU1_SHIFT)
/* HST_TAGACC_LOCKCFG register value modification macros (i.e., will modify */
/* the field specified within an already existing register value).          */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_TAGACC_LOCKCFG_SET_ACTION_USER(rv, va)                          \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_LOCKCFG_ACTION_USER_SIZE,                 \
                       HST_TAGACC_LOCKCFG_ACTION_USER_SHIFT)
#define HST_TAGACC_LOCKCFG_SET_ACTION_TID(rv, va)                           \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_LOCKCFG_ACTION_TID_SIZE,                  \
                       HST_TAGACC_LOCKCFG_ACTION_TID_SHIFT)
#define HST_TAGACC_LOCKCFG_SET_ACTION_EPC(rv, va)                           \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_LOCKCFG_ACTION_EPC_SIZE,                  \
                       HST_TAGACC_LOCKCFG_ACTION_EPC_SHIFT)
#define HST_TAGACC_LOCKCFG_SET_ACTION_ACC(rv, va)                           \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_LOCKCFG_ACTION_ACC_SIZE,                  \
                       HST_TAGACC_LOCKCFG_ACTION_ACC_SHIFT)
#define HST_TAGACC_LOCKCFG_SET_ACTION_KILL(rv, va)                          \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_LOCKCFG_ACTION_KILL_SIZE,                 \
                       HST_TAGACC_LOCKCFG_ACTION_KILL_SHIFT)
#define HST_TAGACC_LOCKCFG_SET_MASK_USER(rv, va)                            \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_LOCKCFG_MASK_USER_SIZE,                   \
                       HST_TAGACC_LOCKCFG_MASK_USER_SHIFT)
#define HST_TAGACC_LOCKCFG_SET_MASK_TID(rv, va)                             \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_LOCKCFG_MASK_TID_SIZE,                    \
                       HST_TAGACC_LOCKCFG_MASK_TID_SHIFT)
#define HST_TAGACC_LOCKCFG_SET_MASK_EPC(rv, va)                             \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_LOCKCFG_MASK_EPC_SIZE,                    \
                       HST_TAGACC_LOCKCFG_MASK_EPC_SHIFT)
#define HST_TAGACC_LOCKCFG_SET_MASK_ACC(rv, va)                             \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_LOCKCFG_MASK_ACC_SIZE,                    \
                       HST_TAGACC_LOCKCFG_MASK_ACC_SHIFT)
#define HST_TAGACC_LOCKCFG_SET_MASK_KILL(rv, va)                            \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_LOCKCFG_MASK_KILL_SIZE,                   \
                       HST_TAGACC_LOCKCFG_MASK_KILL_SHIFT)
#define HST_TAGACC_LOCKCFG_SET_RFU1(rv, va)                                 \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGACC_LOCKCFG_RFU1_SIZE,                        \
                       HST_TAGACC_LOCKCFG_RFU1_SHIFT)

/* HST_TAGWRDAT_N register helper macros                                    */
/* The size of the bit fields in the HST_TAGWRDAT_N register.               */
#define HST_TAGWRDAT_N_DATA_SIZE    16
#define HST_TAGWRDAT_N_OFF_SIZE     16
#define HST_TAGWRDAT_N_DATA1_SIZE    16
#define HST_TAGWRDAT_N_DATA0_SIZE     16
/* The number of bits that fields are left-shifted in the HST_TAGWRDAT_N    */
/* register.                                                                */
#define HST_TAGWRDAT_N_DATA_SHIFT  0
#define HST_TAGWRDAT_N_OFF_SHIFT                                            \
    (HST_TAGWRDAT_N_DATA_SHIFT + HST_TAGWRDAT_N_DATA_SIZE)
#define HST_TAGWRDAT_N_DATA1_SHIFT  0
#define HST_TAGWRDAT_N_DATA0_SHIFT                                            \
    (HST_TAGWRDAT_N_DATA1_SHIFT + HST_TAGWRDAT_N_DATA1_SIZE)
/* HST_TAGWRDAT_N bit-field retrieval and testing macros                    */
/* rv - the value from the HST_TAGWRDAT_N register                          */
#define HST_TAGWRDAT_N_GET_DATA(rv)                                         \
    BIT_FIELD_EXTRACT(rv, HST_TAGWRDAT_N_DATA_SIZE, HST_TAGWRDAT_N_DATA_SHIFT)
#define HST_TAGWRDAT_N_GET_OFF(rv)                                          \
    BIT_FIELD_EXTRACT(rv, HST_TAGWRDAT_N_OFF_SIZE, HST_TAGWRDAT_N_OFF_SHIFT)
#define HST_TAGWRDAT_N_GET_DATA1(rv)                                         \
    BIT_FIELD_EXTRACT(rv, HST_TAGWRDAT_N_DATA1_SIZE, HST_TAGWRDAT_N_DATA1_SHIFT)
#define HST_TAGWRDAT_N_GET_DATA0(rv)                                          \
    BIT_FIELD_EXTRACT(rv, HST_TAGWRDAT_N_DATA0_SIZE, HST_TAGWRDAT_N_DATA0_SHIFT)
/* HST_TAGWRDAT_N bit-field value creation macros                           */
/* va - the value to put into the bit field                                 */
#define HST_TAGWRDAT_N_DATA(va)                                             \
    BIT_FIELD_CREATE(va, HST_TAGWRDAT_N_DATA_SIZE, HST_TAGWRDAT_N_DATA_SHIFT)
#define HST_TAGWRDAT_N_OFF(va)                                              \
    BIT_FIELD_CREATE(va, HST_TAGWRDAT_N_OFF_SIZE, HST_TAGWRDAT_N_OFF_SHIFT)
#define HST_TAGWRDAT_N_DATA1(va)                                             \
    BIT_FIELD_CREATE(va, HST_TAGWRDAT_N_DATA1_SIZE, HST_TAGWRDAT_N_DATA1_SHIFT)
#define HST_TAGWRDAT_N_DATA0(va)                                              \
    BIT_FIELD_CREATE(va, HST_TAGWRDAT_N_DATA0_SIZE, HST_TAGWRDAT_N_DATA0_SHIFT)
/* HST_TAGWRDAT_N register value modification macros (i.e., will modify the */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_TAGWRDAT_N_SET_DATA(rv, va)                                     \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGWRDAT_N_DATA_SIZE,                            \
                       HST_TAGWRDAT_N_DATA_SHIFT)
#define HST_TAGWRDAT_N_SET_OFF(rv, va)                                      \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGWRDAT_N_OFF_SIZE,                             \
                       HST_TAGWRDAT_N_OFF_SHIFT)
#define HST_TAGWRDAT_N_SET_DATA1(rv, va)                                     \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGWRDAT_N_DATA1_SIZE,                            \
                       HST_TAGWRDAT_N_DATA1_SHIFT)
#define HST_TAGWRDAT_N_SET_DATA0(rv, va)                                      \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGWRDAT_N_DATA0_SIZE,                             \
                       HST_TAGWRDAT_N_DATA0_SHIFT)

/* HST_TAGWRDAT_SEL register helper macros                                   */
/* The size of the bit fields in the HST_TAGWRDAT_SEL register.              */
#define HST_TAGWRDAT_SEL_SELECTOR_SIZE   3
#define HST_TAGWRDAT_SEL_RFU1_SIZE   29
/* The number of bits that fields are left-shifted in the HST_TAGWRDAT_SEL   */
/* register.                                                                 */
#define HST_TAGWRDAT_SEL_SELECTOR_SHIFT  0
#define HST_TAGWRDAT_SEL_RFU1_SHIFT                                           \
    (HST_TAGWRDAT_SEL_SELECTOR_SHIFT + HST_TAGWRDAT_SEL_SELECTOR_SIZE)
/* HST_TAGWRDAT_SEL bit-field retrieval and testing macros                   */
/* rv - the value from the HST_TAGWRDAT_SEL register                         */
#define HST_TAGWRDAT_SEL_GET_SELECTOR(rv)                                        \
    BIT_FIELD_EXTRACT(rv, HST_TAGWRDAT_SEL_SELECTOR_SIZE, HST_TAGWRDAT_SEL_SELECTOR_SHIFT)
#define HST_TAGWRDAT_SEL_GET_RFU1(rv)                                        \
    BIT_FIELD_EXTRACT(rv, HST_TAGWRDAT_SEL_RFU1_SIZE, HST_TAGWRDAT_SEL_RFU1_SHIFT)
/* HST_TAGWRDAT_SEL bit-field value creation macros                          */
/* va - the value to put into the bit field                                 */
#define HST_TAGWRDAT_SEL_SELECTOR(va)                                            \
    BIT_FIELD_CREATE(va, HST_TAGWRDAT_SEL_SELECTOR_SIZE, HST_TAGWRDAT_SEL_SELECTOR_SHIFT)
#define HST_TAGWRDAT_SEL_RFU1(va)                                            \
    BIT_FIELD_CREATE(va, HST_TAGWRDAT_SEL_RFU1_SIZE, HST_TAGWRDAT_SEL_RFU1_SHIFT)
/* HST_TAGWRDAT_SEL register value modification macros (i.e., will modify the*/
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_TAGWRDAT_SEL_SET_SELECTOR(rv, va)                                    \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGWRDAT_SEL_SELECTOR_SIZE,                           \
                       HST_TAGWRDAT_SEL_SELECTOR_SHIFT)
#define HST_TAGWRDAT_SEL_SET_RFU1(rv, va)                                    \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_TAGWRDAT_SEL_RFU1_SIZE,                           \
                       HST_TAGWRDAT_SEL_RFU1_SHIFT)

/* HST_RFTC_FRQHOPMODE register helper macros                               */
/* Constants for HST_RFTC_FRQHOPMODE register bit fields (note that the     */
/* values are already shifted into the low-order bits of the constant.      */
#define HST_RFTC_FRQHOPMODE_DISABLED    0x0
#define HST_RFTC_FRQHOPMODE_SWEEP       0x1
#define HST_RFTC_FRQHOPMODE_SEQ         0x2
#define HST_RFTC_FRQHOPMODE_PRAND       0x3

/* HST_RFTC_PROF_CURRENTPROF register helper macros                         */
/* The size of the bit fields in the HST_RFTC_PROF_CURRENTPROF register.    */
#define HST_RFTC_PROF_CURRENTPROF_PROF_SIZE     8
#define HST_RFTC_PROF_CURRENTPROF_DRMOFF_SIZE   1
#define HST_RFTC_PROF_CURRENTPROF_RFU1_SIZE     23
/* The number of bits that fields are left-shifted in the                   */
/* HST_RFTC_PROF_CURRENTPROF register.                                      */
#define HST_RFTC_PROF_CURRENTPROF_PROF_SHIFT    0
#define HST_RFTC_PROF_CURRENTPROF_DRMOFF_SHIFT                              \
    (HST_RFTC_PROF_CURRENTPROF_PROF_SHIFT + HST_RFTC_PROF_CURRENTPROF_PROF_SIZE)
#define HST_RFTC_PROF_CURRENTPROF_RFU1_SHIFT                                \
    (HST_RFTC_PROF_CURRENTPROF_DRMOFF_SHIFT +                               \
     HST_RFTC_PROF_CURRENTPROF_DRMOFF_SIZE)
/* HST_RFTC_PROF_CURRENTPROF bit-field retrieval and testing macros         */
/* rv - the value from the HST_RFTC_PROF_CURRENTPROF register               */
#define HST_RFTC_PROF_CURRENTPROF_GET_PROF(rv)                              \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_PROF_CURRENTPROF_PROF_SIZE,                  \
                      HST_RFTC_PROF_CURRENTPROF_PROF_SHIFT)
#define HST_RFTC_PROF_CURRENTPROF_DRMOFF_IS_ENABLED(rv)                     \
    REGISTER_TEST_BIT(rv, HST_RFTC_PROF_CURRENTPROF_DRMOFF_SHIFT)
#define HST_RFTC_PROF_CURRENTPROF_DRMOFF_IS_DISABLED(rv)                    \
    !(REGISTER_TEST_BIT(rv, HST_RFTC_PROF_CURRENTPROF_DRMOFF_SHIFT))
#define HST_RFTC_PROF_CURRENTPROF_GET_RFU1(rv)                              \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_PROF_CURRENTPROF_RFU1_SIZE,                  \
                      HST_RFTC_PROF_CURRENTPROF_RFU1_SHIFT)
/* HST_RFTC_PROF_CURRENTPROF bit-field value creation macros                */
/* va - the value to put into the bit field                                 */
#define HST_RFTC_PROF_CURRENTPROF_PROF(va)                                  \
    BIT_FIELD_CREATE(va,                                                    \
                      HST_RFTC_PROF_CURRENTPROF_PROF_SIZE,                  \
                      HST_RFTC_PROF_CURRENTPROF_PROF_SHIFT)
#define HST_RFTC_PROF_CURRENTPROF_DRMOFF_ENABLED                            \
    CREATE_1BIT_MASK(HST_RFTC_PROF_CURRENTPROF_DRMOFF_SHIFT)
#define HST_RFTC_PROF_CURRENTPROF_DRMOFF_DISABLED   0
#define HST_RFTC_PROF_CURRENTPROF_RFU1(va)                                  \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_PROF_CURRENTPROF_RFU1_SIZE,                   \
                     HST_RFTC_PROF_CURRENTPROF_RFU1_SHIFT)
/* HST_RFTC_PROF_CURRENTPROF register value modification macros (i.e., will */
/* modify the field specified within an already existing register value).   */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_RFTC_PROF_CURRENTPROF_SET_PROF(rv, va)                          \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_PROF_CURRENTPROF_PROF_SIZE,                 \
                       HST_RFTC_PROF_CURRENTPROF_PROF_SHIFT)
#define HST_RFTC_PROF_CURRENTPROF_SET_DRMOFF_ENABLED(rv)                    \
    REGISTER_SET_BIT(rv, HST_RFTC_PROF_CURRENTPROF_DRMOFF_SHIFT)
#define HST_RFTC_PROF_CURRENTPROF_SET_DRMOFF_DISABLED(rv)                   \
    REGISTER_CLR_BIT(rv, HST_RFTC_PROF_CURRENTPROF_DRMOFF_SHIFT)
#define HST_RFTC_PROF_CURRENTPROF_SET_RFU1(rv, va)                          \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_PROF_CURRENTPROF_RFU1_SIZE,                 \
                       HST_RFTC_PROF_CURRENTPROF_RFU1_SHIFT)

/* HST_RFTC_PROF_SEL register helper macros                                 */
/* The size of the bit fields in the HST_RFTC_PROF_SEL register.            */
#define HST_RFTC_PROF_SEL_PROF_SIZE     8
#define HST_RFTC_PROF_SEL_RFU1_SIZE     24
/* The number of bits that fields are left-shifted in the                   */
/* HST_RFTC_PROF_SEL register.                                              */
#define HST_RFTC_PROF_SEL_PROF_SHIFT    0
#define HST_RFTC_PROF_SEL_RFU1_SHIFT                                        \
    (HST_RFTC_PROF_SEL_PROF_SHIFT + HST_RFTC_PROF_SEL_PROF_SIZE)
/* HST_RFTC_PROF_SEL bit-field retrieval and testing macros                 */
/* rv - the value from the HST_RFTC_PROF_SEL register                       */
#define HST_RFTC_PROF_SEL_GET_PROF(rv)                                      \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_PROF_SEL_PROF_SIZE,                          \
                      HST_RFTC_PROF_SEL_PROF_SHIFT)
#define HST_RFTC_PROF_SEL_GET_RFU1(rv)                                      \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_PROF_SEL_RFU1_SIZE,                          \
                      HST_RFTC_PROF_SEL_RFU1_SHIFT)
/* HST_RFTC_PROF_SEL bit-field value creation macros                        */
/* va - the value to put into the bit field                                 */
#define HST_RFTC_PROF_SEL_PROF(va)                                          \
    BIT_FIELD_CREATE(va,                                                    \
                      HST_RFTC_PROF_SEL_PROF_SIZE,                          \
                      HST_RFTC_PROF_SEL_PROF_SHIFT)
#define HST_RFTC_PROF_SEL_RFU1(va)                                          \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_PROF_SEL_RFU1_SIZE,                           \
                     HST_RFTC_PROF_SEL_RFU1_SHIFT)
/* HST_RFTC_PROF_SEL register value modification macros (i.e., will         */
/* modify the field specified within an already existing register value).   */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_RFTC_PROF_SEL_SET_PROF(rv, va)                                  \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_PROF_SEL_PROF_SIZE,                         \
                       HST_RFTC_PROF_SEL_PROF_SHIFT)
#define HST_RFTC_PROF_SEL_SET_RFU1(rv, va)                                  \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_PROF_SEL_RFU1_SIZE,                         \
                       HST_RFTC_PROF_SEL_RFU1_SHIFT)

/* MAC_RFTC_PROF_CFG register helper macros                                 */
/* The size of the bit fields in the MAC_RFTC_PROF_CFG register.            */
#define MAC_RFTC_PROF_CFG_STATUS_SIZE   1
#define MAC_RFTC_PROF_CFG_DRM_SIZE      1
#define MAC_RFTC_PROF_CFG_DIRTY_SIZE    1
#define MAC_RFTC_PROF_CFG_RFU1_SIZE     29
/* The number of bits that fields are left-shifted in the MAC_RFTC_PROF_CFG */
/* register.                                                                */
#define MAC_RFTC_PROF_CFG_STATUS_SHIFT  0
#define MAC_RFTC_PROF_CFG_DRM_SHIFT                                         \
    (MAC_RFTC_PROF_CFG_STATUS_SHIFT + MAC_RFTC_PROF_CFG_STATUS_SIZE)
#define MAC_RFTC_PROF_CFG_DIRTY_SHIFT                                       \
    (MAC_RFTC_PROF_CFG_DRM_SHIFT + MAC_RFTC_PROF_CFG_DRM_SIZE)
#define MAC_RFTC_PROF_CFG_RFU1_SHIFT                                        \
    (MAC_RFTC_PROF_CFG_DIRTY_SHIFT + MAC_RFTC_PROF_CFG_DIRTY_SIZE)
/* MAC_RFTC_PROF_CFG bit-field retrieval and testing macros                 */
/* rv - the value from the MAC_RFTC_PROF_CFG register                       */
#define MAC_RFTC_PROF_CFG_IS_ENABLED(rv)                                    \
    REGISTER_TEST_BIT(rv, MAC_RFTC_PROF_CFG_STATUS_SHIFT)
#define MAC_RFTC_PROF_CFG_IS_DISABLED(rv)                                   \
    !(REGISTER_TEST_BIT(rv, MAC_RFTC_PROF_CFG_STATUS_SHIFT))
#define MAC_RFTC_PROF_CFG_DRM_IS_ENABLED(rv)                                \
    REGISTER_TEST_BIT(rv, MAC_RFTC_PROF_CFG_DRM_SHIFT)
#define MAC_RFTC_PROF_CFG_DRM_IS_DISABLED(rv)                               \
    !(REGISTER_TEST_BIT(rv, MAC_RFTC_PROF_CFG_DRM_SHIFT))
#define MAC_RFTC_PROF_CFG_IS_DIRTY(rv)                                      \
    REGISTER_TEST_BIT(rv, MAC_RFTC_PROF_CFG_DIRTY_SHIFT)
#define MAC_RFTC_PROF_CFG_IS_NOTDIRTY(rv)                                   \
    !(REGISTER_TEST_BIT(rv, MAC_RFTC_PROF_CFG_DIRTY_SHIFT))
#define MAC_RFTC_PROF_CFG_GET_RFU1(rv)                                      \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      MAC_RFTC_PROF_CFG_RFU1_SIZE,                          \
                      MAC_RFTC_PROF_CFG_RFU1_SHIFT)
/* MAC_RFTC_PROF_CFG bit-field value creation macros                        */
/* va - the value to put into the bit field                                 */
#define MAC_RFTC_PROF_CFG_ENABLED                                           \
    CREATE_1BIT_MASK(MAC_RFTC_PROF_CFG_STATUS_SHIFT)
#define MAC_RFTC_PROF_CFG_DISABLED      0
#define MAC_RFTC_PROF_CFG_DRM_ENABLED                                       \
    CREATE_1BIT_MASK(MAC_RFTC_PROF_CFG_DRM_SHIFT)
#define MAC_RFTC_PROF_CFG_DRM_DISABLED  0
#define MAC_RFTC_PROF_CFG_DIRTY                                             \
    CREATE_1BIT_MASK(MAC_RFTC_PROF_CFG_DIRTY_SHIFT)
#define MAC_RFTC_PROF_CFG_NOTDIRTY      0
#define MAC_RFTC_PROF_CFG_RFU1(va)                                          \
    BIT_FIELD_CREATE(va,                                                    \
                     MAC_RFTC_PROF_CFG_RFU1_SIZE,                           \
                     MAC_RFTC_PROF_CFG_RFU1_SHIFT)
/* MAC_RFTC_PROF_CFG register value modification macros (i.e., will modify  */
/* the field specified within an already existing register value).          */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define MAC_RFTC_PROF_CFG_SET_ENABLED(rv)                                   \
    REGISTER_SET_BIT(rv, MAC_RFTC_PROF_CFG_STATUS_SHIFT)
#define MAC_RFTC_PROF_CFG_SET_DISABLED(rv)                                  \
    REGISTER_CLR_BIT(rv, MAC_RFTC_PROF_CFG_STATUS_SHIFT)
#define MAC_RFTC_PROF_CFG_SET_DRM_ENABLED(rv)                               \
    REGISTER_SET_BIT(rv, MAC_RFTC_PROF_CFG_DRM_SHIFT)
#define MAC_RFTC_PROF_CFG_SET_DRM_DISABLED(rv)                              \
    REGISTER_CLR_BIT(rv, MAC_RFTC_PROF_CFG_DRM_SHIFT)
#define MAC_RFTC_PROF_CFG_SET_DIRTY(rv)                                     \
    REGISTER_SET_BIT(rv, MAC_RFTC_PROF_CFG_DIRTY_SHIFT)
#define MAC_RFTC_PROF_CFG_SET_NOTDIRTY(rv)                                  \
    REGISTER_CLR_BIT(rv, MAC_RFTC_PROF_CFG_DIRTY_SHIFT)
#define MAC_RFTC_PROF_CFG_SET_RFU1(rv, va)                                  \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       MAC_RFTC_PROF_CFG_RFU1_SIZE,                         \
                       MAC_RFTC_PROF_CFG_RFU1_SHIFT)

/* MAC_RFTC_PROF_RSSIAVECFG register helper macros                          */
/* The size of the bit fields in the MAC_RFTC_PROF_RSSIAVECFG               */
/* register.                                                                */
#define MAC_RFTC_PROF_RSSIAVECFG_NORM_WBSAMPS_SIZE      3
#define MAC_RFTC_PROF_RSSIAVECFG_NORM_NBSAMPS_SIZE      3
#define MAC_RFTC_PROF_RSSIAVECFG_RT_EN_SIZE             1
#define MAC_RFTC_PROF_RSSIAVECFG_RT_WBSAMPS_SIZE        3
#define MAC_RFTC_PROF_RSSIAVECFG_RT_NBSAMPS_SIZE        3
#define MAC_RFTC_PROF_RSSIAVECFG_RFU1_SIZE              19
/* The number of bits that fields are left-shifted in the                   */
/* MAC_RFTC_PROF_RSSIAVECFG register.                                       */
#define MAC_RFTC_PROF_RSSIAVECFG_NORM_WBSAMPS_SHIFT     0
#define MAC_RFTC_PROF_RSSIAVECFG_NORM_NBSAMPS_SHIFT                         \
    (MAC_RFTC_PROF_RSSIAVECFG_NORM_WBSAMPS_SHIFT +                          \
     MAC_RFTC_PROF_RSSIAVECFG_NORM_WBSAMPS_SIZE)
#define MAC_RFTC_PROF_RSSIAVECFG_RT_EN_SHIFT                                \
    (MAC_RFTC_PROF_RSSIAVECFG_NORM_NBSAMPS_SHIFT +                          \
     MAC_RFTC_PROF_RSSIAVECFG_NORM_NBSAMPS_SIZE)
#define MAC_RFTC_PROF_RSSIAVECFG_RT_WBSAMPS_SHIFT                           \
    (MAC_RFTC_PROF_RSSIAVECFG_RT_EN_SHIFT +                                 \
     MAC_RFTC_PROF_RSSIAVECFG_RT_EN_SIZE)
#define MAC_RFTC_PROF_RSSIAVECFG_RT_NBSAMPS_SHIFT                           \
    (MAC_RFTC_PROF_RSSIAVECFG_RT_WBSAMPS_SHIFT +                            \
     MAC_RFTC_PROF_RSSIAVECFG_RT_WBSAMPS_SIZE)
#define MAC_RFTC_PROF_RSSIAVECFG_RFU1_SHIFT                                 \
    (MAC_RFTC_PROF_RSSIAVECFG_RT_NBSAMPS_SHIFT +                            \
     MAC_RFTC_PROF_RSSIAVECFG_RT_NBSAMPS_SIZE)
/* MAC_RFTC_PROF_RSSIAVECFG bit-field retrieval and testing macros    */
/* rv - the value from the MAC_RFTC_PROF_RSSIAVECFG register          */
#define MAC_RFTC_PROF_RSSIAVECFG_GET_NORM_WBSAMPS(rv)                       \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      MAC_RFTC_PROF_RSSIAVECFG_NORM_WBSAMPS_SIZE,           \
                      MAC_RFTC_PROF_RSSIAVECFG_NORM_WBSAMPS_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_GET_NORM_NBSAMPS(rv)                       \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      MAC_RFTC_PROF_RSSIAVECFG_NORM_NBSAMPS_SIZE,           \
                      MAC_RFTC_PROF_RSSIAVECFG_NORM_NBSAMPS_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_RT_IS_ENABLED(rv)                          \
    REGISTER_TEST_BIT(rv, MAC_RFTC_PROF_RSSIAVECFG_RT_EN_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_RT_IS_DISABLED(rv)                         \
    !(REGISTER_TEST_BIT(rv, MAC_RFTC_PROF_RSSIAVECFG_RT_EN_SHIFT))
#define MAC_RFTC_PROF_RSSIAVECFG_GET_RT_WBSAMPS(rv)                         \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      MAC_RFTC_PROF_RSSIAVECFG_RT_WBSAMPS_SIZE,             \
                      MAC_RFTC_PROF_RSSIAVECFG_RT_WBSAMPS_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_GET_RT_NBSAMPS(rv)                         \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      MAC_RFTC_PROF_RSSIAVECFG_RT_NBSAMPS_SIZE,             \
                      MAC_RFTC_PROF_RSSIAVECFG_RT_NBSAMPS_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_GET_RFU1(rv)                               \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      MAC_RFTC_PROF_RSSIAVECFG_RFU1_SIZE,                   \
                      MAC_RFTC_PROF_RSSIAVECFG_RFU1_SHIFT)
/* MAC_RFTC_PROF_RSSIAVECFG bit-field value creation macros                 */
/* va - the value to put into the bit field                                 */
#define MAC_RFTC_PROF_RSSIAVECFG_NORM_WBSAMPS(va)                           \
    BIT_FIELD_CREATE(va,                                                    \
                     MAC_RFTC_PROF_RSSIAVECFG_NORM_WBSAMPS_SIZE,            \
                     MAC_RFTC_PROF_RSSIAVECFG_NORM_WBSAMPS_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_NORM_NBSAMPS(va)                           \
    BIT_FIELD_CREATE(va,                                                    \
                     MAC_RFTC_PROF_RSSIAVECFG_NORM_NBSAMPS_SIZE,            \
                     MAC_RFTC_PROF_RSSIAVECFG_NORM_NBSAMPS_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_RT_ENABLED                                 \
    CREATE_1BIT_MASK(MAC_RFTC_PROF_RSSIAVECFG_RT_EN_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_RT_DISABLED    0
#define MAC_RFTC_PROF_RSSIAVECFG_RT_WBSAMPS(va)                             \
    BIT_FIELD_CREATE(va,                                                    \
                     MAC_RFTC_PROF_RSSIAVECFG_RT_WBSAMPS_SIZE,              \
                     MAC_RFTC_PROF_RSSIAVECFG_RT_WBSAMPS_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_RT_NBSAMPS(va)                             \
    BIT_FIELD_CREATE(va,                                                    \
                     MAC_RFTC_PROF_RSSIAVECFG_RT_NBSAMPS_SIZE,              \
                     MAC_RFTC_PROF_RSSIAVECFG_RT_NBSAMPS_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_RFU1(va)                                   \
    BIT_FIELD_CREATE(va,                                                    \
                     MAC_RFTC_PROF_RSSIAVECFG_RFU1_SIZE,                    \
                     MAC_RFTC_PROF_RSSIAVECFG_RFU1_SHIFT)
/* MAC_RFTC_PROF_RSSIAVECFG register value modification macros (i.e.,       */
/* will modify the field specified within an already existing register      */
/* value).                                                                  */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define MAC_RFTC_PROF_RSSIAVECFG_SET_NORM_WBSAMPS(rv, va)                   \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       MAC_RFTC_PROF_RSSIAVECFG_NORM_WBSAMPS_SIZE,          \
                       MAC_RFTC_PROF_RSSIAVECFG_NORM_WBSAMPS_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_SET_NORM_NBSAMPS(rv, va)                   \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       MAC_RFTC_PROF_RSSIAVECFG_NORM_NBSAMPS_SIZE,          \
                       MAC_RFTC_PROF_RSSIAVECFG_NORM_NBSAMPS_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_SET_ENABLED(rv)                            \
    REGISTER_SET_BIT(rv, MAC_RFTC_PROF_RSSIAVECFG_RT_EN_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_SET_DISABLED(rv)                           \
    REGISTER_CLR_BIT(rv, MAC_RFTC_PROF_RSSIAVECFG_RT_EN_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_SET_RT_WBSAMPS(rv, va)                     \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       MAC_RFTC_PROF_RSSIAVECFG_RT_WBSAMPS_SIZE,            \
                       MAC_RFTC_PROF_RSSIAVECFG_RT_WBSAMPS_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_SET_RT_NBSAMPS(rv, va)                     \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       MAC_RFTC_PROF_RSSIAVECFG_RT_NBSAMPS_SIZE,            \
                       MAC_RFTC_PROF_RSSIAVECFG_RT_NBSAMPS_SHIFT)
#define MAC_RFTC_PROF_RSSIAVECFG_SET_RFU1(rv, va)                           \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       MAC_RFTC_PROF_RSSIAVECFG_RFU1_SIZE,                  \
                       MAC_RFTC_PROF_RSSIAVECFG_RFU1_SHIFT)

/* HST_PROT_DESC_R2TMODTYPE register helper macros                          */
/* Constants for HST_PROT_DESC_R2TMODTYPE register bit fields (note that the*/
/* values are already shifted into the low-order bits of the constant.      */
#define HST_PROT_DESC_R2TMODTYPE_DSB_ASK    0x0
#define HST_PROT_DESC_R2TMODTYPE_SSB_ASK    0x1
#define HST_PROT_DESC_R2TMODTYPE_PR_ASK     0x2

/* HST_RFTC_FRQCH_CFG register helper macros                                */
/* The size of the bit fields in the HST_RFTC_FRQCH_CFG register.           */
#define HST_RFTC_FRQCH_CFG_STATUS_SIZE  1
#define HST_RFTC_FRQCH_CFG_TUNED_SIZE   1
#define HST_RFTC_FRQCH_CFG_RFU1_SIZE    30
/* The number of bits that fields are left-shifted in the HST_RFTC_FRQCH_CFG*/
/* register.                                                                */
#define HST_RFTC_FRQCH_CFG_STATUS_SHIFT 0
#define HST_RFTC_FRQCH_CFG_TUNED_SHIFT                                      \
    (HST_RFTC_FRQCH_CFG_STATUS_SHIFT + HST_RFTC_FRQCH_CFG_STATUS_SIZE)
#define HST_RFTC_FRQCH_CFG_RFU1_SHIFT                                       \
    (HST_RFTC_FRQCH_CFG_TUNED_SHIFT + HST_RFTC_FRQCH_CFG_TUNED_SIZE)
/* HST_RFTC_FRQCH_CFG bit-field retrieval and testing macros                */
/* rv - the value from the HST_RFTC_FRQCH_CFG register                      */
#define HST_RFTC_FRQCH_CFG_IS_ENABLED(rv)                                   \
    REGISTER_TEST_BIT(rv, HST_RFTC_FRQCH_CFG_STATUS_SHIFT)
#define HST_RFTC_FRQCH_CFG_IS_DISABLED(rv)                                  \
    !(REGISTER_TEST_BIT(rv, HST_RFTC_FRQCH_CFG_STATUS_SHIFT))
#define HST_RFTC_FRQCH_CFG_IS_TUNED(rv)                                     \
    REGISTER_TEST_BIT(rv, HST_RFTC_FRQCH_CFG_TUNED_SHIFT)
#define HST_RFTC_FRQCH_CFG_IS_NOT_TUNED(rv)                                 \
    !(REGISTER_TEST_BIT(rv, HST_RFTC_FRQCH_CFG_TUNED_SHIFT))
#define HST_RFTC_FRQCH_CFG_GET_RFU1(rv)                                     \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_FRQCH_CFG_RFU1_SIZE,                         \
                      HST_RFTC_FRQCH_CFG_RFU1_SHIFT)
/* HST_RFTC_FRQCH_CFG bit-field value creation macros                       */
/* va - the value to put into the bit field                                 */
#define HST_RFTC_FRQCH_CFG_ENABLED                                          \
    CREATE_1BIT_MASK(HST_RFTC_FRQCH_CFG_STATUS_SHIFT)
#define HST_RFTC_FRQCH_CFG_DISABLED     0
#define HST_RFTC_FRQCH_CFG_TUNED                                            \
    CREATE_1BIT_MASK(HST_RFTC_FRQCH_CFG_TUNED_SHIFT)
#define HST_RFTC_FRQCH_CFG_NOT_TUNED    0
#define HST_RFTC_FRQCH_CFG_RFU1(va)                                         \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_FRQCH_CFG_RFU1_SIZE,                          \
                     HST_RFTC_FRQCH_CFG_RFU1_SHIFT)
/* HST_RFTC_FRQCH_CFG register value modification macros (i.e., will modify */
/* the field specified within an already existing register value).          */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_RFTC_FRQCH_CFG_SET_ENABLED(rv)                                  \
    REGISTER_SET_BIT(rv, HST_RFTC_FRQCH_CFG_STATUS_SHIFT)
#define HST_RFTC_FRQCH_CFG_SET_DISABLED(rv)                                 \
    REGISTER_CLR_BIT(rv, HST_RFTC_FRQCH_CFG_STATUS_SHIFT)
#define HST_RFTC_FRQCH_CFG_SET_TUNED(rv)                                    \
    REGISTER_SET_BIT(rv, HST_RFTC_FRQCH_CFG_TUNED_SHIFT)
#define HST_RFTC_FRQCH_CFG_SET_NOT_TUNED(rv)                                \
    REGISTER_CLR_BIT(rv, HST_RFTC_FRQCH_CFG_TUNED_SHIFT)
#define HST_RFTC_FRQCH_CFG_SET_RFU1(rv, va)                                 \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_FRQCH_CFG_RFU1_SIZE,                        \
                       HST_RFTC_FRQCH_CFG_RFU1_SHIFT)

/* HST_RFTC_FRQCH_DESC_PLLDIVMULT register helper macros                    */
/* The size of the bit fields in the HST_RFTC_FRQCH_DESC_PLLDIVMULT         */
/* register.                                                                */
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_MULTRAT_SIZE     16
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_DIVRAT_SIZE      9
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_RFU1_SIZE        7
/* The number of bits that fields are left-shifted in the                   */
/* HST_RFTC_FRQCH_DESC_PLLDIVMULT register.                                 */
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_MULTRAT_SHIFT    0
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_DIVRAT_SHIFT                         \
    (HST_RFTC_FRQCH_DESC_PLLDIVMULT_MULTRAT_SHIFT +                         \
     HST_RFTC_FRQCH_DESC_PLLDIVMULT_MULTRAT_SIZE)
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_RFU1_SHIFT                           \
    (HST_RFTC_FRQCH_DESC_PLLDIVMULT_DIVRAT_SHIFT +                          \
     HST_RFTC_FRQCH_DESC_PLLDIVMULT_DIVRAT_SIZE)
/* HST_RFTC_FRQCH_DESC_PLLDIVMULT bit-field retrieval and testing macros    */
/* rv - the value from the HST_RFTC_FRQCH_DESC_PLLDIVMULT register          */
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_GET_MULTRAT(rv)                      \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_FRQCH_DESC_PLLDIVMULT_MULTRAT_SIZE,          \
                      HST_RFTC_FRQCH_DESC_PLLDIVMULT_MULTRAT_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_GET_DIVRAT(rv)                       \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_FRQCH_DESC_PLLDIVMULT_DIVRAT_SIZE,           \
                      HST_RFTC_FRQCH_DESC_PLLDIVMULT_DIVRAT_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_GET_RFU1(rv)                         \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_FRQCH_DESC_PLLDIVMULT_RFU1_SIZE,             \
                      HST_RFTC_FRQCH_DESC_PLLDIVMULT_RFU1_SHIFT)
/* HST_RFTC_FRQCH_DESC_PLLDIVMULT bit-field value creation macros           */
/* va - the value to put into the bit field                                 */
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_MULTRAT(va)                          \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_FRQCH_DESC_PLLDIVMULT_MULTRAT_SIZE,           \
                     HST_RFTC_FRQCH_DESC_PLLDIVMULT_MULTRAT_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_DIVRAT(va)                           \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_FRQCH_DESC_PLLDIVMULT_DIVRAT_SIZE,            \
                     HST_RFTC_FRQCH_DESC_PLLDIVMULT_DIVRAT_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_RFU1(va)                             \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_FRQCH_DESC_PLLDIVMULT_RFU1_SIZE,              \
                     HST_RFTC_FRQCH_DESC_PLLDIVMULT_RFU1_SHIFT)
/* HST_RFTC_FRQCH_DESC_PLLDIVMULT register value modification macros (i.e., */
/* will modify the field specified within an already existing register      */
/* value).                                                                  */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_SET_MULTRAT(rv, va)                  \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_FRQCH_DESC_PLLDIVMULT_MULTRAT_SIZE,         \
                       HST_RFTC_FRQCH_DESC_PLLDIVMULT_MULTRAT_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_SET_DIVRAT(rv, va)                   \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_FRQCH_DESC_PLLDIVMULT_DIVRAT_SIZE,          \
                       HST_RFTC_FRQCH_DESC_PLLDIVMULT_DIVRAT_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDIVMULT_SET_RFU1(rv, va)                     \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_FRQCH_DESC_PLLDIVMULT_RFU1_SIZE,            \
                       HST_RFTC_FRQCH_DESC_PLLDIVMULT_RFU1_SHIFT)

/* HST_RFTC_FRQCH_DESC_PLLDACCTL register helper macros                     */
/* The size of the bit fields in the HST_RFTC_FRQCH_DESC_PLLDACCTL          */
/* register.                                                                */
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_MINBAND_SIZE      8
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_PREFERBAND_SIZE   8
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_MAXBAND_SIZE      8
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_GUARDBAND_SIZE    8
/* The number of bits that fields are left-shifted in the                   */
/* HST_RFTC_FRQCH_DESC_PLLDACCTL register.                                  */
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_MINBAND_SHIFT 0
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_PREFERBAND_SHIFT                      \
    (HST_RFTC_FRQCH_DESC_PLLDACCTL_MINBAND_SHIFT +                          \
     HST_RFTC_FRQCH_DESC_PLLDACCTL_MINBAND_SIZE)
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_MAXBAND_SHIFT                         \
    (HST_RFTC_FRQCH_DESC_PLLDACCTL_PREFERBAND_SHIFT +                       \
     HST_RFTC_FRQCH_DESC_PLLDACCTL_PREFERBAND_SIZE)
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_GUARDBAND_SHIFT                       \
    (HST_RFTC_FRQCH_DESC_PLLDACCTL_MAXBAND_SHIFT +                          \
     HST_RFTC_FRQCH_DESC_PLLDACCTL_MAXBAND_SIZE)
/* HST_RFTC_FRQCH_DESC_PLLDACCTL bit-field retrieval and testing macros     */
/* rv - the value from the HST_RFTC_FRQCH_DESC_PLLDACCTL register           */
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_GET_MINBAND(rv)                       \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_FRQCH_DESC_PLLDACCTL_MINBAND_SIZE,           \
                      HST_RFTC_FRQCH_DESC_PLLDACCTL_MINBAND_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_GET_PREFERBAND(rv)                    \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_FRQCH_DESC_PLLDACCTL_PREFERBAND_SIZE,        \
                      HST_RFTC_FRQCH_DESC_PLLDACCTL_PREFERBAND_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_GET_MAXBAND(rv)                       \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_FRQCH_DESC_PLLDACCTL_MAXBAND_SIZE,           \
                      HST_RFTC_FRQCH_DESC_PLLDACCTL_MAXBAND_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_GET_GUARDBAND(rv)                     \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_FRQCH_DESC_PLLDACCTL_GUARDBAND_SIZE,         \
                      HST_RFTC_FRQCH_DESC_PLLDACCTL_GUARDBAND_SHIFT)
/* HST_RFTC_FRQCH_DESC_PLLDACCTL bit-field value creation macros            */
/* va - the value to put into the bit field                                 */
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_MINBAND(va)                           \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_FRQCH_DESC_PLLDACCTL_MINBAND_SIZE,            \
                     HST_RFTC_FRQCH_DESC_PLLDACCTL_MINBAND_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_PREFERBAND(va)                        \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_FRQCH_DESC_PLLDACCTL_PREFERBAND_SIZE,         \
                     HST_RFTC_FRQCH_DESC_PLLDACCTL_PREFERBAND_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_MAXBAND(va)                           \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_FRQCH_DESC_PLLDACCTL_MAXBAND_SIZE,            \
                     HST_RFTC_FRQCH_DESC_PLLDACCTL_MAXBAND_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_GUARDBAND(va)                         \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_FRQCH_DESC_PLLDACCTL_GUARDBAND_SIZE,          \
                     HST_RFTC_FRQCH_DESC_PLLDACCTL_GUARDBAND_SHIFT)
/* HST_RFTC_FRQCH_DESC_PLLDACCTL register value modification macros (i.e.,  */
/* will modify the field specified within an already existing register      */
/* value).                                                                  */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_SET_MINBAND(rv, va)                   \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_FRQCH_DESC_PLLDACCTL_MINBAND_SIZE,          \
                       HST_RFTC_FRQCH_DESC_PLLDACCTL_MINBAND_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_SET_PREFERBAND(rv, va)                \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_FRQCH_DESC_PLLDACCTL_PREFERBAND_SIZE,       \
                       HST_RFTC_FRQCH_DESC_PLLDACCTL_PREFERBAND_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_SET_MAXBAND(rv, va)                   \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_FRQCH_DESC_PLLDACCTL_MAXBAND_SIZE,          \
                       HST_RFTC_FRQCH_DESC_PLLDACCTL_MAXBAND_SHIFT)
#define HST_RFTC_FRQCH_DESC_PLLDACCTL_SET_GUARDBAND(rv, va)                 \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_FRQCH_DESC_PLLDACCTL_GUARDBAND_SIZE,        \
                       HST_RFTC_FRQCH_DESC_PLLDACCTL_GUARDBAND_SHIFT)

/* HST_RFTC_WS_SEL register helper macros                                   */
/* Constants for HST_RFTC_WS_SEL register bit fields (note that the         */
/* values are already shifted into the low-order bits of the constant.      */
#define HST_RFTC_WS_SEL_DSB_ASK     0x0
#define HST_RFTC_WS_SEL_SSB_ASK     0x1
#define HST_RFTC_WS_SEL_PR_ASK      0x2

/* HST_RFTC_WS_CFG register helper macros                                   */
/* The size of the bit fields in the HST_RFTC_WS_CFG register.              */
#define HST_RFTC_WS_CFG_STATUS_SIZE  1
#define HST_RFTC_WS_CFG_RFU1_SIZE    31
/* The number of bits that fields are left-shifted in the HST_RFTC_WS_CFG   */
/* register.                                                                */
#define HST_RFTC_WS_CFG_STATUS_SHIFT 0
#define HST_RFTC_WS_CFG_RFU1_SHIFT                                          \
    (HST_RFTC_WS_CFG_STATUS_SHIFT + HST_RFTC_WS_CFG_STATUS_SIZE)
/* HST_RFTC_WS_CFG bit-field retrieval and testing macros                   */
/* rv - the value from the HST_RFTC_WS_CFG register                         */
#define HST_RFTC_WS_CFG_IS_ENABLED(rv)                                      \
    REGISTER_TEST_BIT(rv, HST_RFTC_WS_CFG_STATUS_SHIFT)
#define HST_RFTC_WS_CFG_IS_DISABLED(rv)                                     \
    !(REGISTER_TEST_BIT(rv, HST_RFTC_WS_CFG_STATUS_SHIFT))
#define HST_RFTC_WS_CFG_GET_RFU1(rv)                                        \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_WS_CFG_RFU1_SIZE,                            \
                      HST_RFTC_WS_CFG_RFU1_SHIFT)
/* HST_RFTC_WS_CFG bit-field value creation macros                          */
/* va - the value to put into the bit field                                 */
#define HST_RFTC_WS_CFG_ENABLED                                             \
    CREATE_1BIT_MASK(HST_RFTC_WS_CFG_STATUS_SHIFT)
#define HST_RFTC_WS_CFG_DISABLED    0
#define HST_RFTC_WS_CFG_RFU1(va)                                            \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_WS_CFG_RFU1_SIZE,                             \
                     HST_RFTC_WS_CFG_RFU1_SHIFT)
/* HST_RFTC_WS_CFG register value modification macros (i.e., will modify    */
/* the field specified within an already existing register value).          */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_RFTC_WS_CFG_SET_ENABLED(rv)                                     \
    REGISTER_SET_BIT(rv, HST_RFTC_WS_CFG_STATUS_SHIFT)
#define HST_RFTC_WS_CFG_SET_DISABLED(rv)                                    \
    REGISTER_CLR_BIT(rv, HST_RFTC_WS_CFG_STATUS_SHIFT)
#define HST_RFTC_WS_CFG_SET_RFU1(rv, va)                                    \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_WS_CFG_RFU1_SIZE,                           \
                       HST_RFTC_WS_CFG_RFU1_SHIFT)

/* HST_RFTC_WS_TX_I_Q_NN register helper macros                             */
/* The size of the bit fields in the HST_RFTC_WS_TX_I_Q_NN register.        */
#define HST_RFTC_WS_TX_I_Q_NN_TXQAMP_SIZE   12
#define HST_RFTC_WS_TX_I_Q_NN_RFU1_SIZE     4
#define HST_RFTC_WS_TX_I_Q_NN_TXIAMP_SIZE   12
#define HST_RFTC_WS_TX_I_Q_NN_RFU2_SIZE     4
/* The number of bits that fields are left-shifted in the                   */
/* HST_RFTC_WS_TX_I_Q_NN register.                                          */
#define HST_RFTC_WS_TX_I_Q_NN_TXQAMP_SHIFT  0
#define HST_RFTC_WS_TX_I_Q_NN_RFU1_SHIFT                                    \
    (HST_RFTC_WS_TX_I_Q_NN_TXQAMP_SHIFT + HST_RFTC_WS_TX_I_Q_NN_TXQAMP_SIZE)
#define HST_RFTC_WS_TX_I_Q_NN_TXIAMP_SHIFT                                  \
    (HST_RFTC_WS_TX_I_Q_NN_RFU1_SHIFT + HST_RFTC_WS_TX_I_Q_NN_RFU1_SIZE)
#define HST_RFTC_WS_TX_I_Q_NN_RFU2_SHIFT                                    \
    (HST_RFTC_WS_TX_I_Q_NN_TXIAMP_SHIFT + HST_RFTC_WS_TX_I_Q_NN_TXIAMP_SIZE)
/* HST_RFTC_WS_TX_I_Q_NN bit-field retrieval and testing macros             */
/* rv - the value from the HST_RFTC_WS_TX_I_Q_NN register                   */
#define HST_RFTC_WS_TX_I_Q_NN_GET_TXQAMP(rv)                                \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_WS_TX_I_Q_NN_TXQAMP_SIZE,                    \
                      HST_RFTC_WS_TX_I_Q_NN_TXQAMP_SHIFT)
#define HST_RFTC_WS_TX_I_Q_NN_GET_RFU1(rv)                                  \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_WS_TX_I_Q_NN_RFU1_SIZE,                      \
                      HST_RFTC_WS_TX_I_Q_NN_RFU1_SHIFT)
#define HST_RFTC_WS_TX_I_Q_NN_GET_TXIAMP(rv)                                \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_WS_TX_I_Q_NN_TXIAMP_SIZE,                    \
                      HST_RFTC_WS_TX_I_Q_NN_TXIAMP_SHIFT)
#define HST_RFTC_WS_TX_I_Q_NN_GET_RFU2(rv)                                  \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_WS_TX_I_Q_NN_RFU2_SIZE,                      \
                      HST_RFTC_WS_TX_I_Q_NN_RFU2_SHIFT)
/* HST_RFTC_WS_TX_I_Q_NN bit-field value creation macros                    */
/* va - the value to put into the bit field                                 */
#define HST_RFTC_WS_TX_I_Q_NN_TXQAMP(va)                                    \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_WS_TX_I_Q_NN_TXQAMP_SIZE,                     \
                     HST_RFTC_WS_TX_I_Q_NN_TXQAMP_SHIFT)
#define HST_RFTC_WS_TX_I_Q_NN_RFU1(va)                                      \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_WS_TX_I_Q_NN_RFU1_SIZE,                       \
                     HST_RFTC_WS_TX_I_Q_NN_RFU1_SHIFT)
#define HST_RFTC_WS_TX_I_Q_NN_TXIAMP(va)                                    \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_WS_TX_I_Q_NN_TXIAMP_SIZE,                     \
                     HST_RFTC_WS_TX_I_Q_NN_TXIAMP_SHIFT)
#define HST_RFTC_WS_TX_I_Q_NN_RFU2(va)                                      \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_WS_TX_I_Q_NN_RFU2_SIZE,                       \
                     HST_RFTC_WS_TX_I_Q_NN_RFU2_SHIFT)
/* HST_RFTC_WS_TX_I_Q_NN register value modification macros (i.e., will     */
/* modify the field specified within an already existing register value).   */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_RFTC_WS_TX_I_Q_NN_SET_TXQAMP(rv, va)                            \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_WS_TX_I_Q_NN_TXQAMP_SIZE,                   \
                       HST_RFTC_WS_TX_I_Q_NN_TXQAMP_SHIFT)
#define HST_RFTC_WS_TX_I_Q_NN_SET_RFU1(rv, va)                              \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_WS_TX_I_Q_NN_RFU1_SIZE,                     \
                       HST_RFTC_WS_TX_I_Q_NN_RFU1_SHIFT)
#define HST_RFTC_WS_TX_I_Q_NN_SET_TXIAMP(rv, va)                            \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_WS_TX_I_Q_NN_TXIAMP_SIZE,                   \
                       HST_RFTC_WS_TX_I_Q_NN_TXIAMP_SHIFT)
#define HST_RFTC_WS_TX_I_Q_NN_SET_RFU2(rv, va)                              \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_WS_TX_I_Q_NN_RFU2_SIZE,                     \
                       HST_RFTC_WS_TX_I_Q_NN_RFU2_SHIFT)

/* HST_RFTC_WS_TX_SD_H_XXX register helper macros                           */
/* The size of the bit fields in the HST_RFTC_WS_TX_SD_H_XXX register.      */
#define HST_RFTC_WS_TX_SD_H_XXX_HOLD_SIZE    12
#define HST_RFTC_WS_TX_SD_H_XXX_RFU1_SIZE    4
#define HST_RFTC_WS_TX_SD_H_XXX_NUMSAMP_SIZE 5
#define HST_RFTC_WS_TX_SD_H_XXX_LUTADDR_SIZE 6
#define HST_RFTC_WS_TX_SD_H_XXX_QSWITCH_SIZE 1
#define HST_RFTC_WS_TX_SD_H_XXX_ISWITCH_SIZE 1
#define HST_RFTC_WS_TX_SD_H_XXX_RFU2_SIZE    3
/* The number of bits that fields are left-shifted in the                   */
/* HST_RFTC_WS_TX_SD_H_XXX register.                                        */
#define HST_RFTC_WS_TX_SD_H_XXX_HOLD_SHIFT   0
#define HST_RFTC_WS_TX_SD_H_XXX_RFU1_SHIFT                                  \
    (HST_RFTC_WS_TX_SD_H_XXX_HOLD_SHIFT +                                   \
     HST_RFTC_WS_TX_SD_H_XXX_HOLD_SIZE)
#define HST_RFTC_WS_TX_SD_H_XXX_NUMSAMP_SHIFT                               \
    (HST_RFTC_WS_TX_SD_H_XXX_RFU1_SHIFT +                                   \
     HST_RFTC_WS_TX_SD_H_XXX_RFU1_SIZE)
#define HST_RFTC_WS_TX_SD_H_XXX_LUTADDR_SHIFT                               \
    (HST_RFTC_WS_TX_SD_H_XXX_NUMSAMP_SHIFT +                                \
     HST_RFTC_WS_TX_SD_H_XXX_NUMSAMP_SIZE)
#define HST_RFTC_WS_TX_SD_H_XXX_QSWITCH_SHIFT                               \
    (HST_RFTC_WS_TX_SD_H_XXX_LUTADDR_SHIFT +                                \
     HST_RFTC_WS_TX_SD_H_XXX_LUTADDR_SIZE)
#define HST_RFTC_WS_TX_SD_H_XXX_ISWITCH_SHIFT                               \
    (HST_RFTC_WS_TX_SD_H_XXX_QSWITCH_SHIFT +                                \
     HST_RFTC_WS_TX_SD_H_XXX_QSWITCH_SIZE)
#define HST_RFTC_WS_TX_SD_H_XXX_RFU2_SHIFT                                  \
    (HST_RFTC_WS_TX_SD_H_XXX_ISWITCH_SHIFT +                                \
     HST_RFTC_WS_TX_SD_H_XXX_ISWITCH_SIZE)
/* HST_RFTC_WS_TX_SD_H_XXX bit-field retrieval and testing macros           */
/* rv - the value from the HST_RFTC_WS_TX_SD_H_XXX register                 */
#define HST_RFTC_WS_TX_SD_H_XXX_GET_HOLD(rv)                                \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_WS_TX_SD_H_XXX_HOLD_SIZE,                    \
                      HST_RFTC_WS_TX_SD_H_XXX_HOLD_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_GET_RFU1(rv)                                \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_WS_TX_SD_H_XXX_RFU1_SIZE,                    \
                      HST_RFTC_WS_TX_SD_H_XXX_RFU1_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_GET_NUMSAMP(rv)                             \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_WS_TX_SD_H_XXX_NUMSAMP_SIZE,                 \
                      HST_RFTC_WS_TX_SD_H_XXX_NUMSAMP_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_GET_LUTADDR(rv)                             \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_WS_TX_SD_H_XXX_LUTADDR_SIZE,                 \
                      HST_RFTC_WS_TX_SD_H_XXX_LUTADDR_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_QSWITCH_IS_ENABLED(rv)                      \
    REGISTER_TEST_BIT(rv, HST_RFTC_WS_TX_SD_H_XXX_QSWITCH_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_QSWITCH_IS_DISABLED(rv)                     \
    !(REGISTER_TEST_BIT(rv, HST_RFTC_WS_TX_SD_H_XXX_QSWITCH_SHIFT))
#define HST_RFTC_WS_TX_SD_H_XXX_ISWITCH_IS_ENABLED(rv)                      \
    REGISTER_TEST_BIT(rv, HST_RFTC_WS_TX_SD_H_XXX_ISWITCH_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_ISWITCH_IS_DISABLED(rv)                     \
    !(REGISTER_TEST_BIT(rv, HST_RFTC_WS_TX_SD_H_XXX_ISWITCH_SHIFT))
#define HST_RFTC_WS_TX_SD_H_XXX_GET_RFU2(rv)                                \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_WS_TX_SD_H_XXX_RFU2_SIZE,                    \
                      HST_RFTC_WS_TX_SD_H_XXX_RFU2_SHIFT)
/* HST_RFTC_WS_TX_SD_H_XXX bit-field value creation macros                  */
/* va - the value to put into the bit field                                 */
#define HST_RFTC_WS_TX_SD_H_XXX_HOLD(va)                                    \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_WS_TX_SD_H_XXX_HOLD_SIZE,                     \
                     HST_RFTC_WS_TX_SD_H_XXX_HOLD_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_RFU1(va)                                    \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_WS_TX_SD_H_XXX_RFU1_SIZE,                     \
                     HST_RFTC_WS_TX_SD_H_XXX_RFU1_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_NUMSAMP(va)                                 \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_WS_TX_SD_H_XXX_NUMSAMP_SIZE,                  \
                     HST_RFTC_WS_TX_SD_H_XXX_NUMSAMP_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_LUTADDR(va)                                 \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_WS_TX_SD_H_XXX_LUTADDR_SIZE,                  \
                     HST_RFTC_WS_TX_SD_H_XXX_LUTADDR_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_QSWITCH_ENABLED                             \
    CREATE_1BIT_MASK(HST_RFTC_WS_TX_SD_H_XXX_QSWITCH_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_QSWITCH_DISABLED   0
#define HST_RFTC_WS_TX_SD_H_XXX_ISWITCH_ENABLED                             \
    CREATE_1BIT_MASK(HST_RFTC_WS_TX_SD_H_XXX_ISWITCH_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_ISWITCH_DISABLED   0
#define HST_RFTC_WS_TX_SD_H_XXX_RFU2(va)                                    \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_WS_TX_SD_H_XXX_RFU2_SIZE,                     \
                     HST_RFTC_WS_TX_SD_H_XXX_RFU2_SHIFT)
/* HST_RFTC_WS_TX_SD_H_XXX register value modification macros (i.e., will*/
/* modify the field specified within an already existing register value).   */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_RFTC_WS_TX_SD_H_XXX_SET_HOLD(rv, va)                            \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_WS_TX_SD_H_XXX_HOLD_SIZE,                   \
                       HST_RFTC_WS_TX_SD_H_XXX_HOLD_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_SET_RFU1(rv, va)                            \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_WS_TX_SD_H_XXX_RFU1_SIZE,                   \
                       HST_RFTC_WS_TX_SD_H_XXX_RFU1_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_SET_NUMSAMP(rv, va)                         \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_WS_TX_SD_H_XXX_NUMSAMP_SIZE,                \
                       HST_RFTC_WS_TX_SD_H_XXX_NUMSAMP_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_SET_LUTADDR(rv, va)                         \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_WS_TX_SD_H_XXX_LUTADDR_SIZE,                \
                       HST_RFTC_WS_TX_SD_H_XXX_LUTADDR_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_SET_QSWITCH_ENABLED(rv)                     \
    REGISTER_SET_BIT(rv, HST_RFTC_WS_TX_SD_H_XXX_QSWITCH_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_SET_QSWITCH_DISABLED(rv)                    \
    REGISTER_CLR_BIT(rv, HST_RFTC_WS_TX_SD_H_XXX_QSWITCH_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_SET_ISWITCH_ENABLED(rv)                     \
    REGISTER_SET_BIT(rv, HST_RFTC_WS_TX_SD_H_XXX_ISWITCH_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_SET_ISWITCH_DISABLED(rv)                    \
    REGISTER_CLR_BIT(rv, HST_RFTC_WS_TX_SD_H_XXX_ISWITCH_SHIFT)
#define HST_RFTC_WS_TX_SD_H_XXX_SET_RFU2(rv, va)                            \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_WS_TX_SD_H_XXX_RFU2_SIZE,                   \
                       HST_RFTC_WS_TX_SD_H_XXX_RFU2_SHIFT)

/* HST_RFTC_FRQHOP_CFG register helper macros                               */
/* The size of the bit fields in the HST_RFTC_FRQHOP_CFG register.          */
#define HST_RFTC_FRQHOP_CFG_STATUS_SIZE  1
#define HST_RFTC_FRQHOP_CFG_RFU1_SIZE    31
/* The number of bits that fields are left-shifted in the                   */
/* HST_RFTC_FRQHOP_CFG register.                                            */
#define HST_RFTC_FRQHOP_CFG_STATUS_SHIFT 0
#define HST_RFTC_FRQHOP_CFG_RFU1_SHIFT                                      \
    (HST_RFTC_FRQHOP_CFG_STATUS_SHIFT + HST_RFTC_FRQHOP_CFG_STATUS_SIZE)
/* HST_RFTC_FRQHOP_CFG bit-field retrieval and testing macros               */
/* rv - the value from the HST_RFTC_FRQHOP_CFG register                     */
#define HST_RFTC_FRQHOP_CFG_IS_ENABLED(rv)                                  \
    REGISTER_TEST_BIT(rv, HST_RFTC_FRQHOP_CFG_STATUS_SHIFT)
#define HST_RFTC_FRQHOP_CFG_IS_DISABLED(rv)                                 \
    !(REGISTER_TEST_BIT(rv, HST_RFTC_FRQHOP_CFG_STATUS_SHIFT))
#define HST_RFTC_FRQHOP_CFG_GET_RFU1(rv)                                    \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_FRQHOP_CFG_RFU1_SIZE,                        \
                      HST_RFTC_FRQHOP_CFG_RFU1_SHIFT)
/* HST_RFTC_FRQHOP_CFG bit-field value creation macros                      */
/* va - the value to put into the bit field                                 */
#define HST_RFTC_FRQHOP_CFG_ENABLED                                         \
    CREATE_1BIT_MASK(HST_RFTC_FRQHOP_CFG_STATUS_SHIFT)
#define HST_RFTC_FRQHOP_CFG_DISABLED    0
#define HST_RFTC_FRQHOP_CFG_RFU1(va)                                        \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_FRQHOP_CFG_RFU1_SIZE,                         \
                     HST_RFTC_FRQHOP_CFG_RFU1_SHIFT)
/* HST_RFTC_FRQHOP_CFG register value modification macros (i.e., will modify*/
/* the field specified within an already existing register value).          */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_RFTC_FRQHOP_CFG_SET_ENABLED(rv)                                 \
    REGISTER_SET_BIT(rv, HST_RFTC_FRQHOP_CFG_STATUS_SHIFT)
#define HST_RFTC_FRQHOP_CFG_SET_DISABLED(rv)                                \
    REGISTER_CLR_BIT(rv, HST_RFTC_FRQHOP_CFG_STATUS_SHIFT)
#define HST_RFTC_FRQHOP_CFG_SET_RFU1(rv, va)                                \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_FRQHOP_CFG_RFU1_SIZE,                       \
                       HST_RFTC_FRQHOP_CFG_RFU1_SHIFT)

/* HST_RFTC_FRQHOP_CHAN_NN register helper macros                           */
/* The size of the bit fields in the HST_RFTC_FRQHOP_CHAN_NN register.      */
#define HST_RFTC_FRQHOP_CHAN_NN_SELECT_SIZE     6
#define HST_RFTC_FRQHOP_CHAN_NN_RFU1_SIZE       26
/* The number of bits that fields are left-shifted in the                   */
/* HST_RFTC_FRQHOP_CHAN_NN register.                                        */
#define HST_RFTC_FRQHOP_CHAN_NN_SELECT_SHIFT    0
#define HST_RFTC_FRQHOP_CHAN_NN_RFU1_SHIFT                                  \
    (HST_RFTC_FRQHOP_CHAN_NN_SELECT_SHIFT + HST_RFTC_FRQHOP_CHAN_NN_SELECT_SIZE)
/* HST_RFTC_FRQHOP_CHAN_NN bit-field retrieval and testing macros           */
/* rv - the value from the HST_RFTC_FRQHOP_CHAN_NN register                 */
#define HST_RFTC_FRQHOP_CHAN_NN_GET_SELECT(rv)                              \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_FRQHOP_CHAN_NN_SELECT_SIZE,                  \
                      HST_RFTC_FRQHOP_CHAN_NN_SELECT_SHIFT)
#define HST_RFTC_FRQHOP_CHAN_NN_GET_RFU1(rv)                                \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      HST_RFTC_FRQHOP_CHAN_NN_RFU1_SIZE,                    \
                      HST_RFTC_FRQHOP_CHAN_NN_RFU1_SHIFT)
/* HST_RFTC_FRQHOP_CHAN_NN bit-field value creation macros                  */
/* va - the value to put into the bit field                                 */
#define HST_RFTC_FRQHOP_CHAN_NN_SELECT(va)                                  \
    BIT_FIELD_CREATE(va,                                                    \
                      HST_RFTC_FRQHOP_CHAN_NN_SELECT_SIZE,                  \
                      HST_RFTC_FRQHOP_CHAN_NN_SELECT_SHIFT)
#define HST_RFTC_FRQHOP_CHAN_NN_RFU1(va)                                    \
    BIT_FIELD_CREATE(va,                                                    \
                     HST_RFTC_FRQHOP_CHAN_NN_RFU1_SIZE,                     \
                     HST_RFTC_FRQHOP_CHAN_NN_RFU1_SHIFT)
/* HST_RFTC_FRQHOP_CHAN_NN register value modification macros (i.e., will   */
/* modify the field specified within an already existing register value).   */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_RFTC_FRQHOP_CHAN_NN_SET_SELECT(rv, va)                          \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_FRQHOP_CHAN_NN_SELECT_SIZE,                 \
                       HST_RFTC_FRQHOP_CHAN_NN_SELECT_SHIFT)
#define HST_RFTC_FRQHOP_CHAN_NN_SET_RFU1(rv, va)                            \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       HST_RFTC_FRQHOP_CHAN_NN_RFU1_SIZE,                   \
                       HST_RFTC_FRQHOP_CHAN_NN_RFU1_SHIFT)

/* Macros for accessing bits in HST_RFTC_FRQCH_CMDSTART */
#define HST_RFTC_FRQCH_CMDSTART_EN(rv)   (rv & 0x00000100)
#define HST_RFTC_FRQCH_CMDSTART_CHAN(rv) (rv & 0x000000FF)

/* MAC_NV_UPDATE_CONTROL register helper macros                             */
#define MAC_NV_UPDATE_CONTROL_UPDATE_FLASH          0
#define MAC_NV_UPDATE_CONTROL_UPDATE_BOOT_LOADER    1
#define MAC_NV_UPDATE_CONTROL_UNLOCK_SECT0_PATTERN  0xA5A5
/* The size of the bit fields in the MAC_NV_UPDATE_CONTROL register.        */
#define MAC_NV_UPDATE_CONTROL_UPDATE_MODE_SIZE      2
#define MAC_NV_UPDATE_CONTROL_UNLOCK_SECTOR_0_SIZE  16
/* The number of bits that fields are left-shifted in the                   */
/* MAC_NV_UPDATE_CONTROL register.                                          */
#define MAC_NV_UPDATE_CONTROL_UPDATE_MODE_SHIFT     0
#define MAC_NV_UPDATE_CONTROL_UNLOCK_SECTOR_0_SHIFT 16                      \
/* MAC_NV_UPDATE_CONTROL bit-field retrieval and testing macros             */
/* rv - the value from the MAC_NV_UPDATE_CONTROL register                   */
#define MAC_NV_UPDATE_CONTROL_GET_UPDATE_MODE(rv)                           \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      MAC_NV_UPDATE_CONTROL_UPDATE_MODE_SIZE,               \
                      MAC_NV_UPDATE_CONTROL_UPDATE_MODE_SHIFT)
#define MAC_NV_UPDATE_CONTROL_GET_UNLOCK_SECTOR_0(rv)                       \
    BIT_FIELD_EXTRACT(rv,                                                   \
                      MAC_NV_UPDATE_CONTROL_UNLOCK_SECTOR_0_SIZE,           \
                      MAC_NV_UPDATE_CONTROL_UNLOCK_SECTOR_0_SHIFT)
/* MAC_NV_UPDATE_CONTROL bit-field value creation macros                    */
/* va - the value to put into the bit field                                 */
#define MAC_NV_UPDATE_CONTROL_UPDATE_MODE(va)                               \
    BIT_FIELD_CREATE(va,                                                    \
                     MAC_NV_UPDATE_CONTROL_UPDATE_MODE_SIZE,                \
                     MAC_NV_UPDATE_CONTROL_UPDATE_MODE_SHIFT)
#define MAC_NV_UPDATE_CONTROL_UNLOCK_SECTOR_0(va)                           \
    BIT_FIELD_CREATE(va,                                                    \
                     MAC_NV_UPDATE_CONTROL_UNLOCK_SECTOR_0_SIZE,            \
                     MAC_NV_UPDATE_CONTROL_UNLOCK_SECTOR_0_SHIFT)
/* MAC_NV_UPDATE_CONTROL register value modification macros (i.e., will     */
/* modify the field specified within an already existing register value).   */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define MAC_NV_UPDATE_CONTROL_SET_UPDATE_MODE(rv, va)                       \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       MAC_NV_UPDATE_CONTROL_UPDATE_MODE_SIZE,              \
                       MAC_NV_UPDATE_CONTROL_UPDATE_MODE_SHIFT)
#define MAC_NV_UPDATE_CONTROL_SET_UNLOCK_SECTOR_0(rv, va)                   \
    REGISTER_SET_VALUE(rv,                                                  \
                       va,                                                  \
                       MAC_NV_UPDATE_CONTROL_UNLOCK_SECTOR_0_SIZE,          \
                       MAC_NV_UPDATE_CONTROL_UNLOCK_SECTOR_0_SHIFT)

/* Macros for access bits in HST_PROTSCH_TXTIME_ON and                      */
/* HST_PROTSCH_TXTIME_OFF regs                                              */
#define HST_PROTSCH_TXTIME_GETTIME(rv) (rv & 0x00007FFF)

/* Macros for access bits in HST_PROTSCH_TXTIME_ON_OVHD and                 */
/* HST_PROTSCH_TXTIME_OFF_OVHD regs                                         */
#define HST_PROTSCH_TXTIME_OVHD_GETTIME(rv) (rv & 0x00007FFF)

/* macros for accessnig bits in HST_PROTSCH_ADJCW register */
#define HST_PROTSCH_ADJCW_GET_ADJ_TIME(rv)         (rv & 0xFF)
#define HST_PROTSCH_ADJCW_SET_ADJ_TIME(rv,va)      ((rv & ~0xFF) | (va & 0xFF))

/* macros for accessing/interpreting the MAC_RFTRANSINFO register */
/*
 *   NOTE: When we have more than these two product variants, we may need
 *          to define masks to identify Indy family of devices
*/
#define MAC_RFTRANSINFO_R1000_ID      0x000C00B0
#define MAC_RFTRANSINFO_R2000_ID      0x02A000A0
#define MAC_RFTRANSINFO_R500_ID       0x32A000A0

/* MAC_FW_DEFAULT_VALUE_SOURCE register bit access macros */
#define MAC_FW_DEFAULT_VALUE_SRC_CAL_RFFWDPWR_SHIFT              0
#define MAC_FW_DEFAULT_VALUE_SRC_CAL_RFREVPWR_SHIFT              1
#define MAC_FW_DEFAULT_VALUE_SRC_CAL_AMBIENT_TEMP_SHIFT          2
#define MAC_FW_DEFAULT_VALUE_SRC_CAL_XCVR_TEMP_SHIFT             3
#define MAC_FW_DEFAULT_VALUE_SRC_CAL_ANT_SENSE_SHIFT             4
#define MAC_FW_DEFAULT_VALUE_SRC_CAL_PA_TEMP_SHIFT               5
#define MAC_FW_DEFAULT_VALUE_SRC_CAL_PA_CURRENT_SHIFT            6
#define MAC_FW_DEFAULT_VALUE_SRC_CAL_PA_BIAS_DAC_SHIFT           7
#define MAC_FW_DEFAULT_VALUE_SRC_CAL_RSSI_SHIFT                  8
#define MAC_FW_DEFAULT_VALUE_SRC_CAL_GROSS_GAIN_SHIFT            9

/* macros for accessing/interpreting the HST_IMPINJ_EXTENSIONS register */
#define HST_IMPINJ_EXTENSIONS_BLOCKWRITE_MODE_SHIFT  0     
#define HST_IMPINJ_EXTENSIONS_BLOCKWRITE_MODE_SIZE   4     
#define HST_IMPINJ_EXTENSIONS_TAG_FOCUS_SHIFT  4
#define HST_IMPINJ_EXTENSIONS_TAG_FOCUS_SIZE   1
#define HST_IMPINJ_EXTENSIONS_FAST_ID_SHIFT   5
#define HST_IMPINJ_EXTENSIONS_FAST_ID_SIZE    1
#define HST_IMPINJ_EXTENSIONS_RFU1_SHIFT             6
#define HST_IMPINJ_EXTENSIONS_RFU1_SIZE             26


/* HST_IMPINJ_EXTENSIONS bit-field retrieval and testing macros                       */
/* rv - the value from the HST_IMPINJ_EXTENSIONS register                             */
#define HST_IMPINJ_EXTENSIONS_GET_BLOCKWRITE_MODE(rv)                  \
    BIT_FIELD_EXTRACT(rv, HST_IMPINJ_EXTENSIONS_BLOCKWRITE_MODE_SIZE,  \
                          HST_IMPINJ_EXTENSIONS_BLOCKWRITE_MODE_SHIFT)
#define HST_IMPINJ_EXTENSIONS_GET_TAG_FOCUS(rv)                  \
    BIT_FIELD_EXTRACT(rv, HST_IMPINJ_EXTENSIONS_TAG_FOCUS_SIZE,  \
                          HST_IMPINJ_EXTENSIONS_TAG_FOCUS_SHIFT)
#define HST_IMPINJ_EXTENSIONS_GET_FAST_ID(rv)                   \
    BIT_FIELD_EXTRACT(rv, HST_IMPINJ_EXTENSIONS_FAST_ID_SIZE,   \
                          HST_IMPINJ_EXTENSIONS_FAST_ID_SHIFT)

#define HST_IMPINJ_EXTENSIONS_BLOCKWRITE_MODE(rv)                      \
    BIT_FIELD_CREATE(rv, HST_IMPINJ_EXTENSIONS_BLOCKWRITE_MODE_SIZE,   \
                         HST_IMPINJ_EXTENSIONS_BLOCKWRITE_MODE_SHIFT)
#define HST_IMPINJ_EXTENSIONS_TAG_FOCUS(rv)                      \
    BIT_FIELD_CREATE(rv, HST_IMPINJ_EXTENSIONS_TAG_FOCUS_SIZE,   \
                         HST_IMPINJ_EXTENSIONS_TAG_FOCUS_SHIFT)
#define HST_IMPINJ_EXTENSIONS_FAST_ID(rv)                       \
    BIT_FIELD_CREATE(rv, HST_IMPINJ_EXTENSIONS_FAST_ID_SIZE,    \
                         HST_IMPINJ_EXTENSIONS_FAST_ID_SHIFT)
#define HST_IMPINJ_EXTENSIONS_RFU1(rv)                       \
    BIT_FIELD_CREATE(rv, HST_IMPINJ_EXTENSIONS_RFU1_SIZE,    \
                         HST_IMPINJ_EXTENSIONS_RFU1_SHIFT)
    
/* HST_IMPINJ_EXTENSIONS register value modification macros (i.e., will modify the    */
/* field specified within an already existing register value).              */
/* rv - register value that will have field set                             */
/* va - value to set the bit field to                                       */
#define HST_IMPINJ_EXTENSIONS_SET_BLOCKWRITE_MODE(rv, va)                   \
    REGISTER_SET_VALUE(rv, va, HST_IMPINJ_EXTENSIONS_BLOCKWRITE_MODE_SIZE,  \
                               HST_IMPINJ_EXTENSIONS_BLOCKWRITE_MODE_SHIFT)
#define HST_IMPINJ_EXTENSIONS_SET_TAG_FOCUS(rv, va)                   \
    REGISTER_SET_VALUE(rv, va, HST_IMPINJ_EXTENSIONS_TAG_FOCUS_SIZE,  \
                               HST_IMPINJ_EXTENSIONS_TAG_FOCUS_SHIFT)
#define HST_IMPINJ_EXTENSIONS_SET_FAST_ID(rv, va)                    \
    REGISTER_SET_VALUE(rv, va, HST_IMPINJ_EXTENSIONS_FAST_ID_SIZE,   \
                               HST_IMPINJ_EXTENSIONS_FAST_ID_SHIFT)
    
/* Supported mode values for BLOCKWRITE_MODE */
#define BLOCKWRITE_MODE_AUTO_DETECT            0
#define BLOCKWRITE_MODE_FORCE_ONE_WORD         1
#define BLOCKWRITE_MODE_FORCE_TWO_WORD         2

#endif /* H_MACREGS_H */
