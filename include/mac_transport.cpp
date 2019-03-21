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
 * $Id: mac_transport.cpp 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This file contains the implementation for the base MAC transport.
 *     
 *
 *****************************************************************************
 */

#include "mac_transport.h"

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name:        MacTransport
// Description: Initializes a MAC transport object
////////////////////////////////////////////////////////////////////////////////
MacTransport::MacTransport(
    INT32U  transportHandle
    ) :
    m_transportHandle(transportHandle)
{
} // MacTransport::MacTransport

} // namespace rfid
