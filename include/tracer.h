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
 * $Id: tracer.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header presents the interface for the base class that is a used to
 *     to represent the RFID tracing facility.
 *     
 *
 *****************************************************************************
 */

#ifndef TRACER_H_INCLUDED
#define TRACER_H_INCLUDED

#include "rfid_types.h"
#include <stdarg.h>

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: Tracer
//
// Description: The base class for the RFID logging/tracing facility.
////////////////////////////////////////////////////////////////////////////////
class Tracer
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name: RFID_LOG_SEVERITY
    //
    // Description: An enum type used to specify the severity of a logged message,
    //              as well as the current logging severity level.
    ////////////////////////////////////////////////////////////////////////////
    enum
    {
        RFID_LOG_SEVERITY_DEBUG     = 0,
        RFID_LOG_SEVERITY_INFO      = 4,
        RFID_LOG_SEVERITY_TRACE     = 7,
        RFID_LOG_SEVERITY_WARNING   = 10,
        RFID_LOG_SEVERITY_ERROR     = 12,
        RFID_LOG_SEVERITY_FATAL     = 15
    };
    typedef INT32U  RFID_LOG_SEVERITY;

    ////////////////////////////////////////////////////////////////////////////
    // Name: ~Tracer
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
    virtual ~Tracer()
    {
    } // ~Tracer

    ////////////////////////////////////////////////////////////////////////////
    // Name: PrintMessage
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
    //   ... - the variable argument list for the format specifier
    //
    // Returns:
    //   true  - message logged succesfully
    //   false - failed to log message
    ////////////////////////////////////////////////////////////////////////////
    void PrintMessage(
        RFID_LOG_SEVERITY   severity,
        const char*         format,
        ...
        ) const
    {
        // Verify that the message is at or above the threshold before logging
        if (severity >= m_severityThreshold)
        {
            va_list arguments;
            va_start(arguments, format);
            this->LogMessage(severity, format, arguments);
        }
    } // PrintMessage

protected:
    ////////////////////////////////////////////////////////////////////////////
    // Name: Tracer
    //
    // Description:
    //   Initializes the tracer class
    //
    // Parameters:
    //   severityThreshold - the minimum severity for logging messages.  Any
    //     message whose severity is lower than this severity will not be
    //     logged.
    //
    // Returns:
    //   Nothing
    ////////////////////////////////////////////////////////////////////////////
    Tracer(
        RFID_LOG_SEVERITY   severityThreshold = RFID_LOG_SEVERITY_FATAL
        ) :
        m_severityThreshold(severityThreshold)
    {
        // Intentionally left blank
    } // Tracer

    ////////////////////////////////////////////////////////////////////////////
    // Name: GetSeverityString
    //
    // Description:
    //   Returns a string for the severity level
    //
    // Parameters:
    //   severity - the severity level requesting string for
    //
    // Returns:
    //   A string that represents the severity
    ////////////////////////////////////////////////////////////////////////////
    static const char* GetSeverityString(
        RFID_LOG_SEVERITY   severity
        )
    {
        const char* severityString;

        switch(severity)
        {
            case RFID_LOG_SEVERITY_DEBUG:
                severityString = "DEBUG";
                break;
            case RFID_LOG_SEVERITY_INFO:
                severityString = "INFO";
                break;
            case RFID_LOG_SEVERITY_TRACE:
                severityString = "TRACE";
                break;
            case RFID_LOG_SEVERITY_WARNING:
                severityString = "WARNING";
                break;
            case RFID_LOG_SEVERITY_ERROR:
                severityString = "ERROR";
                break;
            default:
                severityString = "UNKNOWN";
                break;
        }

        return severityString;
    } // GetSeverityString

    ////////////////////////////////////////////////////////////////////////////
    // Name: LogMessage
    //
    // Description:
    //   A virtual function that is implemented by derived classes to do the
    //   actual logging of the message.
    //
    // Parameters:
    //   severity - the severity level for this message
    //   format - the format string for the message (uses printf format specifiers)
    //   arguments - the variable argument list for the format specifier
    //
    // Returns:
    //   Nothing
    ////////////////////////////////////////////////////////////////////////////
    virtual void LogMessage(
        RFID_LOG_SEVERITY   severity,
        const char*         format,
        va_list             arguments
        ) const = 0;

private:
    RFID_LOG_SEVERITY   m_severityThreshold;  // Minimum severity for logging
                                              // messages

    // Prevent copying of tracer objects
    Tracer(const Tracer&);
    const Tracer& operator = (const Tracer&);
}; // class Tracer

} // namespace rfid

#endif // TRACER_H_INCLUDED
