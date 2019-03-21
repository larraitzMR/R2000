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
 * $Id: radio.h 66624 2011-01-25 04:05:44Z dshaheen $
 * 
 * Description:
 *     This header presents the interface for the class that is a used to
 *     to communicate with a radio's MAC.
 *     
 *
 *****************************************************************************
 */

#ifndef RADIO_H_INCLUDED
#define RADIO_H_INCLUDED

#include <memory>
#include <vector>
#include "rfid_platform_types.h"
#include "rfid_structs.h"
#include "hostpkts.h"
#include "mac.h"
#include "auto_handle_compat.h"
#include "compat_thread.h"
#include "compat_mutex.h"


namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: Radio
//
// Description: This class is used to represent a radio and the high-level
//     operations (e.g., inventory, etc.) that are supported by the radio.
////////////////////////////////////////////////////////////////////////////////
class Radio
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name:        Radio
    // Description: Initializes the radio object.
    // Parameters:  pMac - a pointer to the Mac object that is used by the radio
    //              object
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    Radio(
        std::auto_ptr<Mac> pMac
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ~Radio
    // Description: Cleans up the radio object.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    ~Radio();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Close
    // Description: Requests that the radio object close the underlying radio
    //              module.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Close();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        WriteMacRegister
    // Description: Requests to set a low-level radio module register
    // Parameters:  address - the register to set
    //              value - the value to set the register to
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void WriteMacRegister(
        INT16U  address,
        INT32U  value
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ReadMacRegister
    // Description: Requests a retrieval of  a low-level radio module
    //              register.
    // Parameters:  address - the register address to read
    // Returns:     The value of the register read.
    ////////////////////////////////////////////////////////////////////////////
    INT32U ReadMacRegister(
        INT16U  address
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ReadMacRegisterInfo
    // Description: Requests a retrieval of  a low-level radio module
    //              register info
    // Parameters:  address - the register to retrieve info for
    //              pInfo - pointer to the register info structure
    //                      to be populated
    // Returns:     None
    ////////////////////////////////////////////////////////////////////////////
    void Radio::ReadMacRegisterInfo(
        INT16U                            address,
        RFID_REGISTER_INFO                *pInfo
        );

    ////////////////////////////////////////////////////////////////////////////////
    // Name:        ReadMacBankedRegister
    // Description: Requests a retrieval of  a low-level radio module
    //              banked register.
    // Parameters:  address - the address of the banked register to retrieve
    //              bankSelector - the bank selector for the register
    ////////////////////////////////////////////////////////////////////////////////
    INT32U Radio::ReadMacBankedRegister(
        INT16U  address,
        INT16U  bankSelector
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        WriteMacBankedRegister
    // Description: Requests to set a low-level radio module banked
    //              register.
    // Parameters:  address - the address of the banked register to set
    //              bankSelector - the bank selector for the register
    //              value - the value to set the register to
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void WriteMacBankedRegister(
        INT16U  address,
        INT16U  bankSelector,
        INT32U  value
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        SetOperationMode
    // Description: Sets the operation mode for the radio
    // Parameters:  mode - the new operation mode for the radio
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void SetOperationMode(
        RFID_RADIO_OPERATION_MODE   mode
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetOperationMode
    // Description: Retrieves the operation mode for the radio
    // Parameters:  None
    // Returns:     The operation mode
    ////////////////////////////////////////////////////////////////////////////
    RFID_RADIO_OPERATION_MODE GetOperationMode();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        SetPowerState
    // Description: Sets the radio's power state
    // Parameters:  state - the new power state for the radio
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void SetPowerState(
        RFID_RADIO_POWER_STATE  state
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetPowerState
    // Description: Retrieves the radio's power state
    // Parameters:  None
    // Returns:     The current power state of the radio
    ////////////////////////////////////////////////////////////////////////////
    RFID_RADIO_POWER_STATE GetPowerState();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        SetCurrentLinkProfile
    // Description: Sets the radio's current link profile
    // Parameters:  profile - the profile to set as the current one
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void SetCurrentLinkProfile(
        INT32U  profile
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetCurrentLinkProfile
    // Description: Retrieves the radio's current link profile
    // Parameters:  None
    // Returns:     The current link profile
    ////////////////////////////////////////////////////////////////////////////
    INT32U GetCurrentLinkProfile();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetLinkProfile
    // Description: Retrieves the link profile information for the profile
    //              specified
    // Parameters:  profile - the link profile to retrieve information for
    //              pProfileInfo - a pointer to a structure that will be filled
    //              in with profile information upon return
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void GetLinkProfile(
        INT32U                      profile,
        RFID_RADIO_LINK_PROFILE*    pProfileInfo
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        WriteLinkProfileRegister
    // Description: Sets the link-profile register for the profile specified
    // Parameters:  profile - the link profile for which to set the link-
    //                profile register
    //              address - address of link-profile register
    //              value - the value to the put into the link-profile register
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void WriteLinkProfileRegister(
        INT32U  profile,
        INT16U  address,
        INT16U  value
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ReadLinkProfileRegister
    // Description: Retrieves the link-profile register for the profile
    //              specified
    // Parameters:  profile - the link profile for which to retrieve the link-
    //                profile register
    //              address - address of link-profile register
    // Returns:     The value in the link-profile register
    ////////////////////////////////////////////////////////////////////////////
    INT16U ReadLinkProfileRegister(
        INT32U  profile,
        INT16U  address
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetAntennaPortStatus
    // Description: Retrieves the status for the specified antenna port
    // Parameters:  antennaPort - the antenna port for which to retrieve status
    //              pStatus - pointer to structure that receives status
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void GetAntennaPortStatus(
        INT32U                      antennaPort,
        RFID_ANTENNA_PORT_STATUS*   pStatus
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        SetAntennaPortState
    // Description: Sets the state of an antenna port
    // Parameters:  antennaPort - the antenna port for which to set state
    //              state - the new state for the antenna port
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void SetAntennaPortState(
        INT32U                  antennaPort,
        RFID_ANTENNA_PORT_STATE state
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        SetAntennaPortConfiguration
    // Description: Sets the configuration for an antenna port
    // Parameters:  antennaPort - the antenna port for which to set 
    //                configuration
    //              pConfig - a pointer to a structure which contains the
    //                antenna-port configuration
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void SetAntennaPortConfiguration(
        INT32U                          antennaPort,
        const RFID_ANTENNA_PORT_CONFIG* pConfig
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetAntennaPortConfiguration
    // Description: Retrieves the configuration for an antenna port
    // Parameters:  antennaPort - the antenna port for which to retrieve
    //                configuration
    //              pConfig - a pointer to a structure which receives the
    //                antenna-port configuration
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void GetAntennaPortConfiguration(
        INT32U                      antennaPort,
        RFID_ANTENNA_PORT_CONFIG*   pConfig
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Set18K6CSelectCriteria
    // Description: Sets the ISO 18000-6C tag-selection criteria.
    // Parameters:  pCriteria - a pointer to the ISO 18000-6C tag-selection
    //              criteria
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Set18K6CSelectCriteria(
        const RFID_18K6C_SELECT_CRITERIA* pCriteria
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Get18K6CSelectCriteria
    // Description: Gets the ISO 18000-6C tag-selection criteria.
    // Parameters:  pCriteria - a pointer to the structure which receives the
    //              criteria
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Get18K6CSelectCriteria(
        RFID_18K6C_SELECT_CRITERIA* pCriteria
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Set18K6CPostSingulationMatchCriteria
    // Description: Sets the match criteria to be applied to an ISO 18000-6C tag
    //              after singulation
    // Parameters:  pParms - a pointer to the post-singulation match criteria
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Set18K6CPostSingulationMatchCriteria(
        const RFID_18K6C_SINGULATION_CRITERIA* pParms
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Get18K6CPostSingulationMatchCriteria
    // Description: Gets the match criteria to be applied to an ISO 18000-6C tag
    //              after singulation
    // Parameters:  pParms - a pointer to the structure that will have the
    //              post-singulation match criterian on return
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Get18K6CPostSingulationMatchCriteria(
        RFID_18K6C_SINGULATION_CRITERIA* pParms
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Set18K6CQueryTagGroup
    // Description: Sets the query tag group parameters
    // Parameters:  pGroup - a pointer to the query tag group parameters
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Set18K6CQueryTagGroup(
        const RFID_18K6C_TAG_GROUP* pGroup
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Get18K6CQueryTagGroup
    // Description: Retrieves the query tag group parameters
    // Parameters:  pGroup - a pointer to structure that will receive the query
    //              tag group parameters
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Get18K6CQueryTagGroup(
        RFID_18K6C_TAG_GROUP*   pGroup
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        SetCurrentSingulationAlgorithm
    // Description: Sets the currently-active singulation algorithm
    // Parameters:  algorithm - the algorithm to make currently-active
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void SetCurrentSingulationAlgorithm(
        RFID_18K6C_SINGULATION_ALGORITHM    algorithm
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetCurrentSingulationAlgorithm
    // Description: Retrieves the currently-active singulation algorithm
    // Parameters:  None
    // Returns:     The currently-active singulation algorithm.
    ////////////////////////////////////////////////////////////////////////////
    RFID_18K6C_SINGULATION_ALGORITHM GetCurrentSingulationAlgorithm();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        SetSingulationAlgorithmParameters
    // Description: Sets the singulation algorithm parameters for the fixed Q
    //              singulation algorithm
    // Parameters:  pParms - pointer to singulation parameters
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void SetSingulationAlgorithmParameters(
        const RFID_18K6C_SINGULATION_FIXEDQ_PARMS*      pParms
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        SetSingulationAlgorithmParameters
    // Description: Sets the singulation algorithm parameters for the dynamic Q
    //              with Q-adjustment threshold singulation algorithm
    // Parameters:  pParms - pointer to singulation parameters
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void SetSingulationAlgorithmParameters(
        const RFID_18K6C_SINGULATION_DYNAMICQ_PARMS* pParms
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetSingulationAlgorithmParameters
    // Description: Retrieves the singulation algorithm parameters for the
    //              fixed Q singulation algorithm
    // Parameters:  pParms - pointer to structure that will receive the
    //              singulation algorithm parameters
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void GetSingulationAlgorithmParameters(
        RFID_18K6C_SINGULATION_FIXEDQ_PARMS*    pParms
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetSingulationAlgorithmParameters
    // Description: Retrieves the singulation algorithm parameters for the
    //              dynamic Q with Q-adjustment threshold singulation algorithm
    // Parameters:  pParms - pointer to structure that will receive the
    //              singulation algorithm parameters
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void GetSingulationAlgorithmParameters(
        RFID_18K6C_SINGULATION_DYNAMICQ_PARMS*   pParms
        );


    ////////////////////////////////////////////////////////////////////////////
    // Name:        Start18K6CInventory
    // Description: Requests that an ISO 18000-6C inventory be started on the
    //              radio module.
    // Parameters:  pParms - a pointer to a structure that specifies the
    //              parameters for the inventory operation
    //              flags - flags that control the execution of the inventory
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Start18K6CInventory(
        const RFID_18K6C_INVENTORY_PARMS*   pParms,
        INT32U                              flags
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Start18K6CRead
    // Description: Requests that an ISO 18000-6C tag read be started on the
    //              radio module.
    // Parameters:  pParms - a pointer to a structure that specifies the
    //              parameters for the tag read operation
    //              flags - flags that control the execution of the tag read
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Start18K6CRead(
        const RFID_18K6C_READ_PARMS*        pParms,
        INT32U                              flags
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Start18K6CWrite
    // Description: Requests that an ISO 18000-6C tag write be started on the
    //              radio module.
    // Parameters:  pParms - a pointer to a structure that specifies the
    //              parameters for the tag write operation
    //              flags - flags that control the execution of the tag write
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Start18K6CWrite(
        const RFID_18K6C_WRITE_PARMS*       pParms,
        INT32U                              flags
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Start18K6CKill
    // Description: Requests that an ISO 18000-6C tag kill be started on the
    //              radio module.
    // Parameters:  pParms - a pointer to a structure that specifies the
    //              parameters for the tag kill operation
    //              flags - flags that control the execution of the tag kill
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Start18K6CKill(
        const RFID_18K6C_KILL_PARMS*        pParms,
        INT32U                              flags
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Start18K6CLock
    // Description: Requests that an ISO 18000-6C tag lock be started on the
    //              radio module.
    // Parameters:  pParms - a pointer to a structure that specifies the
    //              parameters for the tag lock operation
    //              flags - flags that control the execution of the tag lock
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Start18K6CLock(
        const RFID_18K6C_LOCK_PARMS*        pParms,
        INT32U                              flags
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Start18K6CBlockWrite
    // Description: Requests that an ISO 18000-6C block write be started on the
    //              radio module.
    // Parameters:  pParms - a pointer to a structure that specifies the
    //              parameters for the block write operation
    //              flags - flags that control the execution of the tag write
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Start18K6CBlockWrite(
        const RFID_18K6C_BLOCK_WRITE_PARMS*       pBParms,
        INT32U                              flags
        );

    ////////////////////////////////////////////////////////////////////////////////
    // Name:        Start18K6CQT
    // Description: Requests that an ISO 18000-6C tag QT operation be started on the
    //              radio module.
    // Parameters:  pParms - a pointer to a structure that specifies the
    //              parameters for the QT operation
    //              flags - flags that control the execution of the QT command
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////////
    void Start18K6CQT(
        const RFID_18K6C_QT_PARMS*          pParms,
        INT32U                              flags
        );


    ////////////////////////////////////////////////////////////////////////////
    // Name:        Start18K6CBlockErase
    // Description: Requests that an ISO 18000-6C tag block erase be started 
    //              on the radio module.
    // Parameters:  pParms - a pointer to a structure that specifies the
    //              parameters for the tag block erase operation
    //              flags - flags that control the execution of the tag 
    //                      block erase
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Start18K6CBlockErase(
        const RFID_18K6C_BLOCK_ERASE_PARMS*    pParms,
        INT32U                                 flags
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ProcessOperationData
    // Description: Requests that the radio object begin receiving operation
    //              response packets from the radio module and invoking the 
    //              supplied callback.
    // Parameters:  handle - the radio handle that will be supplied to the
    //              callback
    //              pCallback - a pointer to the callback function
    //              context - application-supplied piece of data that will be
    //              supplied to the callback
    //              pCallbackCode - a pointer to a variable that upon return
    //              will contain the return value from the last callback.  May
    //              be NULL if application doesn't care about return code.
    //              canBeCancelled - indicates if the application can issue a
    //              cancel while the processing going on
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ProcessOperationData(
        RFID_RADIO_HANDLE               handle,
        RFID_PACKET_CALLBACK_FUNCTION   pCallback,
        void*                           context,
        INT32S*                         pCallbackCode,
        bool                            canBeCancelled = true
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        CancelOperation
    // Description: Requests that the radio object instruct the underlying radio
    //              module to cancel its current operation.  When cancelling,
    //              the remaining packets will still be delivered via the
    //              packet callback function.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void CancelOperation();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        AbortOperation
    // Description: Requests that the radio object instruct the underlying radio
    //              module to abort its current operation.  When aborting,
    //              the remaining packets are discarded.
    // Parameters:  None.
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void AbortOperation();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        SetResponseDataMode
    // Description: Sets the response date mode (i.e., compact, etc.) for the
    //              type of data from the radio
    // Parameters:  type - the type of data for which the response mode will be
    //                set
    //              mode - the response mode for the data
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void SetResponseDataMode(
        RFID_RESPONSE_TYPE  type,
        RFID_RESPONSE_MODE  mode
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetResponseDataMode
    // Description: Retrieves the response date mode (i.e., compact, etc.) for
    //              the type of data from the radio
    // Parameters:  responseType - the type of data for which the response mode
    //                will be retrieved
    // Returns:     The response mode for the data
    ////////////////////////////////////////////////////////////////////////////
    RFID_RESPONSE_MODE GetResponseDataMode(
        RFID_RESPONSE_TYPE  type
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        UpdateMacNonvolatileMemory
    // Description: Updates the MAC's nonvolatile memory blocks specified
    // Parameters:  countBlocks - number of nonvolatile memory blocks to update
    //              pBlocks - pointer to array of nonvolatile memory block
    //                upate structures
    //              flags - flags that control the update
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void UpdateMacNonvolatileMemory(
        INT32U                                  countBlocks,
        const RFID_NONVOLATILE_MEMORY_BLOCK*    pBlocks,
        INT32U                                  flags
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetMacVersion
    // Description: Request to retrieve the version for the MAC firmware on the
    //              radio module.
    // Parameters:  pVersion - a pointer to a structure that upon return will
    //              contain the MAC's version.
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void GetMacVersion(
        RFID_VERSION* pVersion
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetMacBootLoaderVersion
    // Description: Request to retrieve the version for the BootLoader image on 
    //              the radio module.
    // Parameters:  pVersion - a pointer to a structure that upon return will
    //              contain the BootLoader version.
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void GetMacBootLoaderVersion(
        RFID_VERSION* pVersion
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ReadMacOemData
    // Description: Reads the requested data from the MAC OEM configuration area
    // Parameters:  address - the beginning OEM configuration area 32-bit
    //                address to read from
    //              pCount - pointer to the number of 32-bit words to read on 
    //                       input, the number of 32-bit words read on return
    //              pData - pointer to buffer into which data will be read
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ReadMacOemData(
        INT32U  address,
        INT32U* pCount,
        INT32U* pData
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        WriteMacOemData
    // Description: Writes the supplied data to the MAC OEM configuration area
    // Parameters:  address - the beginning OEM configuration area 32-bit
    //                address to write to
    //              pCount - pointer to the number of 32-bit words to write on 
    //                       input, the number of 32-bit words written on return
    //              pData - pointer to buffer that contains data to write
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void WriteMacOemData(
        INT32U          address,
        INT32U*         pCount,
        const INT32U*   pData
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ResetMac
    // Description: Reset the radio's MAC
    // Parameters:  type - the type of reset to perform
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ResetMac(
        RFID_MAC_RESET_TYPE type
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ClearMacError
    // Description: Clears any error condition on the MAC
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ClearMacError();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetMacError
    // Description: Retrieves the current MAC_ERROR and MAC_LAST_ERROR from
    //              the radio.  Note in MAC firmware prior to 2.2, MAC_LAST_ERROR
    //              is not supported, and is returned as 0.
    // Parameters:  pError - pointer to storage for MAC_ERROR
    //              pLastError - pointer to storage for MAC_LAST_ERROR
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Radio::GetMacError(
        INT32U*             pError,
        INT32U*             pLastError
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        WriteRadioRegister
    // Description: Allows for writing directly to the radio's registers (i.e.,
    //              bypassing the MAC.
    // Parameters:  address - address of register to write
    //              value - value to write to register
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void WriteRadioRegister(
        INT16U  address,
        INT16U  value
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ReadRadioRegister
    // Description: Allows for reading directly from the radio's registers (i.e.,
    //              bypassing the MAC.
    // Parameters:  address - address of register to read
    // Returns:     The register's value
    ////////////////////////////////////////////////////////////////////////////
    INT16U ReadRadioRegister(
        INT16U  address
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetMacRegion
    // Description: Retrieves the MAC's region of operation.
    // Parameters:  pRegionConfig - pointer to structure that receives the
    //                region configuration information.  If this information is
    //                not desired, may be NULL.
    // Returns:     The current region of operation.
    ////////////////////////////////////////////////////////////////////////////
    RFID_MAC_REGION GetMacRegion(
        void*   pRegionConfig
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        SetGpioPinsConfiguration
    // Description: Sets the GPIO pin configuration for the radio
    // Parameters:  mask - specifies which pins (bit 0 = pin 0, etc.) are to be
    //                configured.  The presence of a 1 bit indicates that the
    //                pin is to be configured.
    //              configuration - the configuration for the pins specified by
    //                the mask.  A 1 bit in the corrpesponding location 
    //                indicates the pin is an output pin.  A 0 bit indicates an
    //                input pin.
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void SetGpioPinsConfiguration(
        INT32U  mask,
        INT32U  configuration
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetGpioPinsConfiguration
    // Description: Retrieves the GPIO pin configuration for the radio
    // Parameters:  None
    // Returns:     The configuration for the GPIO pins.  Bit 0 = pin 0, etc.  
    //              The presence of a 1 bit indicates an output pin.  The
    //              presence of a 0 bit indicates an input pin.
    ////////////////////////////////////////////////////////////////////////////
    INT32U GetGpioPinsConfiguration();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ReadGpioPins
    // Description: Retrieves the state for the requested GPIO pins
    // Parameters:  mask - specifies which pins (bit 0 = pin 0, etc.) are to be
    //                read.  The presence of a 1 bit indicates that a pin is to
    //                be read.  A 1-bit in a bit corresponding to an output pin
    //                results in 
    //                RfidErrorException(RFID_ERROR_INVALID_PARAMETER) being
    //                thrown.
    // Returns:     The state of the requested GPIO pins.
    ////////////////////////////////////////////////////////////////////////////
    INT32U ReadGpioPins(
        INT32U  mask
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        WriteGpioPins
    // Description: Sets the state for the requested GPIO pins
    // Parameters:  mask - specifies which pins (bit 0 = pin 0, etc.) are to be
    //                written.  The presence of a 1 bit indicates that a pin is to
    //                be read.  A 1-bit in a bit corresponding to an input pin
    //                results in 
    //                RfidErrorException(RFID_ERROR_INVALID_PARAMETER) being
    //                thrown.
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void WriteGpioPins(
        INT32U  mask,
        INT32U  value
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ToggleCarrierWave
    // Description: Turns the radio's carrier wave on or off
    // Parameters:  toggleOn - true if carrier wave is to be toggled on, false
    //                if it is to be toggled off
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ToggleCarrierWave(
        bool toggleOn
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        IssueCommand
    // Description: Issues the command to the radio object
    // Parameters:  command - the command to execute on the radio
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void IssueCommand(
        INT32U  command);

    ////////////////////////////////////////////////////////////////////////////////
    // Name:        RandomCarrierWave
    // Description: Sets up  the CW duration register and issues the random 
    //              CW command to the radio object
    // Paramters:   duration - how long to transmit random CW in milliseconds
    //                         0 means trasnmit continuously
    ////////////////////////////////////////////////////////////////////////////////
    void Radio::RandomCarrierWave(
        INT32U  duration);


    ////////////////////////////////////////////////////////////////////////////
    // Name:        SetImpinjExtensions
    // Description: Sets the Impinj Extensions parameters
    // Parameters:  pExtensions - a pointer to the Impinj Extensions parameters
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Radio::SetImpinjExtensions(
        const RFID_IMPINJ_EXTENSIONS*     pExtensions);

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetImpinjExtensions
    // Description: Retrieves the query tag group parameters
    // Parameters:  pGroup - a pointer to structure that will receive the query
    //              tag group parameters
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Radio::GetImpinjExtensions(
        RFID_IMPINJ_EXTENSIONS*     pExtensions);

private:
    // A pointer to the Mac object for this radio object
    std::auto_ptr<Mac>          m_pMac;
    // A flag to indicate if the radio has been previously closed
    bool                        m_isClosed;
    // A flag to indicate if the radio is currently busy
    bool                        m_isBusy;
    // A flag used to indicate that a cancel was issued
    bool                        m_shouldCancel;
    // A flag used to indicate that an abort was issued
    bool                        m_shouldAbort;
    // A flag used to indicate if the operation was truly cancelled
    bool                        m_operationCancelled;
    // A flag used to indicate if the radio firmware is a pre-2.2 version
    bool                        m_preTwoTwoFirmware;
    // A flag used to indicate if the radio firmware is a pre-2.4 version
    bool                        m_preTwoFourFirmware;
    // If the radio is busy, this is the ID of the thread that is executing the
    // tag-protocol operation
    CPL_ThreadID                m_busyThread;
    // A lock used to synchronize the cancel/abort so that we know that it has
    // completed
    CPL_Mutex                   m_cancelAbortLock;
    // A wrapper around the mutex so that it is automatically cleaned up
    CplMutexAutoHandle          m_cancelAbortLockWrapper;
    // The number of bytes that were still waiting for retrieval on the last
    // time we retrieved bytes from the transport layer
    INT32U                      m_bytesAvailable;

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Start18K6CRequest
    // Description: Performs the generic configuration setting needed for
    //              supporting the ISO 18000-6C tag operations.
    // Parameters:  pParms - a pointer to a structure that specifies the
    //              common parameters for the operations
    //              flags - flags that control the execution of the operation
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Start18K6CRequest(
        const RFID_18K6C_COMMON_PARMS* pParms,
        INT32U                         flags
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        PostMacCommandIssue
    // Description: Performs any post-request work needed for the issuing of a
    //              command to the MAC.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void PostMacCommandIssue();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        WriteMacMaskRegisters
    // Description: Writes the MAC mask registers (select or post-singulation).
    // Parameters:  registerAddress - the address of the first mask register
    //              bitCount - the number of bits in the mask
    //              pMask - a pointer to an array that contains the mask
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void WriteMacMaskRegisters(
        INT16U          registerAddress,
        INT32U          bitCount,
        const INT8U*    pMask
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ReadMacMaskRegisters
    // Description: Reads the MAC mask registers (select or post-singulation).
    // Parameters:  registerAddress - the address of the first mask register
    //              bitCount - the number of bits in the mask
    //              pMask - a pointer to an array that will receive the mask
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ReadMacMaskRegisters(
        INT16U  registerAddress,
        INT32U  bitCount,
        INT8U*  pMask
        );

    typedef std::vector<INT8U>  PACKET_BUFFER;

    ////////////////////////////////////////////////////////////////////////////
    // Name:        RetrieveNextPacket
    // Description: Retrieves the next packet from the MAC.
    // Parameters:  bufferSize - on return contains the number of bytes in the
    //                buffer
    //              buffer - on return contains the packet
    //              canBeCancelled - a flag to indicate if call can be cancelled
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void RetrieveNextPacket(
        INT32U          &bufferSize,
        PACKET_BUFFER   &buffer,
        bool            canBeCancelled
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        RetrieveBuffer
    // Description: Retrieves a buffer of the specified size from the MAC.
    // Parameters:  bufferSize - the number of bytes to retrieve
    //              buffer - on return contains the bytes
    //              canBeCancelled - a flag to indicate if call can be cancelled
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void RetrieveBuffer(
        INT32U  bufferSize,
        INT8U*  buffer,
        bool    canBeCancelled
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        RetrieveRawBytes
    // Description: Retrieves raw bytes from the MAC.  Also will indicate how
    //              many bytes are remaining for retrieval without blocking.
    // Parameters:  bufferSize - the number of bytes to retrieve.  If this value
    //                is non-zero, then blocks until that many bytes are
    //                retrieved.  If this value is zero, then simply determines
    //                how many bytes are available for retrieval.
    //              buffer - if bufferSize is nonzero, on return contains the
    //                bytes retrieved.  If bufferSize is zero, this can be NULL.
    //              canBeCancelled - a flag to indicate if call can be cancelled
    // Returns:     The number of bytes that are available to retrieve without
    //              blocking.
    ////////////////////////////////////////////////////////////////////////////
    INT32U RetrieveRawBytes(
        INT32U  bufferSize,
        INT8U*  buffer,
        bool    canBeCancelled
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        FastCrc32
    // Description: Calculates the CRC for the NV Update packets
    // Parameters:  sum - the current value of sum
    //              p   - byte pointer to the data to be summed
    //              len - the number of bytes to sum starting from p
    // Returns:     The updated value of sum
    ////////////////////////////////////////////////////////////////////////////
    INT32U FastCrc32(INT32U sum, unsigned char *p, INT32U len);

    ////////////////////////////////////////////////////////////////////////////
    // Name:        PacketCallbackFunction
    // Description: The callback that is invoked when a command-response packet
    //              is received.  This is only for processing packets that the
    //              radio object intercepts on the application's behalf.
    // Parameters:  handle - the handle to the radio (not used)
    //              bufferLength - the length, in bytes, of the packet buffer
    //              pBuffer - pointer to the packet buffer
    //              context - context for the call.  In this case, it is a
    //                PACKET_CALLBACK_DATA structure pointer.
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    static INT32S RFID_CALLBACK PacketCallbackFunction(
        RFID_RADIO_HANDLE   handle,
        INT32U              bufferLength,
        const INT8U*        pBuffer,
        void*               context
        );

    // The following are states that the radio may be in for processing MAC 
    // command-response packets that will be processed by the radio (versus
    // passed up to the application
    typedef enum
    {
        RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN,
        RFID_RADIO_STATE_EXPECT_COMMAND_END,
        RFID_RADIO_STATE_EXPECT_MBP_READ_REG,
        RFID_RADIO_STATE_EXPECT_GPIO_READ,
        RFID_RADIO_STATE_EXPECT_OEMCFG_READ,
        RFID_RADIO_STATE_EXPECT_NVMEMUPDCFG_READ,
        RFID_RADIO_STATE_EXPECT_LINKPROFILE_READ_REG
    } RFID_RADIO_STATE;

    // The following structure is used when invoking the packet-processing
    // callback for internally-processed packets (i.e., those packets that we
    // don't want to give to the application).
    typedef struct
    {
        // A pointer to the radio object that issued the command that resulted
        // in the packets
        Radio*              pRadio;
        // The current state of the packet processing
        RFID_RADIO_STATE    state;
        // The command which caused the packets
        INT32U              command;
        // The status as reported by the MAC
        INT32U              status;
        // For some commands, there is additional data that will come back.
        union
        {
            // The command-response data for the OEM Read
            struct
            {
                // The OEM address that was read
                INT32U  address;
                // The data at that address
                INT32U  value;
            } oemRead;
            // The command-reponse data for the MAC by-pass register read
            struct
            {
                // The address of the register read
                INT16U  address;
                // The data at that address (i.e., the register value)
                INT16U  value;
            } macByPassRead;
            // The command-response data for the GPIO read
            struct
            {
                // The values of the GPIO pins
                INT32U  value;
            } gpioRead;
            // The command-response data for the nonvolatile memory update
            struct
            {
                // The maximum packet size
                INT32U  maxPacketSize;
            } nvMemUpdate;
            // The command-response data for the link profile register read
            struct
            {
                // The ilnk profile
                INT16U  profile;
                // The address of the register read
                INT16U  address;
                // The data read from the register
                INT16U  data;
            } linkProfileRead;
        } commandData;
    } PACKET_CALLBACK_DATA;

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ProcessMacPacket
    // Description: Used to process a MAC command-response packet.  This, unlike
    //              the packet callback is a member function.
    // Parameters:  pData - pointer to packet callback data
    //              pPacket - pointer to a command-response packet
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ProcessMacPacket(
        PACKET_CALLBACK_DATA*       pData,
        const RFID_PACKET_COMMON*   pPacket
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ProcessCommandBeginPacket
    // Description: Used to process a command-begin packet.
    // Parameters:  pData - pointer to packet callback data
    //              pPacket - pointer to a command-begin packet
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ProcessCommandBeginPacket(
        PACKET_CALLBACK_DATA*               pData,
        const RFID_PACKET_COMMAND_BEGIN*    pPacket
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ProcessCommandEndPacket
    // Description: Used to process a command-end packet.
    // Parameters:  pData - pointer to packet callback data
    //              pPacket - pointer to a command-end packet
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ProcessCommandEndPacket(
        PACKET_CALLBACK_DATA*           pData,
        const RFID_PACKET_COMMAND_END*  pPacket
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ProcessOemcfgReadPacket
    // Description: Used to process a command-end packet.
    // Parameters:  pData - pointer to packet callback data
    //              pPacket - pointer to a OEM config read packet
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ProcessOemcfgReadPacket(
        PACKET_CALLBACK_DATA*           pData,
        const RFID_PACKET_OEMCFG_READ*  pPacket
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ProcessMbpReadRegPacket
    // Description: Used to process a command-end packet.
    // Parameters:  pData - pointer to packet callback data
    //              pPacket - pointer to a MAC bypass register read packet
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ProcessMbpReadRegPacket(
        PACKET_CALLBACK_DATA*       pData,
        const RFID_PACKET_MBP_READ* pPacket
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ProcessGpioReadPacket
    // Description: Used to process a command-end packet.
    // Parameters:  pData - pointer to packet callback data
    //              pPacket - pointer to a GPIO read packet
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ProcessGpioReadPacket(
        PACKET_CALLBACK_DATA*           pData,
        const RFID_PACKET_GPIO_READ*    pPacket
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ProcessNonvolatileMemoryUpdateConfigPacket
    // Description: Used to process a nonvolatile memory update config packet.
    // Parameters:  pData - pointer to packet callback data
    //              pPacket - pointer to a nonvolatile memory update config
    //              packet
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ProcessNonvolatileMemoryUpdateConfigPacket(
        PACKET_CALLBACK_DATA*           pData,
        const RFID_PACKET_NVMEMUPDCFG*  pPacket
        );


    ////////////////////////////////////////////////////////////////////////////
    // Name:        ProcessLinkProfileRegisterReadPacket
    // Description: Used to process a link profile register read packet.
    // Parameters:  pData - pointer to packet callback data
    //              pPacket - pointer to a link profile register read packet
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ProcessLinkProfileRegisterReadPacket(
        PACKET_CALLBACK_DATA*           pData,
        const RFID_PACKET_LPROF_READ*   pPacket
        );


    ////////////////////////////////////////////////////////////////////////////////
    // Name:        Setup18K6CReadRegisters
    // Description: Configures the tag read specific registers in the radio module.
    // Parameters:  pParms - pointer the read command parameters 
    ////////////////////////////////////////////////////////////////////////////////
    void Radio::Setup18K6CReadRegisters(
        const RFID_18K6C_READ_CMD_PARMS*        pParms
        );


    ////////////////////////////////////////////////////////////////////////////////
    // Name:        Setup18K6CWriteRegisters
    // Description: Configures the tag write specific registers in the radio module.
    // Parameters:  writeType - The discriminator for the pParms pointer (what type of
    //                          write command parameters are provided
    //              pParms - pointer to the write command parameters
    ////////////////////////////////////////////////////////////////////////////////
    void Radio::Setup18K6CWriteRegisters(
        RFID_18K6C_WRITE_TYPE writeType,
        const void*        pParms
        );


    ////////////////////////////////////////////////////////////////////////////////
    // Name:        Setup18K6CBlockEraseRegisters
    // Description: Configures the tag block erase specific registers in the radio 
    //               module.
    // Parameters:  pParms - pointer the block erase command parameters 
    ////////////////////////////////////////////////////////////////////////////////
    void Radio::Setup18K6CBlockEraseRegisters(
        const RFID_18K6C_BLOCK_ERASE_CMD_PARMS*        pParms
        );



    // Don't really want radio objects to be copied
    Radio(const Radio&);
    const Radio& operator = (const Radio&);
}; // class Radio

} // namespace rfid

#endif // #ifndef RADIO_H_INCLUDED
