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
 * $Id: tracer_file.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header presents the interface for the class that is a used to
 *     to represent the RFID tracing facility that logs messages to a file.
 *     
 *
 *****************************************************************************
 */

#ifndef TRACER_FILE_H_INCLUDED
#define TRACER_FILE_H_INCLUDED

#include "tracer.h"
#include "auto_handle_compat.h"
#include "compat_fildes.h"
#include "compat_mutex.h"

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: TracerFile
//
// Description: The file tracer class (i.e., logs all tracer messages to a file)
////////////////////////////////////////////////////////////////////////////////
class TracerFile : public Tracer
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name: TracerFile
    //
    // Description:
    //   Initializes the file logging tracer
    //
    // Parameters:
    //   fileName - name of the log file
    //   severityThreshold - minimum severity for messages to log
    //
    // Returns:
    //   Nothing
    ////////////////////////////////////////////////////////////////////////////
    TracerFile(
        const char*         fileName,
        RFID_LOG_SEVERITY   severityThreshold
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name: ~TracerFile
    //
    // Description:
    //   Shuts down the RFID logging subsystem.
    //
    // Parameters:
    //   None
    //
    // Returns:
    //   Nothing
    ////////////////////////////////////////////////////////////////////////////
    ~TracerFile()
    {
    } // ~TracerFile

private:
    CplFileAutoHandle   m_tracerFileWrapper;    // A wrapper around the tracer
                                                // file handle that ensures that
                                                // it is automatically destroyed
    mutable CPL_Mutex   m_tracerLock;           // A lock to prevent debug
                                                // messages from colliding
    CplMutexAutoHandle  m_tracerLockWrapper;    // A wrapper around the lock
                                                // to ensure automatic cleanup

    enum {
        MAX_BUFFER_SIZE = 1024
    };

    ////////////////////////////////////////////////////////////////////////////
    // Name: LogMessage
    //
    // Description:
    //   Logs a message.  The message will be prepended with the current
    //   time and the severity.  If the severity is below the severity level
    //   specified on startup, the message is discarded.
    //
    // Parameters:
    //   severity - the severity level for this message
    //   format - the format string for the message (uses printf format
    //     specifiers)
    //   arguments - the variable argument list for the format specifier
    //
    // Returns:
    //   true  - message logged succesfully
    //   false - failed to log message
    ////////////////////////////////////////////////////////////////////////////
    void LogMessage(
        RFID_LOG_SEVERITY   severity,
        const char*         format,
        va_list             arguments
        ) const;

    // Prevent copying of tracer objects
    TracerFile(const TracerFile&);
    const TracerFile& operator = (const TracerFile&);
}; // class TracerFile

} // namespace rfid

#endif // TRACER_FILE_H_INCLUDED
