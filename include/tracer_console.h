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
 * $Id: tracer_console.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header presents the interface for the class that is a used to
 *     to represent the RFID tracing facility that logs messages to the console.
 *     
 *
 *****************************************************************************
 */

#ifndef TRACER_CONSOLE_H_INCLUDED
#define TRACER_CONSOLE_H_INCLUDED

#include "tracer.h"
#include "auto_handle_compat.h"
#include "compat_mutex.h"

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: TracerConsole
//
// Description: The console tracer class (i.e., logs to console)
////////////////////////////////////////////////////////////////////////////////
class TracerConsole : public Tracer
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name: TracerConsole
    //
    // Description:
    //   Initializes the console logging tracer
    //
    // Parameters:
    //   severityThreshold - minimum severity for messages to log
    //
    // Returns:
    //   Nothing
    ////////////////////////////////////////////////////////////////////////////
    TracerConsole(
        RFID_LOG_SEVERITY   severityThreshold
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name: ~TracerConsole
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
    ~TracerConsole()
    {
    } // ~TracerConsole

private:
    mutable CPL_Mutex   m_tracerLock;       // Lock for synchronizing tracing
    CplMutexAutoHandle  m_tracerLockWrapper;// Wrap for automatically cleaning
                                            // up the tracer lock

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
    //   Nothing
    ////////////////////////////////////////////////////////////////////////////
    void LogMessage(
        RFID_LOG_SEVERITY   severity,
        const char*         format,
        va_list             arguments
        ) const;

    // Prevent copying of tracer objects
    TracerConsole(const TracerConsole&);
    const TracerConsole& operator = (const TracerConsole&);
}; // class TracerConsole

} // namespace rfid

#endif // TRACER_CONSOLE_H_INCLUDED
