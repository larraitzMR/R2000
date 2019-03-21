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
 * $Id: byte_swap.h 60050 2009-09-25 23:44:23Z dshaheen $
 * 
 * Description:
 *     This file contains inline functions that do byte-swapping for 16-bit and
 *     32-bit data types.
 *     
 *
 *****************************************************************************
 */

#include "rfid_types.h"

#ifdef COMPILE_BIG_ENDIAN

#define MacToHost16(x)                                                      \
    ((((x) & 0x00FF) << 8) |                                                   \
     (((x) & 0xFF00) >> 8))
#define MacToHost32(x)                                                      \
    (((INT32U) MacToHost16(x >> 16)) |                                      \
     (((INT32U) MacToHost16(x)) << 16))


#define HostToMac16(x)                                                      \
    ((((x) & 0x00FF) << 8) |                                                   \
     (((x) & 0xFF00) >> 8))
#define HostToMac32(x)                                                      \
    (((INT32U) HostToMac16(x >> 16)) |                                      \
     (((INT32U) HostToMac16(x)) << 16))
     
#else /* not COMPILE_BIG_ENDIAN */

#define MacToHost16(x)   x
#define MacToHost32(x)   x
#define HostToMac16(x)   x
#define HostToMac32(x)   x

#endif /* COMPILE_BIG_ENDIAN */
