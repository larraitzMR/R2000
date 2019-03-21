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
 * $Id: rfid_error.h 66029 2010-11-11 17:41:32Z dshaheen $
 * 
 * Description:
 *     This is the RFID Library header file that specifies the RFID error codes.
 *     
 *
 *****************************************************************************
 */

#ifndef RFID_ERROR_H_INCLUDED
#define RFID_ERROR_H_INCLUDED

#include "rfid_types.h"

/******************************************************************************
 * Name: RFID_STATUS - RFID Library status and error codes
 ******************************************************************************/
enum
{
    /* Success                                                                */
    RFID_STATUS_OK,
    /* Attempted to open a radio that is already open                         */
    RFID_ERROR_ALREADY_OPEN     = -9999,                             /* -9999 */
    /* Buffer supplied is too small                                           */
    RFID_ERROR_BUFFER_TOO_SMALL,                                     /* -9998 */
    /* General failure                                                        */
    RFID_ERROR_FAILURE,                                              /* -9997 */
    /* Failed to load radio bus driver                                        */
    RFID_ERROR_DRIVER_LOAD,                                          /* -9996 */
    /* Library cannot use version of radio bus driver present on system       */
    RFID_ERROR_DRIVER_MISMATCH,                                      /* -9995 */
    /* This error code is no longer used, maintain slot in enum in case 
     * anyone is using hard-coded error codes for some reason                 */
    RFID_ERROR_RESERVED_01,                                          /* -9994 */
    /* Antenna number is invalid                                              */
    RFID_ERROR_INVALID_ANTENNA,                                      /* -9993 */
    /* Radio handle provided is invalid                                       */
    RFID_ERROR_INVALID_HANDLE,                                       /* -9992 */
    /* One of the parameters to the function is invalid                       */
    RFID_ERROR_INVALID_PARAMETER,                                    /* -9991 */
    /* Attempted to open a non-existent radio                                 */
    RFID_ERROR_NO_SUCH_RADIO,                                        /* -9990 */
    /* Library has not been successfully initialized                          */
    RFID_ERROR_NOT_INITIALIZED,                                      /* -9989 */
    /* Function not supported                                                 */
    RFID_ERROR_NOT_SUPPORTED,                                        /* -9988 */
    /* Operation was cancelled by call to cancel operation, close radio, or   */
    /* shut down the library                                                  */
    RFID_ERROR_OPERATION_CANCELLED,                                  /* -9987 */
    /* Library encountered an error allocating memory                         */
    RFID_ERROR_OUT_OF_MEMORY,                                        /* -9986 */
    /* The operation cannot be performed because the radio is currently busy  */
    RFID_ERROR_RADIO_BUSY,                                           /* -9985 */
    /* The underlying radio module encountered an error                       */
    RFID_ERROR_RADIO_FAILURE,                                        /* -9984 */
    /* The radio has been detached from the system                            */
    RFID_ERROR_RADIO_NOT_PRESENT,                                    /* -9983 */
    /* The RFID library function is not allowed at this time.                 */
    RFID_ERROR_CURRENTLY_NOT_ALLOWED,                                /* -9982 */
    /* The radio module's MAC firmware is not responding to requests.         */
    RFID_ERROR_RADIO_NOT_RESPONDING,                                 /* -9981 */
    /* The MAC firmware encountered an error while initiating the nonvolatile */
    /* memory update.  The MAC firmware will return to its normal idle state  */
    /* without resetting the radio module.                                    */
    RFID_ERROR_NONVOLATILE_INIT_FAILED,                              /* -9980 */
    /* An attempt was made to write data to an address that is not in the     */
    /* valid range of radio module nonvolatile memory addresses.              */
    RFID_ERROR_NONVOLATILE_OUT_OF_BOUNDS,                            /* -9979 */
    /* The MAC firmware encountered an error while trying to write to the     */
    /* radio module's nonvolatile memory region.                              */
    RFID_ERROR_NONVOLATILE_WRITE_FAILED,                             /* -9978 */
    /* The underlying transport layer detected that there was an overflow     */
    /* error resulting in one or more bytes of the incoming data being        */
    /* dropped.  The operation was aborted and all data in the pipeline was   */
    /* flushed.                                                               */
    RFID_ERROR_RECEIVE_OVERFLOW,                                     /* -9977 */
    /* An unexpected value was returned to this function by the MAC firmware  */
    RFID_ERROR_UNEXPECTED_VALUE,                                     /* -9976 */
    /* The MAC firmware encountered CRC errors while trying to                */
    /* write to the radio module's nonvolatile memory region.                 */
    RFID_ERROR_NONVOLATILE_CRC_FAILED,                               /* -9975 */
    /* The MAC firmware encountered unexpected values in the packet header    */
    RFID_ERROR_NONVOLATILE_PACKET_HEADER,                            /* -9974 */
    /* The MAC firmware received more than the specified maximum packet size  */
    RFID_ERROR_NONVOLATILE_MAX_PACKET_LENGTH                         /* -9973 */
};
typedef INT32S  RFID_STATUS;

#endif  /* #ifndef RFID_ERROR_H_INCLUDED */
