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
 * $Id: print_packet.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This file contains the prototype for a function that prints out replies
 *     from the radio in a descriptive fashion.
 *     
 *
 *****************************************************************************
 */

#ifndef PRINT_PACKET_H_INCLUDED
#define PRINT_PACKET_H_INCLUDED

#include "rfid_types.h"

void PrintPacket(
    const INT8U*    packet,
    int             indent
    );

#endif /* PRINT_PACKET_H_INCLUDED */
