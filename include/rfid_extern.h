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
 * $Id: rfid_extern.h 62042 2010-01-26 02:54:12Z dshaheen $
 * 
 * Description:
 *     This header defines externs for global variables that are used in multiple
 *     files.
 *     
 *
 *****************************************************************************
 */

#ifndef RFID_EXTERN_H_INCLUDED
#define RFID_EXTERN_H_INCLUDED

#include <memory>
#include "rfid_platform_types.h"
#include "tracer.h"

// Global constants
const INT32U RFID_MAX_ANTENNA_PORT                   = 15;
const INT32U RFID_MAX_ANTENNA_PORT_PHYSICAL          = 3;
const INT32U RFID_18K6C_MAX_SELECT_CRITERIA_CNT      = 8;
const INT32U RFID_18K6C_MAX_SELECT_MASK_CNT          = 255;
const INT32U RFID_18K6C_MAX_SINGULATION_CRITERIA_CNT = 1;
const INT32U RFID_18K6C_MAX_SINGULATION_MASK_CNT     = 496;
const INT32U RFID_18K6C_MAX_Q                        = 15;
const INT32U RFID_18K6C_MAX_RETRY                    = 255;
const INT32U RFID_18K6C_MAX_QUERYREP                 = 255;
const INT32U RFID_18K6C_MAX_THRESH_MULTIPLIER        = 255;
const INT32U RFID_18K6C_MAX_TAGSTOP                  = 1;
const INT32U RFID_18K6C_MAX_READ_COUNT               = 255;
const INT32U RFID_18K6C_MAX_WRITE_COUNT              = 32;
const INT32U RFID_18K6C_MAX_WRITE_RETRY              = 7;
const INT32U RFID_18K6C_MAX_BLOCK_WRITE_COUNT        = 255;
const INT32U RFID_18K6C_MAX_BLOCK_ERASE_COUNT        = 255;
const INT32U RFID_18K6C_MAX_BLOCK_WRITE_RETRY        = 7;

const INT32U RFID_WIDEBAND_RSSI_BASE_SAMPLES         = 32;
const INT32U RFID_NARROWBAND_RSSI_BASE_SAMPLES       = 8;

// The tracer object that is used by all parts of library
extern std::auto_ptr<rfid::Tracer> g_pTracer;

#endif // #ifndef RFID_EXTERN_H_INCLUDED
