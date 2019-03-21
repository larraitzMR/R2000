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
 * $Id: rfid_exceptions.h 60027 2009-09-25 00:17:51Z dshaheen $
 * 
 * Description:
 *     This header presents the exception classes used in the RFID reader library
 *     
 *
 *****************************************************************************
 */

#ifndef RFID_EXCEPTIONS_H_INCLUDED
#define RFID_EXCEPTIONS_H_INCLUDED

#include <exception>
#include "rfid_error.h"
#include "rfid_extern.h"

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: RfidErrorException
//
// Description: This class is used to represent exceptions that convey an error
//   code as defined by RFID_STATUS.
////////////////////////////////////////////////////////////////////////////////
class RfidErrorException : public std::exception
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name:        RfidErrorException
    // Description: Initializes the RFID error exception object.
    // Parameters:  error - the error code
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    explicit RfidErrorException(
        RFID_STATUS error
        ) : 
        std::exception(),
        m_error(error),
        m_thrownBy(NULL)
    {
        /* if the tracer pointer has been setup, then go ahead and log the exception */
        if (g_pTracer.get() != 0)
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "######## Exception %ld was thrown\n",
                GetError()
                );
        }
    } // RfidErrorException


    ////////////////////////////////////////////////////////////////////////////
    // Name:        RfidErrorException
    // Description: Initializes the RFID error exception object and stores the 
    //              the error and message.
    // Parameters:  error - the error code
    //              thrownBy - name of the function that threw the exception
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    explicit RfidErrorException(
        RFID_STATUS error, 
        const char *thrownBy
        ) : 
        std::exception(),
        m_error(error),
        m_thrownBy(thrownBy)
    {
        /* if the tracer pointer has been setup, then go ahead and log the exception */
        if (g_pTracer.get() != 0)
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "######## Exception %ld thrown by %s\n",
                GetError(),
                GetThrownBy()
                );
        }
    } // RfidErrorException
    
    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetError
    // Description: Retrieves the RFID error status code
    // Parameters:  None
    // Returns:     The RFID error status code
    ////////////////////////////////////////////////////////////////////////////
    RFID_STATUS GetError() const
    {
        return m_error;
    } // GetError


    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetThrownBy
    // Description: Retrieves the name of the function that threw the exception
    //              if available, otherwise returns NULL
    // Parameters:  None
    // Returns:     The RFID error message
    ////////////////////////////////////////////////////////////////////////////
    const char *GetThrownBy() const
    {
        return m_thrownBy;
    } // GetThrownBy
    
private:
    // The RFID error status code associated with this exception
    RFID_STATUS m_error;

    // Name of the function that threw the exception, if available, otherwise NULL
    const char *m_thrownBy;

}; // class RfidErrorException

} // namespace rfid

#endif // #ifndef RFID_EXCEPTIONS_H_INCLUDED
