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
 * $Id: tracer_console.cpp 60023 2009-09-24 23:59:22Z dshaheen $
 * 
 * Description:
 *     This file contains the implementation for console tracer.
 *     
 *
 *****************************************************************************
 */

#include <stdarg.h>
#include <stdio.h>
#include "tracer_console.h"
#include "rfid_exceptions.h"
#include "auto_lock_compat.h"
#include "compat_thread.h"

namespace rfid
{

////////////////////////////////////////////////////////////////////////////
// Name: TracerConsole
//
// Description:
//   Initializes the console logging tracer
////////////////////////////////////////////////////////////////////////////
TracerConsole::TracerConsole(
    RFID_LOG_SEVERITY   severityThreshold
    ) :
    Tracer(severityThreshold)
{
    // Create the lock that is used to synchronize access to the tracer and
    // then wrap it so that it is automatically destructed
    if (CPL_MutexInit(&m_tracerLock))
    {
        throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
    }
    m_tracerLockWrapper.Assume(&m_tracerLock);
} // TracerConsole::TracerConsole

////////////////////////////////////////////////////////////////////////////
// Name: LogMessage
//
// Description:
//   Logs a message.  The message will be prepended with the current
//   time and the severity.  If the severity is below the severity level
//   specified on startup, the message is discarded.
////////////////////////////////////////////////////////////////////////////
void TracerConsole::LogMessage(
    RFID_LOG_SEVERITY   severity,
    const char*         format,
    va_list             arguments
    ) const
{
    // Grab the tracer lock
    CplMutexAutoLock tracerGuard(&m_tracerLock);

    // Get the current time and print it to the console as a prepend to the 
    // message
    CPL_TimeSpec now;
    CPL_TimeSpecGet(&now);
    printf(
        "%d.%.9d,0x%.8x,%s,",
        now.seconds,
        now.nanoseconds,
        CPL_ThreadGetID(),
        Tracer::GetSeverityString(severity));

    // Print message to the console
    vprintf(format, arguments);
    fflush(stdout);
} // TracerConsole::LogMessage

} // namespace rfid
