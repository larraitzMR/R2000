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
 * $Id: tracer_null.h 60276 2009-10-19 18:21:43Z dshaheen $
 * 
 * Description:
 *     This header presents the interface for the class that is a used to
 *     to represent the RFID tracing facility that simply ignores requests to
 *     log a message.//
 *
 *****************************************************************************
 */

#ifndef TRACER_NULL_H_INCLUDED
#define TRACER_NULL_H_INCLUDED

#include "tracer.h"

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: TracerNull
//
// Description: The NULL tracer (i.e., throws away all log messages).
////////////////////////////////////////////////////////////////////////////////
class TracerNull : public Tracer
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name: TracerNull
    //
    // Description:
    //   Initializes the tracer class
    //
    // Parameters:
    //   severityThreshold - the minimum severity for logging
    //
    // Returns:
    //   Nothing
    ////////////////////////////////////////////////////////////////////////////
    TracerNull()
    {
        // Intentionally left blank
    } // ~TracerNull

    ////////////////////////////////////////////////////////////////////////////
    // Name: ~TracerNull
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
    ~TracerNull()
    {
        // Intentionally left blank
    } // ~TracerNull

private:
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
    //   format - the format string for the message (uses printf format specifiers)
    //   arguments - the variable argument list for the format specifier
    //
    // Returns:
    //   Nothing
    ////////////////////////////////////////////////////////////////////////////
    void LogMessage(
        RFID_LOG_SEVERITY   severity,
        const char*         format,
        va_list             arguments
        ) const
    {
        RFID_UNREFERENCED_LOCAL(severity);
        RFID_UNREFERENCED_LOCAL(format);
        RFID_UNREFERENCED_LOCAL(arguments);
        // Intentionally left blank
    } // LogMessage

    // Prevent copying of tracer objects
    TracerNull(const TracerNull&);
    const TracerNull& operator = (const TracerNull&);
}; // class TracerNull

} // namespace rfid

#endif // TRACER_NULL_H_INCLUDED
