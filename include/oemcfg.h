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
 * $Id: oemcfg.h 80229 2012-04-19 22:28:08Z rsales $
 * 
 * Description:
 *
 *   Public header file for oemcfg.c
 *   This includes the addresses of OEM Configuration data - these
 *   address values should be used when accessing oemconfiguration data both
 *   on the mac in host based software. These are not absolute addresses
 *   to non-volatile memory on the mac.
 *   See also oemcfg.c
 *
 *****************************************************************************
 */
#ifndef H_OEMCFG_H
#define H_OEMCFG_H

#include <oemcfgregs.h>

/* Preprocessor checks to make sure that the sizes of Custom Profiles 0 and 1 are the same */
#if ( OEMCFGLEN_CUSTOM_LPROF0_HEADER_1 != OEMCFGLEN_CUSTOM_LPROF1_HEADER_1 )
  #error "Custom Profile Header 1 lengths not equal!"
#endif
#if ( OEMCFGLEN_CUSTOM_LPROF0_HEADER_2 != OEMCFGLEN_CUSTOM_LPROF1_HEADER_2 )
  #error "Custom Profile Header 2 lengths not equal!"
#endif
#if ( OEMCFGLEN_CUSTOM_LPROF0_MAC != OEMCFGLEN_CUSTOM_LPROF1_MAC )
  #error "Custom Profile MAC register lengths not equal!"
#endif
#if ( OEMCFGLEN_CUSTOM_LPROF0_INDY != OEMCFGLEN_CUSTOM_LPROF1_INDY )
  #error "Custom Profile Indy register lengths not equal!"
#endif

#define OEMCFG_LPROF_HEAD_SIZE     (OEMCFGLEN_CUSTOM_LPROF0_HEADER_1 + OEMCFGLEN_CUSTOM_LPROF0_HEADER_2)
#define OEMCFG_LPROF_VREG_SIZE     (OEMCFGLEN_CUSTOM_LPROF0_MAC)
#define OEMCFG_LPROF_XCVR_SIZE     (OEMCFGLEN_CUSTOM_LPROF0_INDY)
#define OEMCFG_LPROF_SIZE          (OEMCFG_LPROF_HEAD_SIZE + OEMCFG_LPROF_VREG_SIZE + OEMCFG_LPROF_XCVR_SIZE)

#define OEMCFG_AUTO_INIT_1         0x13FFF8
#define OEMCFG_AUTO_INIT_2         0x13FFFC

/* OEM Structs */

typedef union oemInitializationConfiguration_s
{
  unsigned int raw;
  struct {
    unsigned int enable : 1,
                 reserved1 : 3,
                 hostif : 4,
                 region : 8,
                 platform : 4,
                 lbt_enable : 1,
                 reserved2 : 11;
  } fld;
}oemInitializationConfiguration_t;

typedef union xcvrHealthCheckConfig_s
{
  unsigned int raw;
  struct {
    unsigned int enable: 1,
                 handler : 2,
                 reserved : 1,
                 address : 12,
                 data : 16;
  } fld;
}xcvrHealthCheckConfig_t;

typedef struct xlinkdata
{
    INT16U  r1000_addr;
    INT16U  r1000_val;
}XcvrLinkProfileEntry;

typedef struct oemcfg_freqcfg_ent
{
  INT32U chan_info;
  INT32U plldivmult;
  INT32U plldacctl;
}OEMCFG_FREQCFG_ENT;

typedef struct oemcfg_phead0
{
	INT32U	xcvr_size	:	16,
	        vreg_size	:	15,
			valid_flag	:	1;
}LINKPROFILE_HEADER0;

typedef struct oemcfg_phead1
{
	INT32U	tilden_step		:	8,
			tilden_version	:	8,
			rfu				:	16;
}LINKPROFILE_HEADER1;

typedef struct oemcfg_linkprofile
{
	LINKPROFILE_HEADER0		head0;
	LINKPROFILE_HEADER1		head1;
    INT32U					vreg[OEMCFG_LPROF_VREG_SIZE];
    XcvrLinkProfileEntry	xcvr[OEMCFG_LPROF_XCVR_SIZE];
}OEM_LINKPROFILE;


/* for OEMCFGADDR_HW_OPTIONS0 */
/* Values use for the phys_host_if field in OEMCFGADDR_HW_OPTIONS0 */
#define OEMCFG_XMIT_GAIN_PROG_MIDMATCH_ENABLE   0x01
#define OEMCFG_XMIT_GAIN_PROG_INMATCH_ENABLE    0x02
#define OEMCFG_XMIT_GAIN_PROG_RFU1_ENABLE       0x04
#define OEMCFG_XMIT_GAIN_PROG_RFU2_ENABLE       0x08


typedef union oemcfg_hwopt0_s {
    unsigned int raw;
    struct {
    unsigned int    fwd_rfpwr_det_option        : 4,
                    rev_rfpwr_det_option        : 4,
                    drm_filter_option           : 3,
                    ext_drm_gpo_level           : 1,
                    pa_temp_sense_option        : 4,
                    ambient_temp_sense_option   : 4,
                    xcvr_temp_sense_option      : 4,
                    antenna_res_sense_option    : 4,
                    xmit_gain_prog_option       : 4;
    } format0;
} oemcfg_hwopt0_t;

/* for OEMCFGADDR_HW_OPTIONS1 */
/* Values use for the phys_host_if field in OEMCFGADDR_HW_OPTIONS1 */
#define OEMCFG_PHYS_HOST_IF_USB  0
#define OEMCFG_PHYS_HOST_IF_UART 1
#define OEMCFG_PHYS_HOST_IF_MAX OEMCFG_PHYS_HOST_IF_UART
#define OEMCFG_PHYS_HOST_IF_NONE 255

/* macro to access phys_host_if member from raw */
#define OEMCFG_PHYS_HOST_IF_GET(v) (v & 0xF)
#define OEMCFG_PHYS_HOST_IF_SET(curr,val) (curr |= (val & 0xF))
#define OEMCFG_PHYS_HOST_IF_CLR(v) (v &= ~0xF)
typedef union oemcfg_hwopt1_s {
    unsigned int raw;
    struct {
    unsigned int    phys_host_if  : 4,
                    rfu           : 28;
    } format0;
} oemcfg_hwopt1_t;


/* for OEMCFGADDR_HW_OPTIONS2 */
/* Values use for the phys_host_if field in OEMCFGADDR_HW_OPTIONS2 */
#define OEMCFG_FWDPWRDET_COMPOPTION_NONE    0x0
#define OEMCFG_FWDPWRDET_COMPOPTION_TEMP    0x1
#define OEMCFG_FWDPWRDET_COMPOPTION_FREQ    0x2
typedef union oemcfg_hwopt2_s {
    unsigned int raw;
    struct {
        unsigned int    fwdpwrdet_compens      : 4,
                        pa_current_dac_control : 4,
                        rfu                    : 24;
    } format0;
} oemcfg_hwopt2_t;

/* for OEMCFGADDR_HW_OPTIONS3 */
#define OEMCFG_CUSTOM_RX_LO_SELECTION_ENABLED   0x1
#define OEMCFG_CUSTOM_RX_LO_SELECTION_INT_LO    0x0
#define OEMCFG_CUSTOM_RX_LO_SELECTION_EXT_LO    0x1
typedef union oemcfg_hwopt3_s {
    unsigned int raw;
    struct {
    unsigned int    custom_rx_lo_sel_enable     : 4,
                    custom_rx_lo_sel_value      : 4,
                    low_gain_en_gpo_level       : 1,
                    rfu                         : 23;
    } format0;
} oemcfg_hwopt3_t;

/* macros for accessing OEMCFGADDR_LED_AVAIL availability bits
   Note inverted logic for backwards compatibility */
#define OEMCFG_IS_LED_AVAIL(oemval,lednum) ((oemval & (0x1 << lednum))^(0x1 << lednum))

/* macros for accessing OEMCFGADDR_GPIO_AVAIL availability bits */
#define OEMCFG_IS_GPIO_AVAIL(oemval,gpionum) ((oemval & (0x1 << gpionum))^(0x1 << gpionum))

/* macros for accessing OEMCFGADDR_ANT_AVAIL availability bits */
#define OEMCFG_IS_ANT_AVAIL(oemval,antnum) ((oemval & (0x1 << antnum))^(0x1 << antnum))

typedef union oemcfg_antavail_s {
    unsigned int raw;
    struct {
    unsigned int    ant0    : 1,
                    ant1    : 1,
                    ant2    : 1,
                    ant3    : 1,
                    rfu     : 28;
    } format0;
} oemcfg_antavail_t;

/* macros for accessing oemcfg_freqcfg_ent struct member bits
   pEnt is a pointer to a OEMCFG_FREQCFG_ENT struct*/
#define FREQCFG_ENT_GET_ENT_EN(pEnt)     (pEnt->chan_info & 0x00000001)
#define FREQCFG_ENT_GET_CHAN_EN(pEnt)    ((pEnt->chan_info & 0x00000002) >> 1)
#define FREQCFG_ENT_GET_PLLDIVMULT(pEnt) (pEnt->plldivmult)
#define FREQCFG_ENT_GET_PLLDACCTL(pEnt)  (pEnt->plldacctl)

/* macros for access OEMCFGADDR_PROTSCH_TXTIME bits */
#define OEMCFG_PROTSCH_TXON_EN(oemval)     (oemval & 0x00000001)
#define OEMCFG_PROTSCH_TXON_VALUE(oemval)  ((oemval >> 1)  & 0x00007FFF)
#define OEMCFG_PROTSCH_TXOFF_EN(oemval)    ((oemval >> 16) & 0x00000001)
#define OEMCFG_PROTSCH_TXOFF_VALUE(oemval) ((oemval >> 17) & 0x00007FFF)

/* macros for access OEMCFGADDR_PROTSCH_TXTIME_OVHD bits */
#define OEMCFG_PROTSCH_TXON_OVHD_EN(oemval)     (oemval & 0x00000001)
#define OEMCFG_PROTSCH_TXON_OVHD_VALUE(oemval)  ((oemval >> 1)  & 0x00007FFF)
#define OEMCFG_PROTSCH_TXOFF_OVHD_EN(oemval)    ((oemval >> 16) & 0x00000001)
#define OEMCFG_PROTSCH_TXOFF_OVHD_VALUE(oemval) ((oemval >> 17) & 0x00007FFF)

/* macros for accessing OEMCFGADDR_LBT_REGCFG entries */
#define OEMCFG_LBT_REGCFG_GET_EN(val)    ((val & 0x80000000) >> 31)
#define OEMCFG_LBT_REGCFG_RESTFLG(val)   ((val & 0x30000000) >> 28)
#define OEMCFG_LBT_REGCFG_GET_ADDR(val)  ((val & 0x0FFF0000) >> 16)
#define OEMCFG_LBT_REGCFG_GET_DATA(val)  (val & 0x0000FFFF)

/* macros for accessing OEMCFGADDR_LBT_CFGOPTS1 */
#define OEMCFG_LBT_CFGOPTS1_GET_LBT_EN(val)   (val & 0x00000001)
#define OEMCFG_LBT_CFGOPTS1_GET_SCANMODE(val) ((val & 0x00000002) >> 1)
#define OEMCFG_LBT_CFGOPTS1_GET_LBTCHANS(val) ((val & 0x00000030) >> 4)
#define OEMCFG_LBT_CFGOPTS1_GET_LBTGAIN(val) ((val & 0x00000040) >> 6)

/* macros for accessing boot cfg */
#define OEMCFG_BOOTOPTS_GET_DELAY(val)      (val & 0x0000000F)
#define OEMCFG_BOOTOPTS_GET_SHOWSPLASH(val) ((val & 0x00000010) >> 4)

/* macros for accessing OEMCFGADDR_DEFAULT_LINK_PROFILE */
#define OEMCFGADDR_DEFAULT_LINK_PROFILE_EN(oemval)     (oemval & 0x00000001)
#define OEMCFGADDR_DEFAULT_LINK_PROFILE_VALUE(oemval)  ((oemval >> 8) & 0x000000FF)

/* macros for OEM_STRING Commands */
#define OEM_STRING_MAX_BUFFER 160
#define OEM_STRING_MAX_OEM_WORDS (OEM_STRING_MAX_BUFFER/4)
#define OEM_STRING_MAX_NUM_CHAR 79
#define OEM_STRING_UNICODE_CHARACTER(val) (val & 0x0000FFFF)


#endif //H_OEMCFG_H
