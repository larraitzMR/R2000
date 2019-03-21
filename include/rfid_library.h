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
 * $Id: rfid_library.h 65980 2010-11-09 23:05:40Z dshaheen $
 * 
 * Description:
 *     This is the main RFID Library header file and specifies the external
 *     interface of the library.
 *     
 *
 *****************************************************************************
 */

#ifndef RFID_LIBRARY_H_INCLUDED
#define RFID_LIBRARY_H_INCLUDED

#include "rfid_types.h"
#include "rfid_constants.h"
#include "rfid_structs.h"
#include "rfid_library_export.h"
#include "rfid_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Name: RFID_Startup
 *
 * Description:
 *   Allows the RFID Host Library to properly initialize any internal data 
 *   structures and put itself into a well-known ready state.  This function 
 *   must be called before calling any other RFID Host Library API functions.  
 *   If the interface has already been initialized, additional calls to 
 *   RFID_Startup have no effect. 
 *
 * Parameters:
 *   pLibraryVersion - pointer to structure that on return will contain the
 *     version of the library.  May be NULL if not required by application.
 *   flags - library startup flags.  There are no flags defined at this time.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_OUT_OF_MEMORY
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_Startup(
    RFID_VERSION*   pLibraryVersion, 
    INT32U          flags
    );

/******************************************************************************
 * Name: RFID_Shutdown
 *
 * Description:
 *   Allows the RFID Host Library to properly clean up any internally-held 
 *   resources.  To prevent resource leaks, an application should ensure that 
 *   the interface is shut down before the application exits.  All 
 *   previously-opened radios are closed and returned to the idle state (i.e., 
 *   any currently-executing tag-protocol operations are aborted and their 
 *   respective functions return RFID_ERROR_OPERATION_CANCELLED) and all 
 *   subsequent interface functions, with the exception of RFID_Startup, will 
 *   fail until the interface is initialized again. 
 *
 *   NOTE: This function must not be called from the packet-processing callback 
 *         function.
 *
 * Parameters:
 *   none
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INTIALIZED
 *   RFID_ERROR_FAILURE
 *   RFID_ERROR_CURRENTLY_NOT_ALLOWED
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_Shutdown(void);

/******************************************************************************
 * Name: RFID_RetrieveAttachedRadiosList
 *
 * Description:
 *   Retrieves the list of radio modules attached to the system.
 *
 * Parameters:
 *   pBuffer - pointer to a buffer into which attached radio information will
 *     be placed.  On input, pBuffer->totalLength must specify the length of
 *     the buffer.  If the buffer is not large enough, on return
 *     pBuffer->totalLength will contain the number of bytes required.
 *   flags - Reserved for future use.  Set to zero.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_BUFFER_TOO_SMALL
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RetrieveAttachedRadiosList(
    RFID_RADIO_ENUM*    pBuffer,
    INT32U              flags
    );

/******************************************************************************
 * Name: RFID_RadioOpen
 *
 * Description:
 *   Requests explicit control of a radio.
 *
 * Parameters:
 *   cookie - the unique cookie for the radio to open.  This cookie was returned
 *     in the RFID_RADIO_INFO structure that was returned from a call to
 *     RFID_RetrieveAttachedRadiosList.
 *   pHandle - a pointer to a radio handle that upon successful return will
 *     contain the handle the application will subsequently use to refer to the
 *     radio.
 *     Must not be NULL.
 *   flags - Reserved for future use.  Set to zero.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_NO_SUCH_RADIO
 *   RFID_ERROR_ALREADY_OPEN
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_DRIVER_MISMATCH
 *   RFID_ERROR_OUT_OF_MEMORY
 *   RFID_ERROR_RECEIVE_OVERFLOW
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioOpen(
    INT32U              cookie,
    RFID_RADIO_HANDLE*  pHandle,
    INT32U              flags
    );

/******************************************************************************
 * Name: RFID_RadioClose
 *
 * Description:
 *   Release control of a previously-opened radio.  On close, any currently-
 *   executing or outstanding requests are cancelled and the radio is returned
 *   to idle state.
 *
 *   NOTE: This function must not be called from the packet callback function.
 *
 * Parameters:
 *   handle - the handle to the radio to close.  This is the handle returned
 *     from a successful call to RFID_RadioOpen.  If the handle doesn't
 *     reference a currently-opened radio, function does nothing and returns
 *     success.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_CURRENTLY_NOT_ALLOWED
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioClose(
    RFID_RADIO_HANDLE   handle
    );

/******************************************************************************
 * Name: RFID_MacWriteRegister
 *
 * Description:
 *   Sets the low-level MAC register for the radio module.  Radio
 *   registers may not be set while a radio module is executing
 *   a tag-protocol operation.  Any valid MAC register may be written
 *   with the exception of the HST_CMD register.
 *
 * Parameters:
 *   handle - handle to radio for which low-level register is to
 *     be written.  This is the handle from a successful call to RFID_RadioOpen.
 *   address - the register address to write
 *   value - the value to which the register will be set
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacWriteRegister(
    RFID_RADIO_HANDLE   handle,
    INT16U              address,
    INT32U              value
    );

/******************************************************************************
 * Name: RFID_MacReadRegister
 *
 * Description:
 *   Retrieves a low-level radio module register.  Radio
 *   registers may not be retrieved while a radio module is
 *   executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which low-level register is to
 *     be retrieved.  This is the handle from a successful call to
 *     RFID_RadioOpen.
 *   address - address to retrieve
 *   pValue - pointer to variable that will receive register value.
 *     Must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacReadRegister(
    RFID_RADIO_HANDLE   handle,
    INT16U              address,
    INT32U*             pValue
    );

/******************************************************************************
 * Name: RFID_MacWriteBankedRegister
 *
 * Description:
 *   Sets the low-level banked register for the radio module.  Radio
 *   banked registers may not be set while a radio module is executing
 *   a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which low-level banked register is to
 *     be set.  This is the handle from a successful call to RFID_RadioOpen.
 *   address - address of the banked register to set
 *   bankSelector - the bank for which the register should be written
 *   value - the value to which the banked register will be set
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacWriteBankedRegister(
    RFID_RADIO_HANDLE   handle,
    INT16U              address,
    INT16U              bankSelector,
    INT32U              value
    );

/******************************************************************************
 * Name: RFID_MacReadBankedRegister
 *
 * Description:
 *   Retrieves a low-level radio module banked register.  Radio
 *   register may not be retrieved while a radio module is
 *   executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which low-level banked register is to
 *     be retrieved.  This is the handle from a successful call to
 *     RFID_RadioOpen.
 *   address - address of banked register to retrieve
 *   bankSelector - bank from which the register should be retrieved
 *   pValue - pointer to variable that will receive banked register
 *     value.  Must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE 
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacReadBankedRegister(
    RFID_RADIO_HANDLE   handle,
    INT16U              address,
    INT16U              bankSelector,
    INT32U*             pValue
    );





/******************************************************************************
 * Name: RFID_MacReadRegisterInfo
 *
 * Description:
 *   Retrieves a low-level radio module register info.  Radio
 *   register info may not be retrieved while a radio module is
 *   executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which low-level register info is to
 *     be retrieved.  This is the handle from a successful call to
 *     RFID_RadioOpen.
 *   address - register address which info is to be retrieved
 *   pInfo - pointer to structure containing register info.  Must
 *     not be NULL.  
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacReadRegisterInfo(
    RFID_RADIO_HANDLE                   handle,
    INT16U                              address,
    RFID_REGISTER_INFO*                 pInfo
    );

/******************************************************************************
 * Name: RFID_RadioSetOperationMode
 *
 * Description:
 *   Sets the radio's operation mode.  An RFID radio module’s operation mode
 *   will remain in effect until it is explicitly changed via
 *   RFID_RadioSetOperationMode.  The operation mode may not be set while a
 *   radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which operation mode will be set.  This is
 *     the handle from a successful call to RFID_RadioOpen.
 *   mode - the operation mode for the radio
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioSetOperationMode(
    RFID_RADIO_HANDLE           handle,
    RFID_RADIO_OPERATION_MODE   mode
    );

/******************************************************************************
 * Name: RFID_RadioGetOperationMode
 *
 * Description:
 *   Retrieves the radio's operation mode.  The operation mode may not be
 *   retrieved while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which operation mode is requested.  This is
 *     the handle from a successful call to RFID_RadioOpen.
 *   pMode - pointer to a variable that will on return will contain the
 *     current operation mode for the radio.  Must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioGetOperationMode(
    RFID_RADIO_HANDLE           handle,
    RFID_RADIO_OPERATION_MODE*  pMode
    );

/******************************************************************************
 * Name: RFID_RadioSetPowerState
 *
 * Description:
 *   Sets the radio module's power state (not to be confused with the antenna
 *   RF power).  The power state may not be set while a radio module is
 *   executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which power state is to be set.  This is the
 *     handle from a successful call to RFID_RadioOpen.
 *   state - the power state for the radio
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioSetPowerState(
    RFID_RADIO_HANDLE       handle,
    RFID_RADIO_POWER_STATE  state
    );

/******************************************************************************
 * Name: RFID_RadioGetPowerState
 *
 * Description:
 *   Retrieves the radio module's power state (not to be confused with the
 *   antenna RF power).  The power state may not be retrieved while a radio
 *   module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle for which power state is requested.  This is the handle
 *     from a successful call to RFID_RadioOpen.
 *   pState - a pointer to a variable that on return will contain the radio
 *     module's power state.  Must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioGetPowerState(
    RFID_RADIO_HANDLE       handle,
    RFID_RADIO_POWER_STATE* pState
    );

/******************************************************************************
 * Name: RFID_RadioSetCurrentLinkProfile
 *
 * Description:
 *   Sets the current link profile for the radio module.  The curren link
 *   profile may not be set while a radio module is executing a tag-protocol
 *   operation.
 *
 * Parameters:
 *   handle - handle to radio for which current link profile is to be set.  This
 *     is the handle from a successful call to RFID_RadioOpen.
 *   profile - the profile to make the current one
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioSetCurrentLinkProfile(
    RFID_RADIO_HANDLE   handle,
    INT32U              profile
    );

/******************************************************************************
 * Name: RFID_RadioGetCurrentLinkProfile
 *
 * Description:
 *   Retrieves the current link profile for the radio module.  The current link
 *   profile may not be retrieved while a radio module is executing a tag-
 *   protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which current link profile is to be retrieved.
 *     This is the handle from a successful call to RFID_RadioOpen.
 *   pCurrentProfile - a pointer to an unsigned 32-bit integer that will receive
 *     the current profile.  Must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioGetCurrentLinkProfile(
    RFID_RADIO_HANDLE   handle,
    INT32U*             pCurrentProfile
    );

/******************************************************************************
 * Name: RFID_RadioGetLinkProfile
 *
 * Description:
 *   Retrieves the information for the specified link profile for the radio
 *   module.  A link profile may not be retrieved while a radio module is
 *   executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which link profile information is to be
 *     retrieved.  This is the handle from a successful call to RFID_RadioOpen.
 *   profile - the link profile for which information will be retrieved
 *   pProfileInfo - a pointer to a structure that will be filled in with link
 *     profile information.  Must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioGetLinkProfile(
    RFID_RADIO_HANDLE           handle,
    INT32U                      profile,
    RFID_RADIO_LINK_PROFILE*    pProfileInfo
    );

/******************************************************************************
 * Name: RFID_RadioWriteLinkProfileRegister
 *
 * Description:
 *   Writes a value to a link-profile register for the specified link
 *   profile.  The link profile register changeswill not take effect until 
 *   the application calls RFID_RadioSetCurrentLinkProfile and selects the 
 *   modified profile.  A link-profile regsiter may not be written while a 
 *   radio module is executing a tag-protocol operation.  
 *
 * Parameters:
 *   handle - handle to radio for which link-profile register should be written
 *   profile - the link profile for link-profile register should be written
 *   address - address of the link-profile register
 *   value - the value to be written to the register
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioWriteLinkProfileRegister(
    RFID_RADIO_HANDLE   handle,
    INT32U              profile,
    INT16U              address,
    INT16U              value
    );

/******************************************************************************
 * Name: RFID_RadioReadLinkProfileRegister
 *
 * Description:
 *   Retrieves the contents of a link-profile register for the specified link
 *   profile.  A link-profile register may not be read while a radio module is
 *   executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which link-profile register should be read
 *   profile - the link profile for link-profile register should be read
 *   address - address of the link-profile register
 *   pValue - a pointer to a 16-bit unsigned integer that upon return will
 *     contain the register's value
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioReadLinkProfileRegister(
    RFID_RADIO_HANDLE   handle,
    INT32U              profile,
    INT16U              address,
    INT16U*             pValue
    );

/******************************************************************************
 * Name: RFID_AntennaPortGetStatus
 *
 * Description:
 *   Retrieves the status of a radio module's antenna port.  The antenna port
 *   status may not be retrieved while a radio module is executing a tag-
 *   protocol operation.
 *
 * Parameters:
 *   handle - handle to the radio for which antenna status is requested.  This
 *     is the handle from a successful call to RFID_RadioOpen.
 *   antennaPort - the antenna port for which status is to be retrieved. 
 *     Antenna ports are numbered beginning with 0.
 *   pStatus - pointer to the structure which upon return will contain the
 *     antenna port's status.  Must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_AntennaPortGetStatus(
    RFID_RADIO_HANDLE           handle,
    INT32U                      antennaPort,
    RFID_ANTENNA_PORT_STATUS*   pStatus
    );

/******************************************************************************
 * Name: RFID_AntennaPortSetState
 *
 * Description:
 *   Sets the state of a radio module's antenna port.  The antenna port state
 *   may not be set while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to the radio for which antenna port state will be set.
 *     This is the handle from a successful call to RFID_RadioOpen.
 *   antennaPort - the antenna port for which state will be set.  Antenna ports
 *     are numbered beginning with 0.
 *   state - the state for the antenna port
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_AntennaPortSetState(
    RFID_RADIO_HANDLE       handle,
    INT32U                  antennaPort,
    RFID_ANTENNA_PORT_STATE state
    );

/******************************************************************************
 * Name: RFID_AntennaPortSetConfiguration
 *
 * Description:
 *   Sets the configuration for a radio module's antenna port.  The antenna
 *   port configuration may not be set while a radio module is executing a
 *   tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which antenna-port configuration will be set.
 *     This is the handle from a successful call to RFID_RadioOpen.
 *   antennaPort - the antenna port for which configuration will be set.
 *     Antenna ports are numbered beginning with 0.
 *   pConfig - pointer to structure containing antenna port configuration.  Must
 *     not be NULL.  
 *     In version priort to 2.3, the physicalRxPort and physicalTxPort
 *     fields must be the same.  
 *     As of version 2.3.0, the physicalRxPort has  been deprecated and should be 
 *     set to 0.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_AntennaPortSetConfiguration(
    RFID_RADIO_HANDLE               handle,
    INT32U                          antennaPort,
    const RFID_ANTENNA_PORT_CONFIG* pConfig
    );

/******************************************************************************
 * Name: RFID_AntennaPortGetConfiguration
 *
 * Description:
 *   Retrieves the configuration for a radio module's antenna port.  The antenna
 *   port configuration may not be retrieved while a radio module is executing a
 *   tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which antenna port configuration will be
 *     retrieved.  This is the handle from a successful call to RFID_RadioOpen.
 *   antennaPort - the antenna port for which configuration will be
 *     retrieved.  Antenna ports are numbered beginning with 0.
 *   pConfig - pointer to structure that on return will contain the antenna
 *     port configuration.  Must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_AntennaPortGetConfiguration(
    RFID_RADIO_HANDLE           handle,
    INT32U                      antennaPort,
    RFID_ANTENNA_PORT_CONFIG*   pConfig
    );

/******************************************************************************
 * Name: RFID_18K6CSetSelectCriteria
 *
 * Description:
 *   Configures the tag-selection criteria for the ISO 18000-6C select command.
 *   The supplied tag-selection criteria will be used for any tag-protocol
 *   operations (i.e., RFID_18K6CTagInventory, etc.) in which the application
 *   specifies that an ISO 18000-6C select command should be issued prior to
 *   executing the tag-protocol operation.  The tag-selection criteria will
 *   stay in effect until the next call to RFID_18K6CSetSelectCriteria.  The
 *   select criteria may not be set while a radio module is executing a tag-
 *   protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which tag-selection criteria will be
 *     configured.  This is the handle from a successful call to RFID_RadioOpen.
 *   pCriteria - pointer to a structure that specifies the ISO 18000-6C
 *     tag-selection criteria.  This parameter must not be NULL.
 *   flags - Reserved for future use.  Set to zero.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CSetSelectCriteria(
    RFID_RADIO_HANDLE                   handle,
    const RFID_18K6C_SELECT_CRITERIA*   pCriteria,
    INT32U                              flags
    );

/******************************************************************************
 * Name: RFID_18K6CGetSelectCriteria
 *
 * Description:
 *   Retrieves the configured tag-selection criteria for the ISO 18000-6C select
 *   command.  The returned tag-selection criteria are used for any tag-protocol
 *   operations (i.e., RFID_18K6CTagInventory, etc.) in which the application
 *   specifies that an ISO 18000-6C select command should be issued prior to 
 *   executing the tag-protocol operation.  The select criteria may not be
 *   retrieved while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which tag-selection criteria will be
 *     retrieved.  This is the handle from a successful call to RFID_RadioOpen.
 *   pCriteria - pointer to a structure that specifies the ISO 18000-6C
 *     tag-selection criteria.  On entry to the function, the countCriteria 
 *     field must contain the number of entries in the array pointed to by the
 *     pCriteria field.  On return from the function, the countCriteria field
 *     will contain the number of tag-selection criteria returned in the array
 *     pointed to by pCriteria.  If the array pointed to by pCriteria is not
 *     large enough to hold the configured tag-selection criteria, on return
 *     countCriteria will contain the number of entries required and the
 *     function will return RFID_ERROR_BUFFER_TOO_SMALL.  This parameter must
 *     not be NULL.  
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_BUFFER_TOO_SMALL
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CGetSelectCriteria(
    RFID_RADIO_HANDLE           handle,
    RFID_18K6C_SELECT_CRITERIA* pCriteria
    );

/******************************************************************************
 * Name: RFID_18K6CSetPostMatchCriteria
 *
 * Description:
 *   Configures the post-singulation match criteria to be used by the RFID
 *   radio module.  The supplied post-singulation match criteria will be used
 *   for any tag-protocol operations (i.e., RFID_18K6CTagInventory, etc.) in
 *   which the application specifies that a post-singulation match should be
 *   performed on the tags that are singulated by the tag-protocol operation.
 *   The post-singulation match criteria will stay in effect until the next call
 *   to RFID_18K6CSetPostMatchCriteria.  The post-singulation match criteria
 *   may not be set while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which post-singulation match criteria will be
 *     configured.  This is the handle from a successful call to RFID_RadioOpen.
 *   pParms - a pointer to a structure that specifies the post-singulation
 *     match mask and disposition that are to be applied to the tag’s Electronic
 *     Product Code after it is singulated to determine if it is to have the
 *     tag-protocol operation applied to it.  This parameter must not be NULL.
 *   flags - Reserved for future use.  Set to zero.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CSetPostMatchCriteria(
    RFID_RADIO_HANDLE                       handle,
    const RFID_18K6C_SINGULATION_CRITERIA*  pParms,
    INT32U                                  flags
    );

/******************************************************************************
 * Name: RFID_18K6CGetPostMatchCriteria
 *
 * Description:
 *   Retrieves the configured post-singulation match criteria to be used by the
 *   RFID radio module.  The post-singulation match criteria is used for any
 *   tag-protocol operations (i.e., RFID_18K6CTagInventory, etc.) in which the
 *   application specifies that a post-singulation match should be performed on
 *   the tags that are singulated by the tag-protocol operation.  Post-
 *   singulation match criteria may not be retrieved while a radio module is
 *   executing a tag-protocol operation.  The post-singulation match criteria
 *   may not be retrieved while a radio module is executing a tag-protocol
 *   operation.
 *
 * Parameters:
 *   handle - handle to radio upon which post-singulation match criteria will be
 *     retrieved.  This is the handle from a successful call to RFID_RadioOpen.
 *   pParms - a pointer to a structure that upon return will contain the
 *     post-singulation match criteria that are to be applied to the tag’s
 *     Electronic Product Code after it is singulated to determine if it is to
 *     have the tag-protocol operation applied to it.  On entry to the function,
 *     the countCriteria field must contain the number of entries in the array
 *     pointed to by the pCriteria field.  On return from the function, the
 *     countCriteria field will contain the number of post-singulation match
 *     criteria returned in the array pointed to by pCriteria.  If the array
 *     pointed to by pCriteria is not large enough to hold the configured tag-
 *     selection criteria, on return countCriteria will contain the number of
 *     entries required and the function will return
 *     RFID_ERROR_BUFFER_TOO_SMALL.  This parameter must not be NULL.  The
 *     pCriteria field may be NULL only if the countCriteria field is zero.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_BUFFER_TOO_SMALL
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CGetPostMatchCriteria(
    RFID_RADIO_HANDLE                   handle,
    RFID_18K6C_SINGULATION_CRITERIA*    pParms
    );

/******************************************************************************
 * Name: RFID_18K6CSetQueryTagGroup
 *
 * Description:
 *   Specifies which tag group will have subsequent tag-protocol operations
 *   (e.g., inventory, tag read, etc.) applied to it.  The tag group may not be
 *   changed while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which the tag group will be configured.
 *     This is the handle from a successful call to RFID_RadioOpen.
 *   pGroup - a pointer to a structure that specifies the tag group that will
 *     have subsequent tag-protocol operations applied to it.  This parameter
 *     must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE 
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CSetQueryTagGroup(
    RFID_RADIO_HANDLE           handle,
    const RFID_18K6C_TAG_GROUP* pGroup
    );

/******************************************************************************
 * Name: RFID_18K6CGetQueryTagGroup
 *
 * Description:
 *   Retrieves the tag group that will have subsequent tag-protocol operations
 *   (e.g., inventory, tag read, etc.) applied to it.  The tag group may not be
 *   retrieved while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which the tag group will be retrieved.
 *     This is the handle from a successful call to RFID_RadioOpen.
 *   pGroup - a pointer to a structure that upon return contains the configured
 *     tag group.  This parameter must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CGetQueryTagGroup(
    RFID_RADIO_HANDLE       handle,
    RFID_18K6C_TAG_GROUP*   pGroup
    );

/******************************************************************************
 * Name: RFID_18K6CSetCurrentSingulationAlgorithm
 *
 * Description:
 *   Allows the application to set the currently-active singulation algorithm
 *   (i.e., the one that is used when performing a tag-protocol operation
 *   (e.g., inventory, tag read, etc.)).  The currently-active singulation
 *   algorithm may not be changed while a radio module is executing a tag-
 *   protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which the currently-active singulation
 *     algorithm will be set.  This is the handle from a successful call to
 *     RFID_RadioOpen.
 *   algorithm - the singulation algorithm to make currently active.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CSetCurrentSingulationAlgorithm(
    RFID_RADIO_HANDLE                   handle,
    RFID_18K6C_SINGULATION_ALGORITHM    algorithm
    );

/******************************************************************************
 * Name: RFID_18K6CGetCurrentSingulationAlgorithm
 *
 * Description:
 *   Allows the application to retrieve the currently-active singulation
 *   algorithm.  The currently-active singulation algorithm may not be changed
 *   while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which the currently-active singulation
 *     algorithm will be retrieved.  This is the handle from a successful call
 *     to RFID_RadioOpen.
 *   pAlgorithm - a pointer to a singulation-algorithm variable that upon
 *     return will contain the currently-active singulation algorithm.  This
 *     parameter must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CGetCurrentSingulationAlgorithm(
    RFID_RADIO_HANDLE                   handle,
    RFID_18K6C_SINGULATION_ALGORITHM*   pAlgorithm
    );

/******************************************************************************
 * Name: RFID_18K6CSetSingulationAlgorithmParameters
 *
 * Description:
 *   Allows the application to configure the settings for a particular
 *   singulation algorithm.  A singulation algorithm may not be configured while
 *   a radio module is executing a tag-protocol operation.
 *
 *   NOTE:  Configuring a singulation algorithm does not automatically set it as
 *   the current singulation algorithm
 *   (see RFID_18K6CSetCurrentSingulationAlgorithm). 
 *
 * Parameters:
 *   handle - handle to radio upon which the singulation-algorithm parameters
 *     will be set.  This is the handle from a successful call to
 *     RFID_RadioOpen.
 *   algorithm - the singulation algorithm to be configured.  This parameter
 *     determines the type of structure to which pParms points.  For example,
 *     if this parameter is RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ, pParms must
 *     point to a RFID_18K6C_SINGULATION_FIXEDQ_PARMS structure.  If this
 *     parameter does not represent a valid singulation algorithm,
 *     RFID_ERROR_INVALID_PARAMETER is returned.
 *   pParms - a pointer to a structure that contains the singulation-algorithm
 *     parameters.  The type of structure this points to is determined by
 *     algorithm.  The structure’s length field must be filled in appropriately.
 *     This parameter must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CSetSingulationAlgorithmParameters(
    RFID_RADIO_HANDLE                   handle,
    RFID_18K6C_SINGULATION_ALGORITHM    algorithm,
    const void*                         pParms
    );

/******************************************************************************
 * Name: RFID_18K6CGetSingulationAlgorithmParameters
 *
 * Description:
 *   Allows the application to retrieve the settings for a particular
 *   singulation algorithm.  Singulation-algorithm parameters may not be
 *   retrieved while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which the singulation-algorithm parameters
 *     will be retrieved.  This is the handle from a successful call to
 *     RFID_RadioOpen.
 *   algorithm - The singulation algorithm for which parameters are to be
 *     retrieved.  This parameter determines the type of structure to which
 *     pParms points.  For example, if this parameter is
 *     RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ, pParms must point to a
 *     RFID_18K6C_SINGULATION_FIXEDQ_PARMS structure.  If this parameter does
 *     not represent a valid singulation algorithm,
 *     RFID_ERROR_INVALID_PARAMETER is returned.
 *   pParms - a pointer to a structure that upon return will contain the
 *     singulation-algorithm parameters.  The type of structure this points to
 *     is determined by algorithm.  The structure’s length field must be filled
 *     in appropriately.  This parameter must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CGetSingulationAlgorithmParameters(
    RFID_RADIO_HANDLE                   handle,
    RFID_18K6C_SINGULATION_ALGORITHM    algorithm,
    void*                               pParms
    );



/******************************************************************************
 * Name: RFID_18K6CTagInventory
 *
 * Description:
 *   Executes a tag inventory for the tags of interest.  If the
 *   RFID_FLAG_PERFORM_SELECT flag is specified, the tag population is
 *   partitioned (i.e., ISO 18000-6C select) prior to the inventory operation.
 *   If the RFID_FLAG_PERFORM_POST_MATCH flag is specified, the post-singulation
 *   match mask is applied to a singulated tag's EPC to determine if the tag
 *   will be returned to the application.  The operation-response packets
 *   will be returned to the application via the application-supplied callback
 *   function.  An application may prematurely stop an inventory operation by
 *   calling RFID_Radio{Cancel|Abort}Operation on another thread or by returning
 *   a non-zero value from the callback function.  A tag inventory may not be
 *   issued while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which inventory operation will be performed.
 *     This is the handle from a successful call to RFID_RadioOpen.
 *   pParms - pointer to a structure that specifies the ISO 18000-6C inventory
 *     operation parameters.  This parameter must not be NULL.
 *   flags - inventory flags.  May be zero or a combination of the following:
 *     RFID_FLAG_PERFORM_SELECT - perform one or more selects before performing
 *       the inventory.
 *     RFID_FLAG_PERFORM_POST_MATCH - perform post-singulation mask match on
 *       singulated tags.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *   RFID_ERROR_OPERATION_CANCELLED
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagInventory(
    RFID_RADIO_HANDLE                   handle,
    const RFID_18K6C_INVENTORY_PARMS*   pParms,
    INT32U                              flags
    );

/******************************************************************************
 * Name: RFID_18K6CTagRead
 *
 * Description:
 *   Executes a tag read for the tags of interest.  If the
 *   RFID_FLAG_PERFORM_SELECT flag is specified, the tag population is
 *   partitioned (i.e., ISO 18000-6C select) prior to the tag-read operation.
 *   If the RFID_FLAG_PERFORM_POST_MATCH flag is specified, the post-singulation
 *   match mask is applied to a singulated tag's EPC to determine if the tag
 *   will be read from.  Reads may only be performed on 16-bit word boundaries
 *   and for multiples of 16-bit words.  If one or more of the memory words
 *   specified by the offset/count combination do not exist or are read-locked,
 *   the read from the tag will fail and this failure will be reported through
 *   the operation response packet.    The operation-response packets will
 *   be returned to the application via the application-supplied callback
 *   function.  Each tag-read record is grouped with its corresponding tag-
 *   inventory record.  An application may prematurely stop a read operation by
 *   calling RFID_Radio{Cancel|Abort}Operation on another thread or by returning
 *   a non-zero value from the callback function.  A tag read may not be
 *   issued while a radio module is executing a tag-protocol operation.
 *
 *   Note that read should not be confused with inventory.  A read allows for
 *   reading a sequence of one or more 16-bit words starting from an arbitrary
 *   16-bit location in any of the tag's memory banks.
 *
 * Parameters:
 *   handle - handle to radio upon which the read operation will be performed.
 *     This is the handle from a successful call to RFID_RadioOpen.
 *   pParms - pointer to a structure that specifies the ISO 18000-6C tag-read
 *     operation parameters.  This parameter must not be NULL.
 *   flags - read flags.  May be zero or a combination of the following:
 *     RFID_FLAG_PERFORM_SELECT - perform one or more selects before performing
 *       the read.
 *     RFID_FLAG_PERFORM_POST_MATCH - perform post-singulation mask match on
 *       singulated tags.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *   RFID_ERROR_OPERATION_CANCELLED
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagRead(
    RFID_RADIO_HANDLE               handle,
    const RFID_18K6C_READ_PARMS*    pParms,
    INT32U                          flags
    );

/******************************************************************************
 * Name: RFID_18K6CTagWrite
 *
 * Description:
 *   Executes a tag write for the tags of interest.  If the
 *   RFID_FLAG_PERFORM_SELECT flag is specified, the tag population is
 *   partitioned (i.e., ISO 18000-6C select) prior to the tag-write operation.
 *   If the RFID_FLAG_PERFORM_POST_MATCH flag is specified, the post-singulation
 *   match mask is applied to a singulated tag's EPC to determine if the tag
 *   will be written to.  Writes may only be performed on 16-bit word boundaries
 *   and for multiples of 16-bit words.  If one or more of the specified memory
 *   words do not exist or are write-locked, the write to the tag will fail and
 *   this failure will be reported through the operation-response packet.  The
 *   operation-response packets will be returned to the application via
 *   the application-supplied callback function.  Each tag-write record is
 *   grouped with its corresponding tag-inventory record.  An application may
 *   prematurely stop a write operation by calling
 *   RFID_Radio{Cancel|Abort}Operation on another thread or by returning a non-
 *   zero value from the callback function.  A tag write may not be
 *   issued while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which the write operation will be performed.
 *     This is the handle from a successful call to RFID_RadioOpen.
 *   pParms - pointer to a structure that specifies the ISO 18000-6C tag-write
 *     operation parameters.  This parameter must not be NULL.
 *   flags - write flags.  May be zero or a combination of the following:
 *     RFID_FLAG_PERFORM_SELECT - perform one or more selects before performing
 *       the write.
 *     RFID_FLAG_PERFORM_POST_MATCH - perform post-singulation mask match on
 *       singulated tags.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *   RFID_ERROR_OPERATION_CANCELLED
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagWrite(
    RFID_RADIO_HANDLE               handle,
    const RFID_18K6C_WRITE_PARMS*   pParms,
    INT32U                          flags 
    );

/******************************************************************************
 * Name: RFID_18K6CTagKill
 *
 * Description:
 *   Executes a tag kill for the tags of interest.  If the
 *   RFID_FLAG_PERFORM_SELECT flag is specified, the tag population is
 *   partitioned (i.e., ISO 18000-6C select) prior to the tag-kill operation.
 *   If the RFID_FLAG_PERFORM_POST_MATCH flag is specified, the post-singulation
 *   match mask is applied to a singulated tag's EPC to determine if the tag
 *   will be killed.  The operation-response packets will be returned to the
 *   application via the application-supplied callback function.  Each tag-kill
 *   record is grouped with its corresponding tag-inventory record.  An
 *   application may prematurely stop a kill operation by calling
 *   RFID_Radio{Cancel|Abort}Operation on another thread or by returning a non-
 *   zerovalue from the callback function.  A tag kill may not be
 *   issued while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which the kill operation will be performed.
 *     This is the handle from a successful call to RFID_RadioOpen.
 *   pParms - pointer to a structure that specifies the ISO 18000-6C tag-kill
 *     operation parameters.  This parameter must not be NULL.
 *   flags - kill flags.  May be zero or a combination of the following:
 *     RFID_FLAG_PERFORM_SELECT - perform one or more selects before performing
 *       the kill.
 *     RFID_FLAG_PERFORM_POST_MATCH - perform post-singulation mask match on
 *       singulated tags.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *   RFID_ERROR_OPERATION_CANCELLED
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagKill(
    RFID_RADIO_HANDLE               handle,
    const RFID_18K6C_KILL_PARMS*    pParms,
    INT32U                          flags
    );

/******************************************************************************
 * Name: RFID_18K6CTagLock
 *
 * Description:
 *   Executes a tag lock for the tags of interest.  If the
 *   RFID_FLAG_PERFORM_SELECT flag is specified, the tag population is
 *   partitioned (i.e., ISO 18000-6C select) prior to the tag-lock operation.
 *   If the RFID_FLAG_PERFORM_POST_MATCH flag is specified, the post-singulation
 *   match mask is applied to a singulated tag's EPC to determine if the tag
 *   will be locked.  The operation-response packets will be returned to the
 *   application via the application-supplied callback function.  Each tag-lock
 *   record is grouped with its corresponding tag-inventory record.  An
 *   application may prematurely stop a lock operation by calling
 *   RFID_Radio{Cancel|Abort}Operation on another thread or by returning a non-
 *   zero value from the callback function.  A tag lock may not be
 *   issued while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which the lock operation will be performed.
 *     This is the handle from a successful call to RFID_RadioOpen.
 *   pParms - pointer to a structure that specifies the ISO 18000-6C tag-lock
 *     operation parameters.  This parameter must not be NULL.
 *   flags - lock flags.  May be zero or a combination of the following:
 *     RFID_FLAG_PERFORM_SELECT - perform one or more selects before performing
 *       the lock.
 *     RFID_FLAG_PERFORM_POST_MATCH - perform post-singulation mask match on
 *       singulated tags.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *   RFID_ERROR_OPERATION_CANCELLED
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagLock(
    RFID_RADIO_HANDLE               handle,
    const RFID_18K6C_LOCK_PARMS*    pParms,
    INT32U                          flags
    );

/******************************************************************************
 * Name: RFID_18K6CBlockWrite
 *
 * Description:
 *   Executes a block write for the tags of interest.  If the
 *   RFID_FLAG_PERFORM_SELECT flag is specified, the tag population is
 *   partitioned (i.e., ISO 18000-6C select) prior to the block-write operation.
 *   If the RFID_FLAG_PERFORM_POST_MATCH flag is specified, the post-singulation
 *   match mask is applied to a singulated tag's EPC to determine if the tag
 *   will be written to.  Writes may only be performed on 16-bit word boundaries
 *   and for multiples of 16-bit words.  If one or more of the specified memory
 *   words do not exist or are write-locked, the write to the tag will fail and
 *   this failure will be reported through the operation-response packet.  The
 *   operation-response packets will be returned to the application via
 *   the application-supplied callback function.  An application may
 *   prematurely stop a write operation by calling
 *   RFID_Radio{Cancel|Abort}Operation on another thread or by returning a non-
 *   zero value from the callback function.  A block write may not be
 *   issued while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which the write operation will be performed.
 *     This is the handle from a successful call to RFID_RadioOpen.
 *   pBParms - pointer to a structure that specifies the ISO 18000-6C block-write
 *     operation parameters.  This parameter must not be NULL.
 *   flags - write flags.  May be zero or a combination of the following:
 *     RFID_FLAG_PERFORM_SELECT - perform one or more selects before performing
 *       the block write.
 *     RFID_FLAG_PERFORM_POST_MATCH - perform post-singulation mask match on
 *       singulated tags.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *   RFID_ERROR_OPERATION_CANCELLED
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagBlockWrite(
    RFID_RADIO_HANDLE                     handle,
    const RFID_18K6C_BLOCK_WRITE_PARMS*   pBWParms,
    INT32U                                flags 
    );


/******************************************************************************
 * Name: RFID_18K6CTagBlockErase
 *
 * Description:
 *   Executes a tag block erase for the tags of interest.  If the
 *   RFID_FLAG_PERFORM_SELECT flag is specified, the tag population is
 *   partitioned (i.e., ISO 18000-6C select) prior to the tag-read operation.
 *   If the RFID_FLAG_PERFORM_POST_MATCH flag is specified, the post-singulation
 *   match mask is applied to a singulated tag's EPC to determine if the tag 
 *   memory will be erased.  Block erase may only be performed on 16-bit word 
 *   boundaries and for multiples of 16-bit words.  If one or more of the memory 
 *   words specified by the offset/count combination do not exist or are 
 *   write-locked, the block erase of the tag will fail and this failure will 
 *   be reported through the operation response packet.    The 
 *   operation-response packets will be returned to the application via the 
 *   application-supplied callback function.  An application may 
 *   prematurely stop a block erase operation by calling 
 *   RFID_Radio{Cancel|Abort}Operation on another thread or by returning
 *   a non-zero value from the callback function.
 *
 * Parameters:
 *   handle - handle to radio upon which the block erase operation will be 
 *     performed.  This is the handle from a successful call to RFID_RadioOpen.
 *   pParms - pointer to a structure that specifies the ISO 18000-6C 
 *     tag-blockerase operation parameters.  This parameter must not be NULL.
 *   flags - read flags.  May be zero or a combination of the following:
 *     RFID_FLAG_PERFORM_SELECT - perform one or more selects before performing
 *       the block erase.
 *     RFID_FLAG_PERFORM_POST_MATCH - perform post-singulation mask match on
 *       singulated tags.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *   RFID_ERROR_OPERATION_CANCELLED
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagBlockErase(
    RFID_RADIO_HANDLE                      handle,
    const RFID_18K6C_BLOCK_ERASE_PARMS*    pParms,
    INT32U                                 flags
    );

/******************************************************************************
 * Name: RFID_18K6CTagQT
 *
 * Description:
 *   Executes a tag QT command with actions to perform upon the tag QT control 
 *   word, optionally performs a Read or Write(s) of one or more 16-bit words 
 *   for all tags of interest.  If the RFID_FLAG_PERFORM_SELECT flag is 
 *   specified, the tag population will be partitioned (i.e., one or more ISO 
 *   18000-6C select commands are issued) before the tag-QT operation.  If the
 *   RFID_FLAG_PERFORM_POST_MATCH flag is specified, a singulated tag's EPC 
 *   will be matched against post-singulation match mask to determine if QT is 
 *   to be performed on the tag. Refer to the tag read and write section for 
 *   the optionally performed Read or Write(s) commands for QT.  Failures will 
 *   be reported through the operation-response packet.  The function will 
 *   return the operation-response packets via the application-supplied 
 *   callback function.  The function will return when the tag-QT operation has 
 *   completed or has been cancelled.  An application may prematurely stop a 
 *   tag-QT operation by calling RFID_RadioCancelOperation or 
 *   RFID_RadioAbortOperation on another thread or by returning a non-zero 
 *   value from the callback function.  A tag QT operation may not be requested 
 *   while a radio module is executing a tag-protocol operation.
 * 
 *   NOTE:  One set of common tag-protocol operation parameters are available. 
 *          So, for example, the pointer to the callback function that the 
 *          interface will invoke with the resulting operation-response packets 
 *          will be the same for all access responses resulting from calling the 
 *          QT API. It is the applications responsibility to associate responses 
 *          (e.g. via the command code within the tag-access response packet) 
 *          with the access commands requested. Similarly for the other common 
 *          tag-protocol operation parameters.
 * 
 *   NOTE:  When performing an ISO 18000-6C tag-QT operation, the radio module 
 *          will only use the first enabled logical antenna (i.e., the enabled 
 *          logical antenna with the smallest logical antenna port number).
 * 
 *   NOTE:  When performing an ISO 18000-6C tag-QT operation, the radio module 
 *          always uses the fixed-Q singulation algorithm with the most-recent 
 *          settings for that algorithm.  If an application has never called 
 *          RFID_18K6CSetQueryTagGroup or RFID_18K6CSetSingulationAlgorithm 
 *          and specified the fixed-Q singulation algorithm, the radio module 
 *          will use the power-up defaults for the fixed-Q singulation 
 *          algorithm.
 * Parameters:
 *   handle - handle to radio upon which the write operation will be performed.
 *     This is the handle from a successful call to RFID_RadioOpen.
 *   pParms - pointer to a structure that specifies the ISO 18000-6C QT
 *     operation parameters.  This parameter must not be NULL.
 *   flags - write flags.  May be zero or a combination of the following:
 *     RFID_FLAG_PERFORM_SELECT - perform one or more selects before performing
 *       the write.
 *     RFID_FLAG_PERFORM_POST_MATCH - perform post-singulation mask match on
 *       singulated tags.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *   RFID_ERROR_OPERATION_CANCELLED
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagQT(
    RFID_RADIO_HANDLE               handle,
    const RFID_18K6C_QT_PARMS*      pParms,
    INT32U                          flags 
    );


/******************************************************************************
 * Name: RFID_RadioCancelOperation
 *
 * Description:
 *   Stops a currently-executing tag-protocol operation on a radio module.  The
 *   packet callback function will be executed until the command-end packet is
 *   received from the MAC or the packet callback returns a non-zero result.
 *
 *   NOTE: This function must not be called from the packet callback function.
 *
 * Parameters:
 *   handle - the handle to the radio for which the tag-protocol operation
 *     should be cancelled.  This is the handle from a successful call to
 *     RFID_RadioOpen.
 *   flags - Reserved for future use.  Set to zero.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_CURRENTLY_NOT_ALLOWED
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioCancelOperation(
    RFID_RADIO_HANDLE   handle,
    INT32U              flags
    );

/******************************************************************************
 * Name: RFID_RadioAbortOperation
 *
 * Description:
 *   Stops a currently-executing tag-protocol operation on a radio module and
 *   discards all remaining command-reponse packets.
 *
 *   NOTE: This function must not be called from the packet callback function.
 *
 * Parameters:
 *   handle - the handle to the radio for which the tag-protocol operation
 *     should be aborted.  This is the handle from a successful call to
 *     RFID_RadioOpen.
 *   flags - Reserved for future use.  Set to zero.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_CURRENTLY_NOT_ALLOWED
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioAbortOperation(
    RFID_RADIO_HANDLE   handle,
    INT32U              flags
    );

/******************************************************************************
 * Name: RFID_RadioSetResponseDataMode
 *
 * Description:
 *   Sets the operation response data reporting mode for tag-protocol
 *   operations.  By default, the reporting mode is set to "compact".  The 
 *   reporting mode will remain in effect until a subsequent call to
 *   RFID_RadioSetResponseDataMode.  The mode may not be changed while the
 *   radio is executing a tag-protocol operation.  
 *
 * Parameters:
 *   handle - the handle to the radio for which the operation response data
 *     reporting mode is to be set.  This is the handle from a successful call
 *     to RFID_RadioOpen.
 *   responseType - the type of data that will have its reporting mode set.  For
 *     version 1.0 of the library, the only valid value is
 *     RFID_RESPONSE_TYPE_DATA.
 *   responseMode - the operation response data reporting mode
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioSetResponseDataMode(
    RFID_RADIO_HANDLE   handle,
    RFID_RESPONSE_TYPE  responseType,
    RFID_RESPONSE_MODE  responseMode
    );

/******************************************************************************
 * Name: RFID_RadioGetResponseDataMode
 *
 * Description:
 *   Retrieves the operation response data reporting mode for tag-protocol
 *   operations.  The data response mode may not be retrieved while a radio
 *   module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - the handle to the radio for which the operation response data
 *     reporting mode is to be retrieved.  This is the handle from a successful
 *     call to RFID_RadioOpen.
 *   responseType - the type of data that will have its reporting mode
 *     retrieved.  For version 1.0 of the library, the only valid value is
 *     RFID_RESPONSE_TYPE_DATA.
 *   pResponseMode - a pointer to a RFID_RESPONSE_MODE variable that upon return
 *     will contain the operation response data reporting mode.  Must not be
 *     NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *   RFID_ERROR_UNEXPECTED_VALUE
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioGetResponseDataMode(
    RFID_RADIO_HANDLE   handle,
    RFID_RESPONSE_TYPE  responseType,
    RFID_RESPONSE_MODE* pResponseMode
    );

/******************************************************************************
 * Name: RFID_MacUpdateNonvolatileMemory
 *
 * Description:
 *   Writes the specified data to the radio module’s nonvolatile-memory 
 *   block(s).  After a successful update, the RFID radio module resets itself
 *   and the RFID Reader Library closes and invalidates the radio handle so that
 *   it may no longer be used by the application.
 *
 *   In the case of an unsuccessful update the RFID Reader Library does not
 *   invalidate the radio handle – i.e., it is the application’s responsibility
 *   to close the handle.
 *
 *   Alternatively, an application can perform the update in “test” mode.
 *   An application uses the “test” mode, by checking the returned status, to
 *   verify that the update would succeed before performing the destructive
 *   update of the radio module’s nonvolatile memory.  When a “test” update has
 *   completed, either successfully or unsuccessfully, the MAC firmware returns
 *   to its normal idle state and the radio handle remains valid (indicating
 *   that the application is still responsible for closing it).
 *
 *   The radio module’s nonvolatile memory may not be updated while a radio
 *   module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - The handle, previously returned by a successful call to
 *     RFID_RadioOpen, of the RFID radio module for which the nonvolatile
 *     memory should be updated.
 *   countBlocks - The number of nonvolatile memory blocks in the array pointed
 *     to by pBlocks.  This value must be greater than zero.
 *   pBlocks - A pointer to an array of countBlocks nonvolatile memory block
 *     structures that are used to control the update of the radio module’s
 *     nonvolatile memory.  This pointer must not be NULL.
 *   flags - Zero, or a combination of the following:
 *     RFID_FLAG_TEST_UPDATE - Indicates that the RFID Reader Library is to
 *       perform a non-destructive nonvolatile memory update to verify that
 *       the update would succeed.  The RFID Reader Library will perform all
 *       of the update operations with the exception that the data will not be
 *       committed to nonvolatile memory.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *   RFID_ERROR_NONVOLATILE_INIT_FAILED
 *   RFID_ERROR_NONVOLATILE_WRITE_FAILED
 *   RFID_ERROR_NONVOLATILE_OUT_OF_BOUNDS
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacUpdateNonvolatileMemory(
    RFID_RADIO_HANDLE                       handle,
    INT32U                                  countBlocks,
    const RFID_NONVOLATILE_MEMORY_BLOCK*    pBlocks,
    INT32U                                  flags
    );

/******************************************************************************
 * Name: RFID_MacGetVersion
 *
 * Description:
 *   Retrieves the radio module's MAC firmware version information.  The MAC
 *   version may not be retrieved while a radio module is executing a tag-
 *   protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which MAC firmware version information is
 *     requested.  This is the handle from a successful call to RFID_RadioOpen.
 *   pVersion - pointer to structure that upon return will contain the radio
 *     module's MAC firmware version information.  Must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *   RFID_ERROR_UNEXPECTED_VALUE
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacGetVersion(
    RFID_RADIO_HANDLE   handle,
    RFID_VERSION*       pVersion
    );

/******************************************************************************
 * Name: RFID_MacGetBootLoaderVersion
 *
 * Description:
 *   Retrieves the radio module's BootLoader firmware version information.  The 
 *   BootLoader version may not be retrieved while a radio module is executing a tag-
 *   protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which BootLoader firmware version information is
 *     requested.  This is the handle from a successful call to RFID_RadioOpen.
 *   pVersion - pointer to structure that upon return will contain the radio
 *     module's BootLoader firmware version information.  Must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *   RFID_ERROR_UNEXPECTED_VALUE
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacGetBootLoaderVersion(
    RFID_RADIO_HANDLE   handle,
    RFID_VERSION*       pVersion
    );
/******************************************************************************
 * Name: RFID_MacReadOemData
 *
 * Description:
 *   Reads one or more 32-bit words from the MAC's OEM configuration data
 *   area.  The OEM data are may not be read while a radio module is executing
 *   a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which MAC OEM configuration data is to be
 *     read.  This is the handle returned from a successful call to
 *     RFID_RadioOpen.
 *   address - the 32-bit address of the first 32-bit word to read from
 *     the MAC's OEM configuration data area.  Note that this is not a byte
 *     address - i.e., address 1 is actually byte 4, address 2 is actually byte
 *     8, etc.  If the address is beyond the end of the OEM configuration data
 *     area, results in an error.
 *   pCount - On input, pointer to the number of 32-bit words to read.   
 *     Must be greater than zero.  If count causes the read to extend beyond 
 *     the end of the OEM configuration data area, results in an error.
 *     On return, the caller's value is updated to reflect the number of  
 *     32-bit words actually read.  This may be less than the requested amount
 *     if the input parameters result in an error.
 *   pData - pointer to the buffer into which the OEM configuration data will
 *     be placed.  The buffer must be at least (count * 4) bytes in length.
 *     Must not be NULL.  Note that the data returned will be in the MAC's
 *     native format (i.e., little endian).
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacReadOemData(
    RFID_RADIO_HANDLE   handle,
    INT32U              address,
    INT32U*             pCount,
    INT32U*             pData
    );

/******************************************************************************
 * Name: RFID_MacWriteOemData
 *
 * Description:
 *   Writes one or more 32-bit words to the MAC's OEM configuration data
 *   area.  The OEM data area may not be written while a radio module is
 *   executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which MAC OEM configuration data is to be
 *     written.  This is the handle returned from a successful call to
 *     RFID_RadioOpen.
 *   address - the 32-bit address of the first 32-bit word to write in
 *     the MAC's OEM configuration data area.  Note that this is not a byte
 *     address - i.e., address 1 is actually byte 4, address 2 is actually byte
 *     8, etc.  If the address is beyond the end of the OEM configuration data
 *     area, results in an error.
 *   pCount - On input, pointer to the number of 32-bit words to read.   
 *     Must be greater than zero.  If count causes the write to extend beyond 
 *     the end of the OEM configuration data area, results in an error.
 *     On return, the caller's value is updated to reflect the number of  
 *     32-bit words actually written.  This may be less than the requested amount
 *     if the input parameters result in an error.
 *   pData - pointer to the buffer that contains the data to write to the OEM
 *     configuration area.  The buffer must be at least (count * 4) bytes in
 *     length.  Must not be NULL.  Note that the data must be in the MAC's
 *     native format (i.e., little endian).
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacWriteOemData(
    RFID_RADIO_HANDLE   handle,
    INT32U              address,
    INT32U*             pCount,
    const INT32U*       pData
    );

/******************************************************************************
 * Name: RFID_MacReset
 *
 * Description:
 *   Instructs the radio module's MAC firmware to perform the specified reset.
 *   Any currently executing tag-protocol operations will be aborted, any
 *   unconsumed data will be discarded, and tag-protocol operation functions
 *   (i.e., RFID_18K6CTagInventory, etc.) will return immediately with an
 *   error of RFID_ERROR_OPERATION_CANCELLED.
 *   Upon reset, the connection to the radio module is lost and the handle
 *   to the radio is invalid.  To obtain control of the radio module after it
 *   has been reset, the application must re-enumerate the radio modules, via
 *   RFID_RetrieveAttachedRadiosList, and request control via RFID_RadioOpen.
 *
 *   NOTE: This function must not be called from the packet callback function.
 *
 * Parameters:
 *   handle - handle for the radio which will be reset.  This is the handle that
 *     was returned from a successful call to RFID_RadioOpen.  Upon return
 *     the handle is invalid and may only be used for RFID_RadioClose.
 *   resetType - the type of reset to perform on the radio
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_CURRENTLY_NOT_ALLOWED
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacReset(
    RFID_RADIO_HANDLE   handle,
    RFID_MAC_RESET_TYPE resetType
    );

/******************************************************************************
 * Name: RFID_MacClearError
 *
 * Description:
 *   Attempts to clear the error state for the radio module’s MAC firmware.  The
 *   MAC error may not be cleared while a radio module is executing a tag-
 *   protocol operation.
 *
 * Parameters:
 *   handle - handle for the radio which will have MAC error state cleared.
 *     This is the handle thatwas returned from a successful call to
 *     RFID_RadioOpen.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacClearError(
    RFID_RADIO_HANDLE   handle
    );

/******************************************************************************
 * Name: RFID_MacGetError
 *
 * Description:
 *   Retrieves the low-level radio module current MAC_ERROR and MAC_LAST_ERROR
 *   values.  These errors may not be retrieved while the radio module is
 *   executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which MAC_ERROR is to
 *     be retrieved.  This is the handle from a successful call to
 *     RFID_RadioOpen.
 *   pError - pointer to variable that will receive MAC_ERROR
 *     value.  Must not be NULL.
 *   pLastError - pointer to variable that will receive MAC_LAST_ERROR
 *     value.  Must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacGetError(
    RFID_RADIO_HANDLE   handle,
    INT32U*             pError,
    INT32U*             pLastError
    );

/******************************************************************************
 * Name: RFID_MacBypassWriteRegister
 *
 * Description:
 *   Allows for direct writing of registers on the radio (i.e., bypassing the
 *   MAC).  The radio registers may not be written while a radio module is
 *   executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which register is to be written.  This is
 *     the handle from a successful call to RFID_RadioOpen.
 *   address - the address of the register to write.  An address that is beyond
 *    the end of the radio module's register set results in an invalid-parameter
 *    return status.
 *   value - the value to write to the register
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacBypassWriteRegister(
    RFID_RADIO_HANDLE   handle,
    INT16U              address,
    INT16U              value
    );

/******************************************************************************
 * Name: RFID_MacBypassReadRegister
 *
 * Description:
 *   Allows for direct reading of registers on the radio (i.e., bypassing the
 *   MAC).  The radio regsiters mode may not be read while a radio module is
 *   executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which register is to be read.  This is
 *     the handle from a successful call to RFID_RadioOpen.
 *   address - the address of the register to write  An address that is beyond
 *    the end of the radio module's register set results in an invalid-parameter
 *    return status.
 *   pValue - pointer to unsigned 16-bit integer that will receive register
 *     value.  This parameter must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RADIO_BUSY
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacBypassReadRegister(
    RFID_RADIO_HANDLE   handle,
    INT16U              address,
    INT16U*             pValue
    );


/******************************************************************************
 * Name: RFID_MacGetRegion
 *
 * Description:
 *   Retrieves the regulatory mode region for the MAC's operation.  The region
 *   of operation may not be retrieved while a radio module is executing a tag-
 *   protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which region is to be retrieved.  This is
 *     the handle from a successful call to RFID_RadioOpen.
 *   pRegion - pointer to variable that will receive region.  Must not be NULL.
 *   pRegionConfig - reserved for future use.  Must be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_MacGetRegion(
    RFID_RADIO_HANDLE   handle,
    RFID_MAC_REGION*    pRegion,
    void*               pRegionConfig
    );

/******************************************************************************
 * Name: RFID_RadioSetGpioPinsConfiguration
 *
 * Description:
 *   Configures the specified radio module's GPIO pins.  Only GPIO pins 0-3 
 *   are valid.  The GPIO pin configuration may not
 *   be set while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which GPIO pins should be configured.  This is
 *     the handle that was returned from a successful call to RFID_RadioOpen.
 *   mask - a 32-bit mask which specifies which GPIO pins are to be configured.
 *     Bit 0 (i.e., the lowest-order bit) represents GPIO pin 0, bit 1
 *     represents GPIO pin 1, etc.  The presence of a 1 bit in a mask-bit
 *     location indicates that the GPIO pin is to be configured.  The presence
 *     of a 0 bit in a mask-bit location indicates that the GPIO pin’s
 *     configuration is to remain unchanged.
 *   configuration - A 32-bit value that indicates the configuration for the
 *     bits corresponding to the ones set in mask – bit 0 (i.e., the lowest-
 *     order bit) represents GPIO pin 0's configuration, etc.  Bits which
 *     correspond to bits set to 0 in mask are ignored.  The presence of a 1 in
 *     a bit location indicates that the GPIO pin is to be configured as an
 *     output pin.  The presence of a 0 in a bit location indicates that the
 *     GPIO pin is to be configured as an input pin.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioSetGpioPinsConfiguration(
    RFID_RADIO_HANDLE   handle,
    INT32U              mask,
    INT32U              configuration
    );

/******************************************************************************
 * Name: RFID_RadioGetGpioPinsConfiguration
 *
 * Description:
 *   Retrieves the configuration for the radio module's GPIO pins. Only GPIO 
 *   pins 0-3 are valid.  The GPIO pin configuration may not be retrieved 
 *   while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which GPIO pin configuration should be
 *     retrieved.  This is the handle that was returned from a successful call
 *     to RFID_RadioOpen.
 *   pConfiguration - A pointer to an unsigned 32-bit integer that upon return
 *     contains the configuration for the radio module’s GPIO pins – bit 0
 *     (i.e., the lowest-order bit) represents GPIO pin 0, etc.  The presence
 *     of a 1 in a bit location indicates that the GPIO pin is configured as an
 *     output pin.  The presence of a 0 in a bit location indicates that the
 *     GPIO pin is configured as an input pin.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioGetGpioPinsConfiguration(
    RFID_RADIO_HANDLE   handle,
    INT32U*             pConfiguration
    );

/******************************************************************************
 * Name: RFID_RadioReadGpioPins
 *
 * Description:
 *   Reads the specified radio module's GPIO pins.  Attempting to read from an
 *   output GPIO pin results in an error.  Only GPIO pins 0-3 are valid.  The 
 *   GPIO pins may not be read while a radio module is executing a 
 *   tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which GPIO pins should be read.  This is the
 *     handle that was returned from a successful call to RFID_RadioOpen.
 *   mask - a 32-bit mask which specifies which GPIO pins are to be read.  Bit
 *     0 (i.e., the lowest-order bit) represents GPIO pin 0, bit 1 represents
 *     GPIO pin 1, etc.  The presence of a 1 bit in a mask bit location
 *     indicates that the GPIO pin is to be read.
 *   pValue - a pointer to a 32-bit unsigned integer that upon return will
 *     contain the bit values of the GPIO pins specified in the mask.  Bit 0 of
 *     the *pValue corresponds to GPIO pin 0, bit 1 corresponds to GPIO
 *     pin 1, etc.  If a GPIO pin's bit is not set in mask, then the bit value
 *     in the corresponding bit in *pValue is undefined.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioReadGpioPins(
    RFID_RADIO_HANDLE   handle,
    INT32U              mask,
    INT32U*             pValue
    );

/******************************************************************************
 * Name: RFID_RadioWriteGpioPins
 *
 * Description:
 *   Writes the specified radio module's GPIO pins.  Attempting to write to an
 *   input GPIO pin results in an error.  Only GPIO pins 0-3 are valid.  The 
 *   GPIO pins may not be written while a radio module is executing a 
 *   tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio for which GPIO pins should be written.  This is
 *     the handle that was returned from a successful call to RFID_RadioOpen.
 *   mask - a 32-bit mask which specifies which GPIO pins are to be written.
 *     Bit 0 (i.e., the lowest-order bit) represents GPIO pin 0, bit 1
 *     represents GPIO pin 1, etc.  The presence of a 1 in a mask bit location
 *     indicates that the corresponding bit in value is to be written to the 
 *     GPIO pin.
 *   value - a 32-bit unsigned integer that contains the bits to write to the
 *     GPIO pins specifed in mask.  Bit 0 of the value corresponds to the value
 *     to write to GPIO pin 0, bit 1 corresponds to the value to write to GPIO
 *     pin 1, etc.  If a GPIO pin's bit is not set in mask, then the bit value
 *     in the corresponding bit is ignored.
 *
 * Parameters:
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioWriteGpioPins(
    RFID_RADIO_HANDLE   handle,
    INT32U              mask,
    INT32U              value
    );


/******************************************************************************
 * Name: RFID_RadioTurnCarrierWaveOn
 *
 * Description:
 *   Turns on the radio's RF carrier wave.
 *
 * Parameters:
 *   handle - handle to radio upon which the carrier wave should be turned on.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioTurnCarrierWaveOn(
    RFID_RADIO_HANDLE   handle
    );

/******************************************************************************
 * Name: RFID_RadioTurnCarrierWaveOff
 *
 * Description:
 *   Turns off the radio's RF carrier wave.
 *
 * Parameters:
 *   handle - handle to radio upon which the carrier wave should be turned off.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioTurnCarrierWaveOff(
    RFID_RADIO_HANDLE   handle
    );

/******************************************************************************
 * Name: RFID_RadioTurnCarrierWaveOnRandom
 *
 * 
 * Description:
 *   Turns on the radio's RF carrier wave with random transmit data for the 
 *   specified duration.  It is the application's responsibility to process the 
 *   resulting packets and verify that the command was successful.
 * 
 * Parameters:
 *   handle - handle to an open radio device
 *   pParms - pointer to a structure that specifies the duration for the random
 *      CW and the callback parameters if desired.
 *      This parameter must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioTurnCarrierWaveOnRandom(
    RFID_RADIO_HANDLE                       handle,
    const RFID_RANDOM_CW_PARMS*             pParms
    );


/******************************************************************************
 * Name: RFID_RadioIssueCommand
 *
 * Description:
 *   Allows an application to issue any radio command.  It is the application's
 *   responsibility to process the resulting packets and verify that the
 *   command was successful.
 *
 * Parameters:
 *   handle - handle to radio upon which the command will be executed
 *   pParms - pointer to a structure that specifies the command to be 
 *      issued and the callback parameters if desired.
 *      This parameter must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioIssueCommand(
    RFID_RADIO_HANDLE                       handle,
    const RFID_ISSUE_RADIO_COMMAND_PARMS*   pParms
    );


/******************************************************************************
 * Name: RFID_RadioSetImpinjExtensions
 *
 * Description:
 *   Specifies which Impinj Extensions are enabled.  The Impinj Extensions 
 *   will remain in effect until a subsequent call to 
 *   RFID_RadioSetImpinjExtensions.  The Impinj Extensions may not be changed 
 *   while the radio is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which the Impinj Extensions will be configured.
 *     This is the handle from a successful call to RFID_RadioOpen.
 *   pExtensions - a pointer to a structure that specifies how the Impinj 
 *     Extensions should be configured.  This parameter must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE 
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioSetImpinjExtensions(
    RFID_RADIO_HANDLE                    handle,
    const RFID_IMPINJ_EXTENSIONS*        pExtensions);

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioGetImpinjExtensions
//
// Description:
//   Retrieves the Impinj Extensions from the Radio.  The extensions may not be
//   retrieved while a radio module is executing a tag-protocol operation.
////////////////////////////////////////////////////////////////////////////////

/******************************************************************************
 * Name: RFID_RadioGetImpinjExtensions
 *
 * Description:
 *   etrieves the Impinj Extensions from the Radio.  The extensions may not be
 *   retrieved while a radio module is executing a tag-protocol operation.
 *
 * Parameters:
 *   handle - handle to radio upon which the Impinj Extensions will be retrieved.
 *     This is the handle from a successful call to RFID_RadioOpen.
 *   pExtensions - a pointer to a structure that upon return contains the Impinj
 *     Extensions.  This parameter must not be NULL.
 *
 * Returns:
 *   RFID_STATUS_OK
 *   RFID_ERROR_NOT_INITIALIZED
 *   RFID_ERROR_INVALID_HANDLE
 *   RFID_ERROR_INVALID_PARAMETER
 *   RFID_ERROR_RADIO_NOT_PRESENT
 *   RFID_ERROR_RADIO_FAILURE
 *   RFID_ERROR_RECEIVE_OVERFLOW
 *   RFID_ERROR_RADIO_BUSY
 *   RFID_ERROR_RADIO_NOT_RESPONDING
 *   RFID_ERROR_UNEXPECTED_VALUE
 *
 ******************************************************************************/
RFID_LIBRARY_API RFID_STATUS RFID_RadioGetImpinjExtensions(
    RFID_RADIO_HANDLE         handle,
    RFID_IMPINJ_EXTENSIONS*   pExtensions);

#ifdef __cplusplus
}
#endif

#endif  /* #ifndef RFID_LIBRARY_H_INCLUDED */
