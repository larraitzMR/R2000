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
 * $Id: rfid_types.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header file defines the generic cross-platform types exposed by the
 *     RFID Reader Library.
 *     
 *
 *****************************************************************************
 */

#ifndef RFID_TYPES_H_INCLUDED
#define RFID_TYPES_H_INCLUDED

#include "rfid_platform_types.h"

/* A typedef that should be used for RFID radio handles */
typedef HANDLE32    RFID_RADIO_HANDLE;

/******************************************************************************
 * Name: RFID_PACKET_CALLBACK_FUNCTION
 *
 * Description:
 *   The tag-protocol operation response packet callback function signature.
 *
 * Parameters:
 *   handle - the handle for the radio for which operation response packets are
 *     being returned
 *   bufferLength - the length of the buffer that contains the packets
 *   pBuffer - a buffer that contains one or more complete operation response
 *     packets
 *   context - an application-defined context value that was supplied when the
 *     the original tag-protocol operation function was invoked
 *
 * Returns:
 *   0 - continue making packet callbacks
 *   !0 - cancel the tag-protocol operation and stop making callbacks for that
 *
 *   The return value of the last packet callback for the tag-protocol operation
 *   is returned to the application if the application indicates the desire for
 *   this value when it invokes the tag-protocol opreation.
 ******************************************************************************/
typedef INT32S (RFID_CALLBACK * RFID_PACKET_CALLBACK_FUNCTION)(
    RFID_RADIO_HANDLE   handle,
    INT32U              bufferLength,
    const INT8U*        pBuffer,
    void*               context
    );

#endif  /* #ifndef RFID_TYPES_H_INCLUDED */
