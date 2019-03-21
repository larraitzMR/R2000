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
 * $Id: tracer_file.cpp 60023 2009-09-24 23:59:22Z dshaheen $
 * 
 * Description:
 *     This file contains the implementation for file tracer.
 *     
 *
 *****************************************************************************
 */

#include <stdarg.h>
#include <stdio.h>
#include "tracer_file.h"
#include "rfid_exceptions.h"
#include "auto_lock_compat.h"
#include "compat_thread.h"

namespace rfid
{

////////////////////////////////////////////////////////////////////////////
// Name: TracerFile
//
// Description:
//   Initializes the console logging tracer
////////////////////////////////////////////////////////////////////////////
TracerFile::TracerFile(
    const char*         fileName,
    RFID_LOG_SEVERITY   minSeverity
    ) :
    Tracer(minSeverity),
    m_tracerFileWrapper(
        CPL_FileOpen(fileName, CPL_WRITE, CPL_CREAT | CPL_TRUNC))
{
    // Verify that the tracer file was created properly
    if (-1 == m_tracerFileWrapper.Get())
    {
        throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
    }

    // Create the mutex that is used to lock the file
    if (CPL_MutexInit(&m_tracerLock))
    {
        throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
    }
    m_tracerLockWrapper.Assume(&m_tracerLock);
} // TracerFile::TracerFile

////////////////////////////////////////////////////////////////////////////
// Name: LogMessage
//
// Description:
//   Logs a message.  The message will be prepended with the current
//   time and the severity.  If the severity is below the severity level
//   specified on startup, the message is discarded.
////////////////////////////////////////////////////////////////////////////
void TracerFile::LogMessage(
    RFID_LOG_SEVERITY   severity,
    const char*         format,
    va_list             arguments
    ) const
{
    // Lock the tracer log file
    CplMutexAutoLock tracerGuard(&m_tracerLock);

    char    buffer[MAX_BUFFER_SIZE];
    INT32S  bytes;

    // Get the current time and place that at the front of the buffer
    CPL_TimeSpec now;
    CPL_TimeSpecGet(&now);
    bytes =
        sprintf(
            buffer,
            "%d.%.9d,0x%.8x,%s,",
            now.seconds,
            now.nanoseconds,
            CPL_ThreadGetID(),
            Tracer::GetSeverityString(severity));

    // Create the message and log the message to the file
    if ((bytes += vsprintf(buffer + bytes, format, arguments)) > 0)
    {
        CPL_FileWrite(m_tracerFileWrapper, buffer, bytes);
    }

    CPL_FileFlush(m_tracerFileWrapper);
} // TracerFile::LogMessage

} // namespace rfid
