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
 * $Id: radio.cpp 70427 2011-08-05 23:10:53Z dciampi $
 * 
 * Description:
 *     This file contains the implementation for the radio objects.
 *     
 *
 *****************************************************************************
 */

#include <vector>
#include <assert.h>
#include "radio.h"
#include "mac.h"
#include "macregs.h"
#include "maccmds.h"
#include "macerror.h"
#include "rfid_exceptions.h"
#include "hostpkts.h"
#include "compat_lib.h"
#include "compat_time.h"
#include "auto_lock_compat.h"
#include "auto_handle_compat.h"
#include "rfid_extern.h"
#include "tracer.h"
#include "nvmemupd.h"


// Constants we don't want to see outside this file
namespace
{
const INT32U BITS_PER_BYTE                    = 8;
const INT32U BYTES_PER_REGISTER               = 4;
const INT32U BITS_PER_REGISTER                = BITS_PER_BYTE * BYTES_PER_REGISTER;
const INT32U MAC_SLEEP_MILLIS                 = 10;
const INT32U RFID_NUM_TAGWRDAT_REGS_PER_BANK  = 16;


const INT32U crc32_table[256] = 
    {
      0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
      0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005, 
      0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 
      0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 
      0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 
      0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
      0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 
      0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd, 
      0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039, 
      0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 
      0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
      0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d, 
      0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 
      0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95, 
      0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 
      0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
      0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 
      0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072, 
      0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 
      0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca, 
      0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
      0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 
      0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066, 
      0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba, 
      0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 
      0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
      0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6, 
      0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 
      0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 
      0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2, 
      0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
      0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a, 
      0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637, 
      0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 
      0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 
      0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
      0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 
      0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b, 
      0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 
      0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 
      0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
      0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b, 
      0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 
      0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3, 
      0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 
      0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
      0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 
      0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3, 
      0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 
      0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c, 
      0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
      0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 
      0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 
      0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec, 
      0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 
      0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
      0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 
      0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 
      0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 
      0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4, 
      0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
      0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c, 
      0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668, 
      0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4 
    };

    #define MAX_NV_ATTEMPTS 2
} // namespace

namespace rfid
{
////////////////////////////////////////////////////////////////////////////////
// Name:        Radio
// Description: Initializes the radio object.
////////////////////////////////////////////////////////////////////////////////
Radio::Radio(
    std::auto_ptr<Mac> pMac
    ) :
    m_pMac(pMac),
    m_isClosed(false),
    m_isBusy(false),
    m_shouldCancel(false),
    m_shouldAbort(false),
    m_operationCancelled(false),
    m_preTwoTwoFirmware(false),
    m_preTwoFourFirmware(false),
    m_bytesAvailable(0)
{
    INT32U  result;
    INT32U  macInfo;

    assert(NULL != m_pMac.get());

    // Create the lock that is used to synchronize access to the state of the
    // radio object and then wrap it so that it will be automatically cleaned up
    result = CPL_MutexInit(&m_cancelAbortLock);
    if (result)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Failed to create mutex.  Result = 0x%.8x\n",
            __FUNCTION__,
            result);
        throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
    }
    m_cancelAbortLockWrapper.Assume(&m_cancelAbortLock);

    // To ensure that the MAC is in an idle state, issue a cancel that will
    // result in the driver giving it an abort
    m_pMac->AbortOperation();


    // The following logic leverages the fact the MAC_INFO prior to 2.2 MAC firmware
    // was always 0, and subsequent to 2.2 at a minimum indicates R1000/R2000 target
    // build as non-zero values in bits 3:0.  Ideally, we would prefer to use the 
    // MAC_VER register for this, but since MAC_VER format changed with 2.2, this is
    // a cleaner solution.
    macInfo = m_pMac->ReadRegister(MAC_INFO);
    if (0 == macInfo)
    {
        m_preTwoTwoFirmware = true;
    }


    // The following logic leverages the fact the MAC_ACTIVE_FW prior to 2.4 MAC firmware
    // did not exist, so the read will throw an excpetion if it is not 2.4 firmware.
    // Ideally, we would prefer to use the MAC_VER register for this, but since MAC_VER 
    // format changed with 2.2, this is a cleaner solution, for backward compatibility.
    //
    try
    {
        result = m_pMac->ReadRegister(MAC_ACTIVE_FW);
    }
    catch (rfid::RfidErrorException& error)
    {
        if (error.GetError() == RFID_ERROR_INVALID_PARAMETER)
        {
            m_preTwoFourFirmware = true;
            this->ClearMacError();
        }
        else
        {
            // propagate all other exceptions
            throw;
        }
    }
} // Radio::Radio

////////////////////////////////////////////////////////////////////////////////
// Name:        ~Radio
// Description: Cleans up the radio object.
////////////////////////////////////////////////////////////////////////////////
Radio::~Radio()
{
    // Ensure that the radio has been closed
    try
    {
        this->Close();
    }
    catch (...)
    {
    }
} // Radio::~Radio

////////////////////////////////////////////////////////////////////////////////
// Name:        Close
// Description: Requests that the radio object close the underlying radio
//              module.
////////////////////////////////////////////////////////////////////////////////
void Radio::Close()
{
    if (!m_isClosed)
    {
        // Cancel any operations that are still in progress and then get rid of
        // the MAC object
        this->AbortOperation();
        m_pMac.reset();

        // Indicate that the radio is closed
        m_isClosed = true;
    }
} // Radio::Close

////////////////////////////////////////////////////////////////////////////////
// Name:        WriteMacRegister
// Description: Requests to set a low-level radio module register
////////////////////////////////////////////////////////////////////////////////
void Radio::WriteMacRegister(
    INT16U  address,
    INT32U  value
    )
{
    INT32U macError;

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // We cannot allow the command register to be written
    if (HST_CMD == address)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Reject request to write MAC's HST_CMD virtual register\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    m_pMac->WriteRegister(address, value); // Generic Write

    macError = m_pMac->ReadRegister(MAC_ERROR);
    if (MACERR_SUCCESS != macError)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: WriteRegister generated MAC error %d\n",
            __FUNCTION__,
            macError);

        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }
} // Radio::WriteMacRegister

////////////////////////////////////////////////////////////////////////////////
// Name:        ReadMacRegister
// Description: Requests a retrieval of  a low-level radio module
//              register.
////////////////////////////////////////////////////////////////////////////////
INT32U Radio::ReadMacRegister(
    INT16U  address
    )
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    return m_pMac->ReadRegister(address); // Generic Read
} // Radio::ReadMacRegister


////////////////////////////////////////////////////////////////////////////////
// Name:        ReadMacRegisterInfo
// Description: Requests a retrieval of  a low-level radio module
//              register info.
////////////////////////////////////////////////////////////////////////////////
void Radio::ReadMacRegisterInfo(
    INT16U                            address,
    RFID_REGISTER_INFO                *pInfo
    )
{
    INT32U                            paramValue   = 0;
    INT32U                            regInfoValue = 0;
    RFID_REGISTER_INFO                selectorInfo;

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // prior to 2.2 firmware, the info concept is not supported, so just 
    // throw invalid parameter
    //
    if (m_preTwoTwoFirmware)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Feature not supported by MAC firmware prior to 2.2\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    // default init the info structure
    
    pInfo->type            = RFID_REGISTER_NORMAL;
    pInfo->bankSize        = 0;
    pInfo->selectorAddress = 0;
    pInfo->currentSelector = 0;
    pInfo->reserved        = 0;
    pInfo->accessType      = RFID_REGISTER_ACCESS_RW;

    // For HST_CMD, we can't get reginfo but the default init values are correct, so
    // only process if the request address is not HST_CMD
    if (HST_CMD == address)
    {
        return;
    }

    // set the reginfo request bit in the paremeter and request it again
    address = address | (1 << 12);

    regInfoValue = m_pMac->ReadRegister(address); // Generic Read

    // now decode the regInfoValue into the pInfo return structure

    // type is low 4 bits of 1st byte
    pInfo->type = (RFID_REGISTER_TYPE) ((regInfoValue >> 0 ) & 0xF);  

    // readOnly is high bit of 1st byte
    if ((regInfoValue >> 0x07) & 0x01)
    {
        pInfo->accessType =  RFID_REGISTER_ACCESS_RO;
    }
    else if ((regInfoValue >> 0x06) & 0x01)
    {
        pInfo->accessType = RFID_REGISTER_ACCESS_WO;
    }


    // bankSize is 2nd byte (may be updated again below, if this is a banked register)
    pInfo->bankSize = (regInfoValue >> 8) & 0xFF;   

    // if it's a banked register, go read the current selector
    if (RFID_REGISTER_BANKED == pInfo->type)
    {
        // upper 2 bytes contain selector offset so need to combine with high nibble of original input
        pInfo->selectorAddress = ((regInfoValue >> 16) & 0xFFFF) | ((INT32U)(address & 0x0F00));   

        pInfo->currentSelector = m_pMac->ReadRegister(pInfo->selectorAddress ); // Generic Read

        // recursion alert - go get the the reginfo on the selector for the banked
        // register, so we can populate bankSize
        this->ReadMacRegisterInfo(pInfo->selectorAddress, &selectorInfo);

        pInfo->bankSize = selectorInfo.bankSize;            
    }
    else if (RFID_REGISTER_SELECTOR == pInfo->type)
    {
        // the inital paramValue is what we need here             
        pInfo->currentSelector = paramValue;                
    }
} // Radio::ReadMacRegisterInfo



////////////////////////////////////////////////////////////////////////////////
// Name:        ReadMacBankedRegister
// Description: Requests a retrieval of  a low-level radio module
//              banked register.
////////////////////////////////////////////////////////////////////////////////
INT32U Radio::ReadMacBankedRegister(
    INT16U  address,
    INT16U  bankSelector
    )
{
    RFID_REGISTER_INFO                info;
    INT32U                            selector;

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // first get register info, and confirm this is a banked register
    this->ReadMacRegisterInfo(address, &info);
    if (RFID_REGISTER_BANKED != info.type)
    {        
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Reject request to get banked register on non-banked address\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    selector = info.selectorAddress;
    // then get register info for the selector
    this->ReadMacRegisterInfo(selector, &info);

    // confirm the bankSelector is less than the bankSize for the selector
    if (bankSelector >= info.bankSize)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Reject request to get banked register, over indexed bank selector=%d, bankSize=%d\n",
            __FUNCTION__, bankSelector, info.bankSize);
        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    // write the selector
    m_pMac->WriteRegister(selector, bankSelector); // Generic Write

    // read and return the requested parameter
    return m_pMac->ReadRegister(address); // Generic Read

} // Radio::ReadMacBankedRegister

////////////////////////////////////////////////////////////////////////////////
// Name:        WriteMacBankedRegister
// Description: Requests to set a low-level radio module banked register
////////////////////////////////////////////////////////////////////////////////
void Radio::WriteMacBankedRegister(
    INT16U  address,
    INT16U  bankSelector,
    INT32U  value
    )
{
    RFID_REGISTER_INFO                info;
    INT32U                            selector;

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // first get register info, and confirm this is a banked register
    this->ReadMacRegisterInfo(address, &info);
    if (RFID_REGISTER_BANKED != info.type)
    {        
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Reject request to set banked register on non-banked address\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    selector = info.selectorAddress;
    // then get registerinfo for the selector
    this->ReadMacRegisterInfo(selector, &info);

    // confirm the bankSelector is less than the bankSize for the selector
    if (bankSelector >= info.bankSize)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Reject request to set banked register, over indexed bank selector=%d, bankSize=%d\n",
            __FUNCTION__, bankSelector, info.bankSize);
        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    // write the selector
    m_pMac->WriteRegister(selector, bankSelector); // Generic Write

    // write the register
    m_pMac->WriteRegister(address, value); // Generic Write

} // Radio::WriteMacBankedRegister


////////////////////////////////////////////////////////////////////////////////
// Name:        SetOperationMode
// Description: Sets the operation mode for the radio
////////////////////////////////////////////////////////////////////////////////
void Radio::SetOperationMode(
    RFID_RADIO_OPERATION_MODE   mode
    )
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Set the antenna cycles register to either perform a single cycle or to
    // cycle until a cancel
    INT32U registerValue = 
        HST_ANT_CYCLES_CYCLES(RFID_RADIO_OPERATION_MODE_CONTINUOUS == mode ?
                              HST_ANT_CYCLES_CYCLES_INFINITE : 1)   |
        HST_ANT_CYCLES_RFU1(0);
                            
    m_pMac->WriteRegister(HST_ANT_CYCLES, registerValue);
} // Radio::SetOperationMode

////////////////////////////////////////////////////////////////////////////////
// Name:        Radio::GetOperationMode
// Description: Retrieves the operation mode for the radio
////////////////////////////////////////////////////////////////////////////////
RFID_RADIO_OPERATION_MODE Radio::GetOperationMode()
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Retrieve the antenna cycles register and return the appropriate type
    return (HST_ANT_CYCLES_GET_CYCLES(m_pMac->ReadRegister(HST_ANT_CYCLES)) ==
            HST_ANT_CYCLES_CYCLES_INFINITE) ?
            RFID_RADIO_OPERATION_MODE_CONTINUOUS :
            RFID_RADIO_OPERATION_MODE_NONCONTINUOUS;
} // Radio::GetOperationMode

////////////////////////////////////////////////////////////////////////////////
// Name:        SetPowerState
// Description: Sets the radio's power state
////////////////////////////////////////////////////////////////////////////////
void Radio::SetPowerState(
    RFID_RADIO_POWER_STATE  state
    )
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    g_pTracer->PrintMessage(
        Tracer::RFID_LOG_SEVERITY_DEBUG,
        "%s: Change the power state of radio to %u\n",
        __FUNCTION__,
        state);

    // Set the appropriate power state on the MAC
    m_pMac->WriteRegister(HST_PWRMGMT,
        HST_PWRMGMT_MODE(state) |
        HST_PWRMGMT_RFU1(0));
    m_pMac->WriteRegister(HST_CMD, CMD_SETPWRMGMTCFG);

    // Process the MAC's command-response packets
    PACKET_CALLBACK_DATA callbackData;
    callbackData.pRadio  = this;
    callbackData.command = CMD_SETPWRMGMTCFG;
    callbackData.state   = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;

    this->ProcessOperationData(RFID_INVALID_RADIO_HANDLE,
                               Radio::PacketCallbackFunction,
                               &callbackData,
                               NULL,
                               false);

    // If the command failed, clear the MAC error and return an error
    if (callbackData.status)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Setting radio power state failed with status 0x%.8x\n",
            __FUNCTION__,
            callbackData.status);

        this->ClearMacError();
        throw RfidErrorException(
            CSM_ERR_INVAL_PWRMODE == callbackData.status ?
                RFID_ERROR_INVALID_PARAMETER :
                RFID_ERROR_RADIO_FAILURE, __FUNCTION__);
    }
} // Radio::SetPowerState

////////////////////////////////////////////////////////////////////////////////
// Name:        GetPowerState
// Description: Retrieves the radio's power state
////////////////////////////////////////////////////////////////////////////////
RFID_RADIO_POWER_STATE Radio::GetPowerState()
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    RFID_RADIO_POWER_STATE state = 0;

    // Retrieve the power mode
    switch (HST_PWRMGMT_GET_MODE(m_pMac->ReadRegister(HST_PWRMGMT)))
    {
        case HST_PWRMGMT_MODE_NORMAL:
        {
            state = RFID_RADIO_POWER_STATE_FULL;
            break;
        } // case HST_PWRMGMT_MODE_NORMAL
        case HST_PWRMGMT_MODE_LOWPOWER_STANDBY:
        {
            state = RFID_RADIO_POWER_STATE_STANDBY;
            break;
        } // case HST_PWRMGMT_MODE_NORMAL
        default:
        {
            throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            break;
        } // default
    } // switch (HST_PWRMGMT_GET_MODE(m_pMac->ReadRegister(HST_PWRMGMT))

    return state;
} // Radio::GetPowerState

////////////////////////////////////////////////////////////////////////////
// Name:        SetCurrentLinkProfile
// Description: Sets the radio's current link profile
////////////////////////////////////////////////////////////////////////////
void Radio::SetCurrentLinkProfile(
    INT32U  profile
    )
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Read off the current profile just in case we need to restore it
    INT32U currentProfile = m_pMac->ReadRegister(HST_RFTC_CURRENT_PROFILE);

    try
    {
        // Write the current profile register with the appropriate value and
        // then instruct the MAC to set the current profile (by issuing the
        // appropriate command)
        INT32U registerValue = currentProfile;
        HST_RFTC_PROF_CURRENTPROF_SET_PROF(registerValue, profile);
        m_pMac->WriteRegister(HST_RFTC_CURRENT_PROFILE, registerValue);
        m_pMac->WriteRegister(HST_CMD, CMD_UPDATELINKPROFILE);

        // Process the MAC's command-response packets
        PACKET_CALLBACK_DATA callbackData;
        callbackData.pRadio  = this;
        callbackData.command = CMD_UPDATELINKPROFILE;
        callbackData.state   = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;

        this->ProcessOperationData(RFID_INVALID_RADIO_HANDLE,
                                   Radio::PacketCallbackFunction,
                                   &callbackData,
                                   NULL,
                                   false);

        // If the command failed, clear the MAC error and return an error
        if (callbackData.status)
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Setting radio current link profile failed with status "
                "0x%.8x\n",
                __FUNCTION__,
                callbackData.status);

            this->ClearMacError();
            throw RfidErrorException(
                HOSTIF_ERR_SELECTORBNDS == callbackData.status ?
                    RFID_ERROR_INVALID_PARAMETER : 
                    RFID_ERROR_RADIO_FAILURE, __FUNCTION__);
        }
    }
    catch (...)
    {
        // Restore the current profile register
        try
        {
            // Restore the previous profile
            m_pMac->WriteRegister(HST_RFTC_CURRENT_PROFILE, currentProfile);
        }
        catch(...)
        {
        }

        throw;
    }
} // Radio::SetCurrentLinkProfile

////////////////////////////////////////////////////////////////////////////
// Name:        GetCurrentLinkProfile
// Description: Retrieves the radio's current link profile
////////////////////////////////////////////////////////////////////////////
INT32U Radio::GetCurrentLinkProfile()
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Simply return the value in the current profile register
    return
        HST_RFTC_PROF_CURRENTPROF_GET_PROF(
            m_pMac->ReadRegister(HST_RFTC_CURRENT_PROFILE));
} // Radio::GetCurrentLinkProfile

////////////////////////////////////////////////////////////////////////////
// Name:        GetLinkProfile
// Description: Retrieves the link profile information for the profile
//              specified
////////////////////////////////////////////////////////////////////////////
void Radio::GetLinkProfile(
    INT32U                      profile,
    RFID_RADIO_LINK_PROFILE*    pProfileInfo
    )
{
    INT32U  registerValue;

    assert(NULL != pProfileInfo);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Set the profile selector and verify that it was a valid selector
    m_pMac->WriteRegister(HST_RFTC_PROF_SEL,
                          HST_RFTC_PROF_SEL_PROF(profile) |
                          HST_RFTC_PROF_SEL_RFU1(0));
    if (HOSTIF_ERR_SELECTORBNDS == m_pMac->ReadRegister(MAC_ERROR))
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Link profile %u is invalid\n",
            __FUNCTION__,
            profile);

        this->ClearMacError();
        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    // Get the config information for the profile
    registerValue                   = 
        m_pMac->ReadRegister(MAC_RFTC_PROF_CFG);
    pProfileInfo->enabled           = 
        MAC_RFTC_PROF_CFG_IS_ENABLED(registerValue);
    pProfileInfo->denseReaderMode   =
        MAC_RFTC_PROF_CFG_DRM_IS_ENABLED(registerValue);

    // Retrieve the profile's ID
    INT64U profileIdHigh    = m_pMac->ReadRegister(MAC_RFTC_PROF_ID_HIGH);
    INT64U profileIdLow     = m_pMac->ReadRegister(MAC_RFTC_PROF_ID_LOW);
    pProfileInfo->profileId = (profileIdHigh << 32) | profileIdLow;

    // Retrieve the profile's version
    pProfileInfo->profileVersion = m_pMac->ReadRegister(MAC_RFTC_PROF_IDVER);

    // Get the protocol for the link profile and then based upon the protocol,
    // Retrieve the appropriate information
    pProfileInfo->profileProtocol = 
        m_pMac->ReadRegister(MAC_RFTC_PROF_PROTOCOL);
    
    // Read the RSSI information for the profile
    registerValue                               =
        m_pMac->ReadRegister(MAC_RFTC_PROF_RSSIAVECFG);
    pProfileInfo->widebandRssiSamples           =
        RFID_WIDEBAND_RSSI_BASE_SAMPLES <<
            MAC_RFTC_PROF_RSSIAVECFG_GET_NORM_WBSAMPS(registerValue);
    pProfileInfo->narrowbandRssiSamples         =
        RFID_NARROWBAND_RSSI_BASE_SAMPLES <<
            MAC_RFTC_PROF_RSSIAVECFG_GET_NORM_NBSAMPS(registerValue);
    pProfileInfo->realtimeRssiEnabled           =
        MAC_RFTC_PROF_RSSIAVECFG_RT_IS_ENABLED(registerValue);
    pProfileInfo->realtimeWidebandRssiSamples   =
        MAC_RFTC_PROF_RSSIAVECFG_GET_RT_WBSAMPS(registerValue);
    pProfileInfo->realtimeNarrowbandRssiSamples =
        MAC_RFTC_PROF_RSSIAVECFG_GET_RT_NBSAMPS(registerValue);

    switch (pProfileInfo->profileProtocol)
    {
        case RFID_RADIO_PROTOCOL_ISO18K6C:
        {
            RFID_RADIO_LINK_PROFILE_ISO18K6C_CONFIG* pConfig =
                &(pProfileInfo->profileConfig.iso18K6C);

            pConfig->length             =
                sizeof(RFID_RADIO_LINK_PROFILE_ISO18K6C_CONFIG);
            pConfig->modulationType     =
                m_pMac->ReadRegister(MAC_RFTC_PROF_R2TMODTYPE);
            pConfig->tari               =
                m_pMac->ReadRegister(MAC_RFTC_PROF_TARI);
            pConfig->data01Difference   =
                m_pMac->ReadRegister(MAC_RFTC_PROF_X);
            pConfig->pulseWidth         =
                m_pMac->ReadRegister(MAC_RFTC_PROF_PW);
            pConfig->rtCalibration      =
                m_pMac->ReadRegister(MAC_RFTC_PROF_RTCAL);
            pConfig->trCalibration      =
                m_pMac->ReadRegister(MAC_RFTC_PROF_TRCAL);
            pConfig->divideRatio        =
                m_pMac->ReadRegister(MAC_RFTC_PROF_DIVIDERATIO);
            pConfig->millerNumber       =
                m_pMac->ReadRegister(MAC_RFTC_PROF_MILLERNUM);
            pConfig->trLinkFrequency    =
                m_pMac->ReadRegister(MAC_RFTC_PROF_T2RLINKFREQ);
            pConfig->varT2Delay         =
                m_pMac->ReadRegister(MAC_RFTC_PROF_VART2DELAY);
            pConfig->rxDelay            =
                m_pMac->ReadRegister(MAC_RFTC_PROF_RXDELAY);
            pConfig->minT2Delay         =
                m_pMac->ReadRegister(MAC_RFTC_PROF_MINTOTT2DELAY);
            pConfig->txPropagationDelay =
                m_pMac->ReadRegister(MAC_RFTC_PROF_TXPROPDELAY);
            break;
        } // case RFID_RADIO_PROTOCOL_ISO18K6C
        default:
        {
            break;
        } // default
    } // switch (pProfileInfo->profileProtocol)
} // Radio::GetLinkProfile

////////////////////////////////////////////////////////////////////////////
// Name:        WriteLinkProfileRegister
// Description: Sets the link-profile register for the profile specified
////////////////////////////////////////////////////////////////////////////
void Radio::WriteLinkProfileRegister(
    INT32U  profile,
    INT16U  address,
    INT16U  value
    )
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Tell the MAC which profile we are writing to, which profile link register
    // we want to write, what value to write to it, and then send the command
    m_pMac->WriteRegister(HST_LPROF_SEL, 
                          HST_LPROF_SEL_SELECTOR(profile)   |
                          HST_LPROF_SEL_RFU1(0));
    m_pMac->WriteRegister(HST_LPROF_ADDR,
                          HST_LPROF_ADDR_REGADDR(address)   |
                          HST_LPROF_ADDR_RFU1(0));
    m_pMac->WriteRegister(HST_LPROF_DATA,
                          HST_LPROF_DATA_REGDATA(value)     |
                          HST_LPROF_DATA_RFU1(0));
    m_pMac->WriteRegister(HST_CMD, CMD_LPROF_WRXCVRREG);

    // Process the MAC's command-response packets
    PACKET_CALLBACK_DATA callbackData;
    callbackData.pRadio  = this;
    callbackData.command = CMD_LPROF_WRXCVRREG;
    callbackData.state   = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;

    this->ProcessOperationData(RFID_INVALID_RADIO_HANDLE,
                               Radio::PacketCallbackFunction,
                               &callbackData,
                               NULL,
                               false);

    // If the command failed, clear the MAC error and return an error
    if (callbackData.status)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Write to link profile register failed with status 0x%.8x\n",
            __FUNCTION__,
            callbackData.status);


        this->ClearMacError();

        switch (callbackData.status)
        {
            case RFTC_ERR_LPROFBADSELECTOR:
            case RFTC_ERR_BADXCVRADDR:
            case RFTC_ERR_XCVRADDRNOTINLIST:
			case HOSTIF_ERR_SELECTORBNDS:
            {
                throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                break;
            } // Error status that result from invalid parameter
            default:
            {
                throw RfidErrorException(RFID_ERROR_RADIO_FAILURE, __FUNCTION__);
                break;
            }
        }
    }
} // Radio::WriteLinkProfileRegister

////////////////////////////////////////////////////////////////////////////
// Name:        ReadLinkProfileRegister
// Description: Retrieves the link-profile register for the profile
//              specified
////////////////////////////////////////////////////////////////////////////
INT16U Radio::ReadLinkProfileRegister(
    INT32U  profile,
    INT16U  address
    )
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Tell the MAC which profile and register we want to read and send the
    // command to read it
    m_pMac->WriteRegister(HST_LPROF_SEL, 
                          HST_LPROF_SEL_SELECTOR(profile)   |
                          HST_LPROF_SEL_RFU1(0));
    m_pMac->WriteRegister(HST_LPROF_ADDR,
                          HST_LPROF_ADDR_REGADDR(address)   |
                          HST_LPROF_ADDR_RFU1(0));
    m_pMac->WriteRegister(HST_CMD, CMD_LPROF_RDXCVRREG);

    // Process the MAC's command-response packets
    PACKET_CALLBACK_DATA callbackData;
    callbackData.pRadio  = this;
    callbackData.command = CMD_LPROF_RDXCVRREG;
    callbackData.state   = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;

    this->ProcessOperationData(RFID_INVALID_RADIO_HANDLE,
                               Radio::PacketCallbackFunction,
                               &callbackData,
                               NULL,
                               false);

    // If the command failed, clear the MAC error and return an error
    if (callbackData.status)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Read of link profile register failed with status 0x%.8x\n",
            __FUNCTION__,
            callbackData.status);

        this->ClearMacError();
        switch (callbackData.status)
        {
            case RFTC_ERR_LPROFBADSELECTOR:
            case RFTC_ERR_BADXCVRADDR:
            case RFTC_ERR_XCVRADDRNOTINLIST:
            {
                throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                break;
            } // Error status that result from invalid parameter
            default:
            {
                throw RfidErrorException(RFID_ERROR_RADIO_FAILURE, __FUNCTION__);
                break;
            }
        }
    }
    // Otherwise, make a sanity check to ensure that the profile and address of
    // the data returned matches the profile and address of the data requested
    else if ((callbackData.commandData.linkProfileRead.profile != profile) ||
             (callbackData.commandData.linkProfileRead.address != address))
    {
        throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
    }

    return callbackData.commandData.linkProfileRead.data;
} // Radio::ReadLinkProfileRegister

////////////////////////////////////////////////////////////////////////////////
// Name:        GetAntennaPortStatus
// Description: Retrieves the status for the specified antenna port
////////////////////////////////////////////////////////////////////////////////
void Radio::GetAntennaPortStatus(
    INT32U                      antennaPort,
    RFID_ANTENNA_PORT_STATUS*   pStatus
    )
{
    assert(NULL != pStatus);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // First, tell the MAC which antenna descriptors we'll be reading and
    // verify that it was a valid selector
    m_pMac->WriteRegister(HST_ANT_DESC_SEL, antennaPort);
    if (HOSTIF_ERR_SELECTORBNDS == m_pMac->ReadRegister(MAC_ERROR))
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Antenna port %u is invalid\n",
            __FUNCTION__,
            antennaPort);

        this->ClearMacError();
        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    // Get the state of the antenna
    pStatus->state =
        (HST_ANT_DESC_CFG_IS_ENABLED(m_pMac->ReadRegister(HST_ANT_DESC_CFG)) ?
         RFID_ANTENNA_PORT_STATE_ENABLED : RFID_ANTENNA_PORT_STATE_DISABLED);

    // Now read the anteanna sense value
    pStatus->antennaSenseValue =
        MAC_ANT_DESC_STAT_GET_RESIST(m_pMac->ReadRegister(MAC_ANT_DESC_STAT));
} // Radio::GetAntennaPortStatus

////////////////////////////////////////////////////////////////////////////////
// Name:        SetAntennaPortState
// Description: Sets the state of an antenna port
////////////////////////////////////////////////////////////////////////////////
void Radio::SetAntennaPortState(
    INT32U                  antennaPort,
    RFID_ANTENNA_PORT_STATE state
    )
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // First, tell the MAC which antenna descriptors we'll be reading and
    // verify that it was a valid selector
    m_pMac->WriteRegister(HST_ANT_DESC_SEL, antennaPort);
    if (HOSTIF_ERR_SELECTORBNDS == m_pMac->ReadRegister(MAC_ERROR))
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Antenna port %u is invalid\n",
            __FUNCTION__,
            antennaPort);

        this->ClearMacError();
        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    // Read the current value of the anteann port configuration
    INT32U registerValue = m_pMac->ReadRegister(HST_ANT_DESC_CFG);

    // Now set the enabled bit appropriately
    switch (state)
    {
        case RFID_ANTENNA_PORT_STATE_DISABLED:
        {
            HST_ANT_DESC_CFG_SET_DISABLED(registerValue);
            break;
        } // case RFID_ANTENNA_PORT_STATE_DISABLED
        case RFID_ANTENNA_PORT_STATE_ENABLED:
        {
            HST_ANT_DESC_CFG_SET_ENABLED(registerValue);
            break;
        } // case RFID_ANTENNA_PORT_STATE_ENABLED
        default:
        {
            throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            break;
        } // default
    } // switch (state)

    // Write back the configuration register
    m_pMac->WriteRegister(HST_ANT_DESC_CFG, registerValue);
} // Radio::SetAntennaPortState

////////////////////////////////////////////////////////////////////////////////
// Name:        SetAntennaPortConfiguration
// Description: Sets the configuration for an antenna port
////////////////////////////////////////////////////////////////////////////////
void Radio::SetAntennaPortConfiguration(
    INT32U                          antennaPort,
    const RFID_ANTENNA_PORT_CONFIG* pConfig
    )
{
    assert(NULL != pConfig);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // First, tell the MAC which antenna descriptors we'll be reading and
    // verify that it was a valid selector
    m_pMac->WriteRegister(HST_ANT_DESC_SEL, antennaPort);
    if (HOSTIF_ERR_SELECTORBNDS == m_pMac->ReadRegister(MAC_ERROR))
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Antenna port %u is invalid\n",
            __FUNCTION__,
            antennaPort);

        this->ClearMacError();
        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    // Write the physical port mapping register
    m_pMac->WriteRegister(HST_ANT_DESC_PORTDEF, 
                          HST_ANT_DESC_PORTDEF_TXPORT(pConfig->physicalTxPort) |
                          HST_ANT_DESC_PORTDEF_RFU1(0)                         |
                          HST_ANT_DESC_PORTDEF_RXPORT(pConfig->physicalRxPort) |
                          HST_ANT_DESC_PORTDEF_RFU2(0));

    // Write the antenna dwell, RF power, inventory cycle count, and sense
    // resistor threshold registers
    m_pMac->WriteRegister(HST_ANT_DESC_DWELL, pConfig->dwellTime);
    m_pMac->WriteRegister(HST_ANT_DESC_RFPOWER, pConfig->powerLevel);
    m_pMac->WriteRegister(HST_ANT_DESC_INV_CNT, pConfig->numberInventoryCycles);
    m_pMac->WriteRegister(HST_RFTC_ANTSENSRESTHRSH, pConfig->antennaSenseThreshold);
} // Radio::SetAntennaPortConfiguration

////////////////////////////////////////////////////////////////////////////////
// Name:        GetAntennaPortConfiguration
// Description: Retrieves the configuration for an antenna port
////////////////////////////////////////////////////////////////////////////////
void Radio::GetAntennaPortConfiguration(
    INT32U                      antennaPort,
    RFID_ANTENNA_PORT_CONFIG*   pConfig
    )
{
    assert(NULL != pConfig);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // First, tell the MAC which antenna descriptors we'll be reading and
    // verify that it was a valid selector
    m_pMac->WriteRegister(HST_ANT_DESC_SEL, antennaPort);
    if (HOSTIF_ERR_SELECTORBNDS == m_pMac->ReadRegister(MAC_ERROR))
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Antenna port %u is invalid\n",
            __FUNCTION__,
            antennaPort);

        this->ClearMacError();
        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    // Read the physical port mapping register
    INT32U registerValue = m_pMac->ReadRegister(HST_ANT_DESC_PORTDEF);

    pConfig->physicalTxPort = HST_ANT_DESC_PORTDEF_GET_TXPORT(registerValue);
    pConfig->physicalRxPort = HST_ANT_DESC_PORTDEF_GET_RXPORT(registerValue);

    // Read the antenna dwell time, RF power, inventory cycle count, and 
    // sense resistor registers
    pConfig->dwellTime              =
        m_pMac->ReadRegister(HST_ANT_DESC_DWELL);
    pConfig->powerLevel             =
        m_pMac->ReadRegister(HST_ANT_DESC_RFPOWER);
    pConfig->numberInventoryCycles  =
        m_pMac->ReadRegister(HST_ANT_DESC_INV_CNT);
    pConfig->antennaSenseThreshold  = 
        m_pMac->ReadRegister(HST_RFTC_ANTSENSRESTHRSH);
} // Radio::GetAntennaPortConfiguration

////////////////////////////////////////////////////////////////////////////////
// Name:        Set18K6CSelectCriteria
// Description: Sets the ISO 18000-6C tag-selection criteria.
////////////////////////////////////////////////////////////////////////////////
void Radio::Set18K6CSelectCriteria(
    const RFID_18K6C_SELECT_CRITERIA* pCriteria
    )
{
    INT32U                             index;
    const RFID_18K6C_SELECT_CRITERION* pCriterion;
    RFID_REGISTER_INFO                 info;
    INT16U                             bankSize = RFID_18K6C_MAX_SELECT_CRITERIA_CNT;

    assert(NULL != pCriteria);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }


    // Use select register info to get bank size, if its supported
    // 
    if (!m_preTwoTwoFirmware)
    {
        this->ReadMacRegisterInfo(HST_TAGMSK_DESC_SEL, &info);
        bankSize = info.bankSize;
    }

    // ensure the countCriteria does not excced bank size
    if (pCriteria->countCriteria > bankSize)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Select countCriteria %u is invalid\n",
            __FUNCTION__,
            pCriteria->countCriteria);
        throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    // Set up the selection criteria
    pCriterion = pCriteria->pCriteria;
    for (index = 0; index < pCriteria->countCriteria; index++)
    {
        INT32U                          registerValue;
        const RFID_18K6C_SELECT_MASK*   pMask   = &pCriterion->mask;
        const RFID_18K6C_SELECT_ACTION* pAction = &pCriterion->action;

        // Instruct the MAC as to which select mask we want to work with
        m_pMac->WriteRegister(HST_TAGMSK_DESC_SEL, index);

        // Create the HST_TAGMSK_DESC_CFG register value and write it to the MAC
        registerValue = HST_TAGMSK_DESC_CFG_ENABLED                 |
                        HST_TAGMSK_DESC_CFG_TARGET(pAction->target) |
                        HST_TAGMSK_DESC_CFG_ACTION(pAction->action) |
                        (pAction->enableTruncate ? 
                            HST_TAGMSK_DESC_CFG_TRUNC_ENABLED :
                            HST_TAGMSK_DESC_CFG_TRUNC_DISABLED)     |
                        HST_TAGMSK_DESC_CFG_RFU1(0);
        m_pMac->WriteRegister(HST_TAGMSK_DESC_CFG, registerValue);

        // Create the HST_TAGMSK_BANK register value and write it to the MAC
        registerValue = HST_TAGMSK_BANK_BANK(pMask->bank)   |
                        HST_TAGMSK_BANK_RFU1(0);
        m_pMac->WriteRegister(HST_TAGMSK_BANK, registerValue);

        // Write the mask offset to the HST_TAGMSK_PTR register
        m_pMac->WriteRegister(HST_TAGMSK_PTR, pMask->offset);

        // Create the HST_TAGMSK_LEN register and write it to the MAC
        registerValue = HST_TAGMSK_LEN_LEN(pMask->count) |
                        HST_TAGMSK_LEN_RFU1(0);
        m_pMac->WriteRegister(HST_TAGMSK_LEN, registerValue);

        // Now write the MAC's mask registers
        this->WriteMacMaskRegisters(HST_TAGMSK_0_3, pMask->count, pMask->mask);

        // advance to next criterion
        pCriterion++;
    } // for (each criterion)

    // Turn off select for the remainder of the select mask registers
    while (index < bankSize)
    {
        // Instruct the MAC as to which select mask we want to work with
        m_pMac->WriteRegister(HST_TAGMSK_DESC_SEL, index);

        // Set the descriptor to disabled
        m_pMac->WriteRegister(HST_TAGMSK_DESC_CFG, HST_TAGMSK_DESC_CFG_DISABLED);

        index++;
    }
} // Radio::Set18K6CSelectCriteria

////////////////////////////////////////////////////////////////////////////////
// Name:        Get18K6CSelectCriteria
// Description: Gets the ISO 18000-6C tag-selection criteria.
////////////////////////////////////////////////////////////////////////////////
void Radio::Get18K6CSelectCriteria(
    RFID_18K6C_SELECT_CRITERIA* pCriteria
    )
{
    INT32U                            index;
    INT32U                            countCriteria;
    bool                              arrayIsTooSmall  = false;
    RFID_REGISTER_INFO                info;
    INT16U                            bankSize         = RFID_18K6C_MAX_SELECT_CRITERIA_CNT;

    assert(NULL != pCriteria);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    assert(!pCriteria->countCriteria || pCriteria->pCriteria);

    // Use select register info to get bank size, if its supported
    // 
    if (!m_preTwoTwoFirmware)
    {
        this->ReadMacRegisterInfo(HST_TAGMSK_DESC_SEL, &info);
        bankSize = info.bankSize;
    }


    countCriteria = 0;
    // Go through the criteria and pick out the enabled ones
    for (index = 0; index<bankSize; index++)
    {
        INT32U  registerValue;

        // Instruct the MAC as to which criteria we want to work with
        m_pMac->WriteRegister(HST_TAGMSK_DESC_SEL, index);

        // Read the criteria configuration and determine if it is enabled
        registerValue = m_pMac->ReadRegister(HST_TAGMSK_DESC_CFG);
        if (HST_TAGMSK_DESC_CFG_IS_ENABLED(registerValue))
        {
            // If the array is large enough, copy the criteria information
            if (countCriteria < pCriteria->countCriteria)
            {
                RFID_18K6C_SELECT_CRITERION*    pCriterion  = 
                    pCriteria->pCriteria + countCriteria;
                RFID_18K6C_SELECT_MASK*         pMask       = 
                    &pCriterion->mask;
                RFID_18K6C_SELECT_ACTION*       pAction     = 
                    &pCriterion->action;

                // Fill out the action portion of the criterion
                pAction->target         = 
                    HST_TAGMSK_DESC_CFG_GET_TARGET(registerValue);
                pAction->action         = 
                    HST_TAGMSK_DESC_CFG_GET_ACTION(registerValue);
                pAction->enableTruncate = 
                    (HST_TAGMSK_DESC_CFG_TRUNC_IS_ENABLED(registerValue) ? 1 : 0);

                // Fill out the mask bank, offset, and length for the criterion
                pMask->bank     = 
                    HST_TAGMSK_BANK_GET_BANK(
                        m_pMac->ReadRegister(HST_TAGMSK_BANK));
                pMask->offset   =  
                    m_pMac->ReadRegister(HST_TAGMSK_PTR);
                pMask->count    = 
                    HST_TAGMSK_LEN_GET_LEN(
                        m_pMac->ReadRegister(HST_TAGMSK_LEN));

                // Clear out the mask first
                memset(pMask->mask, 0, RFID_18K6C_SELECT_MASK_BYTE_LEN);

                // Now read the selector mask
                this->ReadMacMaskRegisters(HST_TAGMSK_0_3, pMask->count, pMask->mask);
            }
            else
            {
                arrayIsTooSmall = true;
            }

            // Increment the number of enabled criteria found
            ++countCriteria;
        }
    } // for (each criterion)

    // Set the caller's criteria count
    pCriteria->countCriteria = countCriteria;

    // Check to see if the array was not large enough
    if (arrayIsTooSmall)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_DEBUG,
            "%s: Caller's select criteria buffer too small\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_BUFFER_TOO_SMALL, __FUNCTION__);
    }
} // Radio::Get18K6CSelectCriteria

////////////////////////////////////////////////////////////////////////////
// Name:        Set18K6CPostSingulationMatchCriteria
// Description: Sets the criteria to be applied to an ISO 18000-6C tag
//              after singulation
// Parameters:  pCriteria - a pointer to the post-singulation criteria
// Returns:     Nothing
////////////////////////////////////////////////////////////////////////////////
void Radio::Set18K6CPostSingulationMatchCriteria(
    const RFID_18K6C_SINGULATION_CRITERIA* pParms
    )
{
    const RFID_18K6C_SINGULATION_CRITERION* pCriterion;
    INT32U                                  registerValue;
    
    assert(NULL != pParms);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    if (0 != pParms->countCriteria)
    {
        // Set up the post-singulation match criteria
        pCriterion = pParms->pCriteria;
        const RFID_18K6C_SINGULATION_MASK* pMask = &pCriterion->mask;
    
        // Set up the HST_INV_EPC_MATCH_CFG register and write it to the MAC.
        // For now, we are going to assume that the singulation match should be
        // enabled (if the application so desires, we can turn it off when we
        // actually do the tag-protocol operation).
        registerValue = HST_INV_EPC_MATCH_CFG_ENABLED               |
                        (pCriterion->match ?
                            HST_INV_EPC_MATCH_CFG_MATCH(
                                HST_INV_EPC_MATCH_CFG_MATCH_YES) :
                            HST_INV_EPC_MATCH_CFG_MATCH(
                                HST_INV_EPC_MATCH_CFG_MATCH_NO))    |
                        HST_INV_EPC_MATCH_CFG_LEN(pMask->count)     |
                        HST_INV_EPC_MATCH_CFG_OFF(pMask->offset)    |
                        HST_INV_EPC_MATCH_CFG_RFU1(0);
        m_pMac->WriteRegister(HST_INV_EPC_MATCH_CFG, registerValue);
    
        // Now write the MAC's mask registers
        this->WriteMacMaskRegisters(HST_INV_EPCDAT_0_3, pMask->count, pMask->mask);
    }
    else // must be calling to disable criteria
    {
        m_pMac->WriteRegister(HST_INV_EPC_MATCH_CFG, HST_INV_EPC_MATCH_CFG_DISABLED);
    }

} // Radio::Set18K6CPostSingulationMatchCriteria

///////////////////////////////////////////////////////////////////////////
// Name:        Get18K6CPostSingulationMatchCriteria
// Description: Gets the match criteria to be applied to an ISO 18000-6C tag
//              after singulation
////////////////////////////////////////////////////////////////////////////
void Radio::Get18K6CPostSingulationMatchCriteria(
    RFID_18K6C_SINGULATION_CRITERIA* pParms
    )
{
    INT32U  countCriteria   = 0;
    INT32U  registerValue;
    bool    arrayIsTooSmall = false;

    assert(NULL != pParms);
    assert(!pParms->countCriteria || pParms->pCriteria);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Read the criteria configuration and determine if it is enabled
    registerValue = m_pMac->ReadRegister(HST_INV_EPC_MATCH_CFG);
    if (HST_INV_EPC_MATCH_CFG_IS_ENABLED(registerValue))
    {

        // If the array is large enough, copy the criteria information
        if (countCriteria < pParms->countCriteria)
        {
            RFID_18K6C_SINGULATION_CRITERION*  pCriterion  = pParms->pCriteria;
            RFID_18K6C_SINGULATION_MASK*       pMask       = &pCriterion->mask;
    
            // Set the match/non-match, offset, and count for the criterion.
            pCriterion->match   = 
                (HST_INV_EPC_MATCH_CFG_GET_MATCH(registerValue) == 
                 HST_INV_EPC_MATCH_CFG_MATCH_YES ? 1 : 0);
            pMask->offset       = 
                HST_INV_EPC_MATCH_CFG_GET_OFF(registerValue);
            pMask->count        = 
                HST_INV_EPC_MATCH_CFG_GET_LEN(registerValue);
    
            // Clear out the mask first
            memset(pMask->mask, 0, RFID_18K6C_SINGULATION_MASK_BYTE_LEN);
    
            // Now read the selector mask
            this->ReadMacMaskRegisters(HST_INV_EPCDAT_0_3, pMask->count, pMask->mask);
        }
        else
        {
            arrayIsTooSmall = true;
        }
        // Increment the number of enabled criteria found
        ++countCriteria;
    }

    // Set the caller's criteria count
    pParms->countCriteria = countCriteria;

    // Check to see if the array was not large enough
    if (arrayIsTooSmall)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_DEBUG,
            "%s: Caller's post-singualation criteria buffer too small\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_BUFFER_TOO_SMALL);
    }
} // Radio::Get18K6CPostSingulationMatchCriteria

////////////////////////////////////////////////////////////////////////////
// Name:        Set18K6CQueryTagGroup
// Description: Sets the query tag group parameters
// Parameters:  pGroup - a pointer to the query tag group parameters
// Returns:     Nothing
////////////////////////////////////////////////////////////////////////////
void Radio::Set18K6CQueryTagGroup(
    const RFID_18K6C_TAG_GROUP* pGroup
    )
{
    assert(NULL != pGroup);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Write the tag group values to the query configuration register
    m_pMac->WriteRegister(HST_QUERY_CFG,
        HST_QUERY_CFG_RFU1(0)               |
        HST_QUERY_CFG_TARG(pGroup->target)  |
        HST_QUERY_CFG_SESS(pGroup->session) |
        HST_QUERY_CFG_SEL(pGroup->selected) |
        HST_QUERY_CFG_RFU2(0));
} // Radio::Set18K6CQueryTagGroup

////////////////////////////////////////////////////////////////////////////
// Name:        Get18K6CQueryTagGroup
// Description: Retrieves the query tag group parameters
// Parameters:  pGroup - a pointer to structure that will receive the query
//              tag group parameters
// Returns:     Nothing
////////////////////////////////////////////////////////////////////////////
void Radio::Get18K6CQueryTagGroup(
    RFID_18K6C_TAG_GROUP*   pGroup
    )
{
    assert(NULL != pGroup);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Retrieve the Query configuration register and set the fields
    // in the structure appropriately
    INT32U registerValue    = m_pMac->ReadRegister(HST_QUERY_CFG);
    pGroup->selected        = HST_QUERY_CFG_GET_SEL(registerValue);

    // At the library level, we only support the 3 select values (1, 2, 3).  If the MAC
    // returned 0 (alternate select all value) we will force it to 1 (preferred select all)
    if (pGroup->selected < RFID_18K6C_SELECT_ALL)     
    {
        pGroup->selected = RFID_18K6C_SELECT_ALL;
    }
    
    pGroup->session         = HST_QUERY_CFG_GET_SESS(registerValue);
    pGroup->target          = HST_QUERY_CFG_GET_TARG(registerValue);
} // Radio::Get18K6CQueryTagGroup

////////////////////////////////////////////////////////////////////////////
// Name:        SetCurrentSingulationAlgorithm
// Description: Sets the currently-active singulation algorithm
////////////////////////////////////////////////////////////////////////////
void Radio::SetCurrentSingulationAlgorithm(
    RFID_18K6C_SINGULATION_ALGORITHM    algorithm
    )
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Write the HST_INV_CFG register common parts (we'll write the
    // repeat count and the auto-select flags when the we are actually going
    // to do a tag-access or inventory operation).
    m_pMac->WriteRegister(HST_INV_CFG,
        HST_INV_CFG_ALGO(algorithm)     |
        HST_INV_CFG_REP(0)              |
        HST_INV_CFG_AUTOSEL_DISABLED    |
        HST_INV_CFG_RFU1(0));
} // Radio::SetCurrentSingulationAlgorithm

////////////////////////////////////////////////////////////////////////////
// Name:        GetCurrentSingulationAlgorithm
// Description: Retrieves the currently-active singulation algorithm
////////////////////////////////////////////////////////////////////////////
RFID_18K6C_SINGULATION_ALGORITHM Radio::GetCurrentSingulationAlgorithm()
{
    INT32U algorithm;
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Retrieve the inventory configuration register so we can figure out what
    // singulation algorithm is configured
    algorithm =  HST_INV_CFG_GET_ALGO(m_pMac->ReadRegister(HST_INV_CFG));

    if (!((RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ == algorithm)   ||
          (RFID_18K6C_SINGULATION_ALGORITHM_DYNAMICQ == algorithm)))
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Invalid singulation algorithm found (%d)\n",
            __FUNCTION__, algorithm);
        throw RfidErrorException(RFID_ERROR_UNEXPECTED_VALUE, __FUNCTION__);
    }

    return algorithm;
} // Radio::GetCurrentSingulationAlgorithm

////////////////////////////////////////////////////////////////////////////
// Name:        SetSingulationAlgorithmParameters
// Description: Sets the singulation algorithm parameters for the fixed Q
//              singulation algorithm
////////////////////////////////////////////////////////////////////////////
void Radio::SetSingulationAlgorithmParameters(
    const RFID_18K6C_SINGULATION_FIXEDQ_PARMS*      pParms
    )
{
    assert(NULL != pParms);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Write the inventory algorithm parameter registers
    m_pMac->WriteRegister(HST_INV_SEL, RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ);
    m_pMac->WriteRegister(HST_INV_ALG_PARM_0,
        HST_INV_ALG_PARM_0_ALGO0_Q(pParms->qValue)  |
        HST_INV_ALG_PARM_0_ALGO0_RFU1(0));
    m_pMac->WriteRegister(HST_INV_ALG_PARM_1,
        HST_INV_ALG_PARM_1_ALGO0_RETRY(pParms->retryCount) |
        HST_INV_ALG_PARM_1_ALGO0_RFU1(0));
    m_pMac->WriteRegister(HST_INV_ALG_PARM_2,
        (pParms->toggleTarget ?
            HST_INV_ALG_PARM_2_ALGO0_TOGGLE_ENABLED :
            HST_INV_ALG_PARM_2_ALGO0_TOGGLE_DISABLED)   |
        (pParms->repeatUntilNoTags ?
            HST_INV_ALG_PARM_2_ALGO0_REPEAT_ENABLED :
            HST_INV_ALG_PARM_2_ALGO0_REPEAT_DISABLED)   |
        HST_INV_ALG_PARM_2_ALGO0_RFU1(0));
    m_pMac->WriteRegister(HST_INV_ALG_PARM_3, 0);
} // Radio::SetSingulationAlgorithmParameters


////////////////////////////////////////////////////////////////////////////
// Name:        SetSingulationAlgorithmParameters
// Description: Sets the singulation algorithm parameters for the dynamic Q
//              with Q-adjustment threshold singulation algorithm
////////////////////////////////////////////////////////////////////////////
void Radio::SetSingulationAlgorithmParameters(
    const RFID_18K6C_SINGULATION_DYNAMICQ_PARMS* pParms
    )
{
    assert(NULL != pParms);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Write the inventory algorithm parameter registers.  For register
    // zero, remember to preserve values that we aren't exposing
    m_pMac->WriteRegister(HST_INV_SEL,
        RFID_18K6C_SINGULATION_ALGORITHM_DYNAMICQ);
    m_pMac->WriteRegister(HST_INV_ALG_PARM_0,
        HST_INV_ALG_PARM_0_ALGO1_STARTQ(pParms->startQValue)        |
        HST_INV_ALG_PARM_0_ALGO1_MAXQ(pParms->maxQValue)            |
        HST_INV_ALG_PARM_0_ALGO1_MINQ(pParms->minQValue)            |
        HST_INV_ALG_PARM_0_ALGO1_TMULT(pParms->thresholdMultiplier) |
        HST_INV_ALG_PARM_0_ALGO1_RFU1(0));
    m_pMac->WriteRegister(HST_INV_ALG_PARM_1,
        HST_INV_ALG_PARM_1_ALGO1_RETRY(pParms->retryCount)  |
        HST_INV_ALG_PARM_1_ALGO1_RFU1(0));
    m_pMac->WriteRegister(HST_INV_ALG_PARM_2,
        (pParms->toggleTarget ?
            HST_INV_ALG_PARM_2_ALGO1_TOGGLE_ENABLED :
            HST_INV_ALG_PARM_2_ALGO1_TOGGLE_DISABLED)   |
        HST_INV_ALG_PARM_2_ALGO1_RFU1(0));
    m_pMac->WriteRegister(HST_INV_ALG_PARM_3, 0);
} // Radio::SetSingulationAlgorithmParameters

////////////////////////////////////////////////////////////////////////////
// Name:        GetSingulationAlgorithmParameters
// Description: Retrieves the singulation algorithm parameters for the
//              fixed Q singulation algorithm
////////////////////////////////////////////////////////////////////////////
void Radio::GetSingulationAlgorithmParameters(
    RFID_18K6C_SINGULATION_FIXEDQ_PARMS*    pParms
    )
{
    assert(NULL != pParms);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Tell the MAC which singulation algorithm selector to use and then
    // read the singulation algorithm registers
    m_pMac->WriteRegister(HST_INV_SEL, RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ);
    INT32U  parm0Register = m_pMac->ReadRegister(HST_INV_ALG_PARM_0);
    INT32U  parm1Register = m_pMac->ReadRegister(HST_INV_ALG_PARM_1);
    INT32U  parm2Register = m_pMac->ReadRegister(HST_INV_ALG_PARM_2);

    // Set up the fixed Q singulation algorithm structure
    pParms->length              =  
        sizeof(RFID_18K6C_SINGULATION_FIXEDQ_PARMS);
    pParms->qValue              =  
        HST_INV_ALG_PARM_0_ALGO0_GET_Q(parm0Register);
    pParms->retryCount          = 
        HST_INV_ALG_PARM_1_ALGO0_GET_RETRY(parm1Register);
    pParms->toggleTarget        = 
        (HST_INV_ALG_PARM_2_ALGO0_TOGGLE_IS_ENABLED(parm2Register) ? 1 : 0);
    pParms->repeatUntilNoTags   = 
        (HST_INV_ALG_PARM_2_ALGO0_REPEAT_IS_ENABLED(parm2Register) ? 1 : 0);
} // Radio::GetSingulationAlgorithmParameters


////////////////////////////////////////////////////////////////////////////
// Name:        GetSingulationAlgorithmParameters
// Description: Retrieves the singulation algorithm parameters for the
//              dynamic Q with Q-adjustment threshold singulation algorithm
////////////////////////////////////////////////////////////////////////////
void Radio::GetSingulationAlgorithmParameters(
    RFID_18K6C_SINGULATION_DYNAMICQ_PARMS*   pParms
    )
{
    assert(NULL != pParms);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Tell the MAC which singulation algorithm selector to use and then
    // read the singulation algorithm registers
    m_pMac->WriteRegister(HST_INV_SEL,
        RFID_18K6C_SINGULATION_ALGORITHM_DYNAMICQ);
    INT32U  parm0Register = m_pMac->ReadRegister(HST_INV_ALG_PARM_0);
    INT32U  parm1Register = m_pMac->ReadRegister(HST_INV_ALG_PARM_1);
    INT32U  parm2Register = m_pMac->ReadRegister(HST_INV_ALG_PARM_2);

    // Extract the dynamic-Q with Q-adjustment threshold singulation algorithm
    // parameters
    pParms->length              = 
        sizeof(RFID_18K6C_SINGULATION_DYNAMICQ_PARMS);
    pParms->startQValue         = 
        HST_INV_ALG_PARM_0_ALGO1_GET_STARTQ(parm0Register);
    pParms->minQValue           = 
        HST_INV_ALG_PARM_0_ALGO1_GET_MINQ(parm0Register);
    pParms->maxQValue           = 
        HST_INV_ALG_PARM_0_ALGO1_GET_MAXQ(parm0Register);
    pParms->thresholdMultiplier = 
        HST_INV_ALG_PARM_0_ALGO1_GET_TMULT(parm0Register);
    pParms->retryCount          = 
        HST_INV_ALG_PARM_1_ALGO1_GET_RETRY(parm1Register);;
    pParms->toggleTarget        = 
        (HST_INV_ALG_PARM_2_ALGO1_TOGGLE_IS_ENABLED(parm2Register) ? 1 : 0);
} // Radio::GetSingulationAlgorithmParameters



////////////////////////////////////////////////////////////////////////////////
// Name:        Start18K6CInventory
// Description: Requests that an ISO 18000-6C inventory be started on the
//              radio module.
////////////////////////////////////////////////////////////////////////////////
void Radio::Start18K6CInventory(
    const RFID_18K6C_INVENTORY_PARMS*   pParms,
    INT32U                              flags
    )
{
    assert(NULL != pParms);

    // Perform the common 18K6C tag-operation request setup
    this->Start18K6CRequest(&pParms->common, flags);

    // Issue the inventory command to the MAC
    m_pMac->WriteRegister(HST_CMD, CMD_18K6CINV);

    // Perform the common 18K6C tag-operation request post setup
    this->PostMacCommandIssue();
} // Radio::Start18K6CInventory


////////////////////////////////////////////////////////////////////////////////
// Name:        Setup18K6CReadRegisters
// Description: Configures the tag read specific registers in the radio module.
////////////////////////////////////////////////////////////////////////////////
void Radio::Setup18K6CReadRegisters(
    const RFID_18K6C_READ_CMD_PARMS*        pParms
    )
{
    // Set up the access bank register
    m_pMac->WriteRegister(HST_TAGACC_BANK,
                          HST_TAGACC_BANK_BANK(pParms->bank) |
                          HST_TAGACC_BANK_RFU1(0));

    // Set up the access pointer register (tells the offset)
    m_pMac->WriteRegister(HST_TAGACC_PTR, pParms->offset);

    // Set up the access count register (i.e., number values to read)
    m_pMac->WriteRegister(HST_TAGACC_CNT,
                          HST_TAGACC_CNT_LEN(pParms->count) |
                          HST_TAGACC_CNT_RFU1(0));
}


////////////////////////////////////////////////////////////////////////////////
// Name:        Start18K6CRead
// Description: Requests that an ISO 18000-6C tag read be started on the
//              radio module.
////////////////////////////////////////////////////////////////////////////////
void Radio::Start18K6CRead(
    const RFID_18K6C_READ_PARMS*        pParms,
    INT32U                              flags
    )
{
    assert(NULL != pParms);

    // Perform the common 18K6C tag operation setup
    this->Start18K6CRequest(&pParms->common, flags);

    this->Setup18K6CReadRegisters(&pParms->readCmdParms);

    // Set up the access password register
    m_pMac->WriteRegister(HST_TAGACC_ACCPWD, pParms->accessPassword);

    // Issue the read command
    m_pMac->WriteRegister(HST_CMD, CMD_18K6CREAD);

    // Perform the common 18K6C tag-operation request post setup
    this->PostMacCommandIssue();
} //  Radio::Start18K6CRead


////////////////////////////////////////////////////////////////////////////////
// Name:        Setup18K6CWriteRegisters
// Description: Configures the tag write specific registers in the radio module.
////////////////////////////////////////////////////////////////////////////////
void Radio::Setup18K6CWriteRegisters(
    RFID_18K6C_WRITE_TYPE writeType,
    const void*        pParms
    )
{
    // Set the offset to zero
    m_pMac->WriteRegister(HST_TAGACC_PTR, 0);

    std::vector<INT16U>     sequentialOffsets;
    RFID_18K6C_MEMORY_BANK  bank    = 0;
    INT16U                  count   = 0;
    const INT16U*           pData   = NULL;
    const INT16U*           pOffset = NULL;

    // Based upon the type of write, set up the locals accordingly
    switch (writeType)
    {
        // Sequential write
        case RFID_18K6C_WRITE_TYPE_SEQUENTIAL:
        {
            INT16U                                   offset;
            const RFID_18K6C_WRITE_SEQUENTIAL_CMD_PARMS* pWriteParms =
                (RFID_18K6C_WRITE_SEQUENTIAL_CMD_PARMS*) pParms;

            bank  = pWriteParms->bank;
            pData = pWriteParms->pData;

            sequentialOffsets.reserve(pWriteParms->count);

            // Set up the vector so that it has sequential offsets, with the
            // [0] element containing the first offset.  When we are done,
            // the count variable will also be appropriately set.
            for (count = 0, offset = pWriteParms->offset;
                 count < pWriteParms->count;
                 ++count, ++offset)
            {
                sequentialOffsets.push_back(offset);
            }

            // Get the pointer to the start of the vector
            pOffset = &sequentialOffsets[0];
            break;
        } // case RFID_18K6C_WRITE_TYPE_SEQUENTIAL
        case RFID_18K6C_WRITE_TYPE_RANDOM:
        {
            const RFID_18K6C_WRITE_RANDOM_CMD_PARMS* pWriteParms =
                (RFID_18K6C_WRITE_RANDOM_CMD_PARMS*) pParms;

            bank    = pWriteParms->bank;
            count   = pWriteParms->count;
            pData   = pWriteParms->pData;
            pOffset = pWriteParms->pOffset;

            break;
        } // case RFID_18K6C_WRITE_TYPE_RANDOM
        default:
        {
            throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            break;
        } // default
    } // switch (pParms->writeType)

    // NOTE: The Verify bit is deprecated, and is forced here only for backward 
    //       compatibility with earlier versions of the MAC firmware.
    INT32U registerValue = m_pMac->ReadRegister(HST_TAGACC_DESC_CFG);
    HST_TAGACC_DESC_CFG_SET_VERIFY_ENABLED(registerValue);
    m_pMac->WriteRegister(HST_TAGACC_DESC_CFG, registerValue);

    // Set up the tag bank register (tells where to write the data)
    m_pMac->WriteRegister(HST_TAGACC_BANK,
                          HST_TAGACC_BANK_BANK(bank) |
                          HST_TAGACC_BANK_RFU1(0));

    // Set up the access count register (i.e., number of words to write)
    m_pMac->WriteRegister(HST_TAGACC_CNT,
                          HST_TAGACC_CNT_LEN(count) |
                          HST_TAGACC_CNT_RFU1(0));


    // Set up the HST_TAGWRDAT_N registers.  Fill up a bank at a time.
    for (INT32U registerBank = 0; count; ++registerBank)
    {
        // Indicate which bank of tag write registers we are going to fill
        m_pMac->WriteRegister(HST_TAGWRDAT_SEL,
                              HST_TAGWRDAT_SEL_SELECTOR(registerBank) |
                              HST_TAGWRDAT_SEL_RFU1(0));

        if (HOSTIF_ERR_SELECTORBNDS == m_pMac->ReadRegister(MAC_ERROR))
        {
            this->ClearMacError();
            throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        // Write the values to the bank until either the bank is full or we run out of data
        INT16U registerAddress = 0;
        INT32U offset = 0;
        while ((count > 0) && (offset < RFID_NUM_TAGWRDAT_REGS_PER_BANK))
        {
            // Set up the register and then write it to the MAC
            INT32U  registerValue = HST_TAGWRDAT_N_DATA(*pData) |
                                    HST_TAGWRDAT_N_OFF(*pOffset);
            m_pMac->WriteRegister(HST_TAGWRDAT_0 + registerAddress, registerValue);

            registerAddress++;
            pData++;
            pOffset++;
            offset++;
            count--;
        }
    }

}


////////////////////////////////////////////////////////////////////////////////
// Name:        Start18K6CWrite
// Description: Requests that an ISO 18000-6C tag write be started on the
//              radio module.
////////////////////////////////////////////////////////////////////////////////
void Radio::Start18K6CWrite(
    const RFID_18K6C_WRITE_PARMS*       pParms,
    INT32U                              flags
    )
{
    assert(NULL != pParms);

    // Perform the common 18K6C tag operation setup
    this->Start18K6CRequest(&pParms->common, flags);

    // Based upon the type of write, set up the locals accordingly
    switch (pParms->writeType)
    {
        // Sequential write
        case RFID_18K6C_WRITE_TYPE_SEQUENTIAL:
        {
            this->Setup18K6CWriteRegisters(pParms->writeType, &pParms->writeCmdParms.sequential);
            break;
        } 
        case RFID_18K6C_WRITE_TYPE_RANDOM:
        {
            this->Setup18K6CWriteRegisters(pParms->writeType, &pParms->writeCmdParms.random);
            break;
        }
        default:
        {
            throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            break;
        } // default
    }

    // Set up the tag access password
    m_pMac->WriteRegister(HST_TAGACC_ACCPWD,
                          pParms->accessPassword);

    // Issue the write command to the MAC
    m_pMac->WriteRegister(HST_CMD, CMD_18K6CWRITE);

    // Perform the common 18K6C tag-operation request post setup
    this->PostMacCommandIssue();

} //  Radio::Start18K6CWrite

////////////////////////////////////////////////////////////////////////////////
// Name:        Start18K6CKill
// Description: Requests that an ISO 18000-6C tag kill be started on the
//              radio module.
////////////////////////////////////////////////////////////////////////////////
void Radio::Start18K6CKill(
    const RFID_18K6C_KILL_PARMS*        pParms,
    INT32U                              flags
    )
{
    assert(NULL != pParms);

    // Perform the common 18K6C tag operation setup
    this->Start18K6CRequest(&pParms->common, flags);

    // Set up the access password register
    m_pMac->WriteRegister(HST_TAGACC_ACCPWD, pParms->accessPassword);

    // Set up the kill password register
    m_pMac->WriteRegister(HST_TAGACC_KILLPWD, pParms->killCmdParms.killPassword);

    // Issue the kill command
    m_pMac->WriteRegister(HST_CMD, CMD_18K6CKILL);

    // Perform the common 18K6C tag-operation request post setup
    this->PostMacCommandIssue();
} //  Radio::Start18K6CKill

////////////////////////////////////////////////////////////////////////////////
// Name:        Start18K6CLock
// Description: Requests that an ISO 18000-6C tag lock be started on the
//              radio module.
////////////////////////////////////////////////////////////////////////////////
void Radio::Start18K6CLock(
    const RFID_18K6C_LOCK_PARMS*        pParms,
    INT32U                              flags
    )
{
    INT32U registerValue = 0;

    assert(NULL != pParms);

    // Perform the common 18K6C tag operation setup
    this->Start18K6CRequest(&pParms->common, flags);

    // If the kill password access permissions are not to change, then indicate
    // to ignore those bits.
    if (RFID_18K6C_TAG_PWD_PERM_NO_CHANGE ==
        pParms->lockCmdParms.permissions.killPasswordPermissions)
    {
        HST_TAGACC_LOCKCFG_SET_MASK_KILL(registerValue,
                                         HST_TAGACC_LOCKCFG_MASK_USE_NO_ACTION);
    }
    // Otherwise, indicate to look at the kill password bits and set the
    // persmission for it
    else
    {
        HST_TAGACC_LOCKCFG_SET_MASK_KILL(
            registerValue,
            HST_TAGACC_LOCKCFG_MASK_USE_BOTH_ACTION);
        HST_TAGACC_LOCKCFG_SET_ACTION_KILL(
            registerValue,
            pParms->lockCmdParms.permissions.killPasswordPermissions);
    }

    // If the access password access permissions are not to change, then
    // indicate to ignore those bits.
    if (RFID_18K6C_TAG_PWD_PERM_NO_CHANGE ==
        pParms->lockCmdParms.permissions.accessPasswordPermissions)
    {
        HST_TAGACC_LOCKCFG_SET_MASK_ACC(registerValue,
                                        HST_TAGACC_LOCKCFG_MASK_USE_NO_ACTION);
    }
    // Otherwise, indicate to look at the access password bits and set the
    // persmission for it
    else
    {
        HST_TAGACC_LOCKCFG_SET_MASK_ACC(
            registerValue,
            HST_TAGACC_LOCKCFG_MASK_USE_BOTH_ACTION);
        HST_TAGACC_LOCKCFG_SET_ACTION_ACC(
            registerValue,
            pParms->lockCmdParms.permissions.accessPasswordPermissions);
    }

    // If the EPC memory access permissions are not to change, then indicate
    // to ignore those bits.
    if (RFID_18K6C_TAG_MEM_PERM_NO_CHANGE ==
        pParms->lockCmdParms.permissions.epcMemoryBankPermissions)
    {
        HST_TAGACC_LOCKCFG_SET_MASK_EPC(registerValue,
                                        HST_TAGACC_LOCKCFG_MASK_USE_NO_ACTION);
    }
    // Otherwise, indicate to look at the EPC memory bits and set the
    // persmission for it
    else
    {
        HST_TAGACC_LOCKCFG_SET_MASK_EPC(
            registerValue,
            HST_TAGACC_LOCKCFG_MASK_USE_BOTH_ACTION);
        HST_TAGACC_LOCKCFG_SET_ACTION_EPC(
            registerValue,
            pParms->lockCmdParms.permissions.epcMemoryBankPermissions);
    }

    // If the TID memory access permissions are not to change, then indicate
    // to ignore those bits.
    if (RFID_18K6C_TAG_MEM_PERM_NO_CHANGE ==
        pParms->lockCmdParms.permissions.tidMemoryBankPermissions)
    {
        HST_TAGACC_LOCKCFG_SET_MASK_TID(registerValue,
                                        HST_TAGACC_LOCKCFG_MASK_USE_NO_ACTION);
    }
    // Otherwise, indicate to look at the TID memory bits and set the
    // persmission for it
    else
    {
        HST_TAGACC_LOCKCFG_SET_MASK_TID(
            registerValue,
            HST_TAGACC_LOCKCFG_MASK_USE_BOTH_ACTION);
        HST_TAGACC_LOCKCFG_SET_ACTION_TID(
            registerValue,
            pParms->lockCmdParms.permissions.tidMemoryBankPermissions);
    }

    // If the user memory access permissions are not to change, then indicate
    // to ignore those bits.
    if (RFID_18K6C_TAG_MEM_PERM_NO_CHANGE ==
        pParms->lockCmdParms.permissions.userMemoryBankPermissions)
    {
        HST_TAGACC_LOCKCFG_SET_MASK_USER(registerValue,
                                         HST_TAGACC_LOCKCFG_MASK_USE_NO_ACTION);
    }
    // Otherwise, indicate to look at the user memory bits and set the
    // persmission for it
    else
    {
        HST_TAGACC_LOCKCFG_SET_MASK_USER(
            registerValue,
            HST_TAGACC_LOCKCFG_MASK_USE_BOTH_ACTION);
        HST_TAGACC_LOCKCFG_SET_ACTION_USER(
            registerValue,
            pParms->lockCmdParms.permissions.userMemoryBankPermissions);
    }

    // Set up the lock configuration register
    m_pMac->WriteRegister(HST_TAGACC_LOCKCFG, registerValue);

    // Set up the access password register
    m_pMac->WriteRegister(HST_TAGACC_ACCPWD, pParms->accessPassword);

    // Issue the lock command
    m_pMac->WriteRegister(HST_CMD, CMD_18K6CLOCK);

    // Perform the common 18K6C tag-operation request post setup
    this->PostMacCommandIssue();
} //  Radio::Start18K6CLock

////////////////////////////////////////////////////////////////////////////////
// Name:        Start18K6CBlockWrite
// Description: Requests that an ISO 18000-6C block write be started on the
//              radio module.
////////////////////////////////////////////////////////////////////////////////
void Radio::Start18K6CBlockWrite(
    const RFID_18K6C_BLOCK_WRITE_PARMS*       pBWParms,
    INT32U                                    flags
    )
{
    assert(NULL != pBWParms);

    // Perform the common 18K6C tag operation setup
    this->Start18K6CRequest(&pBWParms->common, flags);

    // NOTE: The Verify bit is deprecated, and is forced here only for backward 
    //       compatibility with earlier versions of the MAC firmware.
    INT32U registerValue = m_pMac->ReadRegister(HST_TAGACC_DESC_CFG);
    HST_TAGACC_DESC_CFG_SET_VERIFY_ENABLED(registerValue);
    m_pMac->WriteRegister(HST_TAGACC_DESC_CFG, registerValue);

    // Set the tag write data select register to zero
    m_pMac->WriteRegister(HST_TAGWRDAT_SEL,
                          HST_TAGWRDAT_SEL_SELECTOR(0) |
                          HST_TAGWRDAT_SEL_RFU1(0));

    INT16U                  count   = pBWParms->blockWriteCmdParms.count;
    const INT16U*           pData   = pBWParms->blockWriteCmdParms.pData;

    // Set up the tag bank register (tells where to write the data)
    m_pMac->WriteRegister(HST_TAGACC_BANK,
                          HST_TAGACC_BANK_BANK(pBWParms->blockWriteCmdParms.bank) |
                          HST_TAGACC_BANK_RFU1(0));

    // Set up the access pointer register (tells the offset)
    m_pMac->WriteRegister(HST_TAGACC_PTR, pBWParms->blockWriteCmdParms.offset);

    // Set up the access count register (i.e., number of words to write)
    m_pMac->WriteRegister(HST_TAGACC_CNT,
                          HST_TAGACC_CNT_LEN(count) |
                          HST_TAGACC_CNT_RFU1(0));

    // Set up the tag access password
    m_pMac->WriteRegister(HST_TAGACC_ACCPWD,
                          pBWParms->accessPassword);

    // Set up the HST_TAGWRDAT_N registers.  Fill up a bank at a time.
    for (INT32U registerBank = 0; count; ++registerBank)
    {
        // Indicate which bank of tag write registers we are going to fill
        m_pMac->WriteRegister(HST_TAGWRDAT_SEL,
                              HST_TAGWRDAT_SEL_SELECTOR(registerBank) |
                              HST_TAGWRDAT_SEL_RFU1(0));

        if (HOSTIF_ERR_SELECTORBNDS == m_pMac->ReadRegister(MAC_ERROR))
        {
            this->ClearMacError();
            throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        // Write the values to the bank until either the bank is full or we get to
        // a point where we cannot fill a register (i.e., we have 0 or 1 words left)
        INT32U offset = 0;
        for (;
             (offset < RFID_NUM_TAGWRDAT_REGS_PER_BANK) && (count > 1);
             count -= 2, pData += 2, ++offset)
        {
            m_pMac->WriteRegister((INT16U) (HST_TAGWRDAT_0 + offset),
                                  (INT32U)(HST_TAGWRDAT_N_DATA0(*pData) |
                                          HST_TAGWRDAT_N_DATA1(*(pData + 1))));
        }

        // If we didn't use all registers in the bank and count is non-zero, it means
        // that the request was for an odd number of words to be written.  Make sure
        // that the last word is written.
        if ((offset < RFID_NUM_TAGWRDAT_REGS_PER_BANK) && (1 == count)) 
        {
            m_pMac->WriteRegister((INT16U) (HST_TAGWRDAT_0 + offset),
                                  (INT32U) (HST_TAGWRDAT_N_DATA0(*pData) |
                                            HST_TAGWRDAT_N_DATA1(0)));
            break;
        }
    }

    // Issue the write command to the MAC
    m_pMac->WriteRegister(HST_CMD, CMD_18K6CBLOCKWRITE);

    // Perform the common 18K6C tag-operation request post setup
    this->PostMacCommandIssue();
} //  Radio::Start18K6CBlockWrite


////////////////////////////////////////////////////////////////////////////////
// Name:        Start18K6CQT
// Description: Requests that an ISO 18000-6C tag QT operation be started on the
//              radio module.
////////////////////////////////////////////////////////////////////////////////
void Radio::Start18K6CQT(
    const RFID_18K6C_QT_PARMS*          pParms,
    INT32U                              flags
    )
{
    INT32U qtValue = 0;

    assert(NULL != pParms);

    // Perform the common 18K6C tag operation setup
    this->Start18K6CRequest(&pParms->common, flags);

    // If an optional access command is provided, ensure the relevant 
    // registers for that command are setup

    switch (pParms->optCmdType)
    {
        // no optional access command, there's nothing else to do
        case RFID_18K6C_QT_OPT_NONE:
        {
            HST_TAGQTDAT_SET_OPT_ACCESS_CMD(qtValue, HST_TAGQTDAT_OPT_ACCESS_CMD_NONE);
            break; 
        }

        case RFID_18K6C_QT_OPT_READ:
        {
            this->Setup18K6CReadRegisters(&pParms->parameters.readCmdParms);
            HST_TAGQTDAT_SET_OPT_ACCESS_CMD(qtValue, HST_TAGQTDAT_OPT_ACCESS_CMD_READ);
            break;
        }

        case RFID_18K6C_QT_OPT_WRITE_TYPE_SEQUENTIAL:
        {
            this->Setup18K6CWriteRegisters(RFID_18K6C_WRITE_TYPE_SEQUENTIAL, &pParms->parameters.seqWriteCmdParms);
            HST_TAGQTDAT_SET_OPT_ACCESS_CMD(qtValue, HST_TAGQTDAT_OPT_ACCESS_CMD_WRITE);
            break;
        }

        case RFID_18K6C_QT_OPT_WRITE_TYPE_RANDOM:
        {
            this->Setup18K6CWriteRegisters(RFID_18K6C_WRITE_TYPE_RANDOM, &pParms->parameters.randWriteCmdParms);
            HST_TAGQTDAT_SET_OPT_ACCESS_CMD(qtValue, HST_TAGQTDAT_OPT_ACCESS_CMD_WRITE);
            break;
        }

        default:
        {
            throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            break;
        } // default
    }

    // Set up the access password register
    m_pMac->WriteRegister(HST_TAGACC_ACCPWD, pParms->accessPassword);

    // Now setup the QT specific register
    
    // The following assumes the RFID_18K6CTagQT has already validated the qtCmdParms are in range
    //
    HST_TAGQTDAT_SET_QT_READ_WRITE(qtValue, pParms->qtCmdParms.qtReadWrite);
    HST_TAGQTDAT_SET_QT_PERSISTENCE(qtValue, pParms->qtCmdParms.qtPersistence);
    HST_TAGQTDAT_SET_QT_MEM_MAP(qtValue, pParms->qtCmdParms.qtMemoryMap);
    HST_TAGQTDAT_SET_QT_SHORT_RANGE(qtValue,pParms->qtCmdParms.qtShortRange);
    HST_TAGQTDAT_SET_QT_RFU1(qtValue, 0);
    HST_TAGQTDAT_SET_QT_RAW_DISABLED(qtValue);

    m_pMac->WriteRegister(HST_TAGQTDAT, qtValue);

    // Issue the QT command to the MAC
    m_pMac->WriteRegister(HST_CMD, CMD_18K6CQT);

    // Perform the common 18K6C tag-operation request post setup
    this->PostMacCommandIssue();

} //  Radio::Start18K6CQT


////////////////////////////////////////////////////////////////////////////////
// Name:        Setup18K6CBlockEraseRegisters
// Description: Configures the tag block erase specific registers in the radio 
//              module.
////////////////////////////////////////////////////////////////////////////////
void Radio::Setup18K6CBlockEraseRegisters(
    const RFID_18K6C_BLOCK_ERASE_CMD_PARMS*   pParms
    )
{
    // Set up the access bank register
    m_pMac->WriteRegister(HST_TAGACC_BANK,
                          HST_TAGACC_BANK_BANK(pParms->bank) |
                          HST_TAGACC_BANK_RFU1(0));

    // Set up the access pointer register (tells the offset)
    m_pMac->WriteRegister(HST_TAGACC_PTR, pParms->offset);

    // Set up the access count register (i.e., number values to read)
    m_pMac->WriteRegister(HST_TAGACC_CNT,
                          HST_TAGACC_CNT_LEN(pParms->count) |
                          HST_TAGACC_CNT_RFU1(0));
}


////////////////////////////////////////////////////////////////////////////////
// Name:        Start18K6CBlockErase
// Description: Requests that an ISO 18000-6C tag block erase be started on the
//              radio module.
////////////////////////////////////////////////////////////////////////////////
void Radio::Start18K6CBlockErase(
    const RFID_18K6C_BLOCK_ERASE_PARMS*    pParms,
    INT32U                                 flags
    )
{
    assert(NULL != pParms);

    // Perform the common 18K6C tag operation setup
    this->Start18K6CRequest(&pParms->common, flags);

    this->Setup18K6CBlockEraseRegisters(&pParms->blockEraseCmdParms);

    // Set up the access password register
    m_pMac->WriteRegister(HST_TAGACC_ACCPWD, pParms->accessPassword);

    // Issue the read command
    m_pMac->WriteRegister(HST_CMD, CMD_18K6CBLOCKERASE);

    // Perform the common 18K6C tag-operation request post setup
    this->PostMacCommandIssue();
} //  Radio::Start18K6CBlockErase



////////////////////////////////////////////////////////////////////////////////
// Name:        ProcessOperationData
// Description: Requests that the radio object begin receiving operation
//              response packets from the radio module and invoking the 
//              supplied callback.
////////////////////////////////////////////////////////////////////////////////
void Radio::ProcessOperationData(
    RFID_RADIO_HANDLE               handle,
    RFID_PACKET_CALLBACK_FUNCTION   pCallback,
    void*                           context,
    INT32S*                         pCallbackCode,
    bool                            canBeCancelled
    )
{
    CplMutexAutoLock    cancelAbortGuard;
    PACKET_BUFFER       buffer;
    INT32U              bufferSize;
    bool                sawCommandEnd = false;
    INT32S              status;
    RFID_STATUS         result = RFID_STATUS_OK;

    // If we can be cancelled, grab the abort/cancel lock again (a precondition
    // to being able to be cancelled is that the lock is already held).
    // It is already locked and we want to ensure that it is released on exit.
    // This lets a cancel or abort call know that we have completed processing
    // of the packets.
    if (canBeCancelled)
    {
        cancelAbortGuard.Assume(&m_cancelAbortLock);
    }

    if (NULL != pCallbackCode)
    {
        *pCallbackCode = 0;
    }

    // Process packets until we see a command-end (note that some event, such
    // as an abort, cause us to break out early).
    while (!sawCommandEnd)
    {
        try
        {
            // Retrieve the next packet
            this->RetrieveNextPacket(bufferSize, buffer, canBeCancelled);

            // Check for 32-bit alignment.
            assert(!(reinterpret_cast<INT32U>(&buffer[0]) & 0x00000003));

            // Check to see if this is the end packet
            sawCommandEnd = 
                (RFID_PACKET_TYPE_COMMAND_END == 
                 CPL_HostToMac16(
                    reinterpret_cast<RFID_PACKET_COMMON *>(
                        &buffer[0])->pkt_type));

            // If a callback was provided, invoke it
            if (NULL != pCallback)
            {
                // If the application callback returned a non-zero value, then it
                // doesn't care to receive any more packets...that includes the
                // command-end packet.
                status = pCallback(handle, bufferSize, &buffer[0], context);
                if (status)
                {
                    g_pTracer->PrintMessage(
                        Tracer::RFID_LOG_SEVERITY_INFO,
                        "%s: Packet callback returned %d\n",
                        __FUNCTION__,
                        status);

                    if (NULL != pCallbackCode)
                    {
                        *pCallbackCode = status;
                    }

                    // Indicate that we should abort packet processing
                    m_shouldAbort = true;
                }
            }
        }
        catch (RfidErrorException& exception)
        {
            result = exception.GetError();
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Caught an exection with error 0x%.8x\n",
                __FUNCTION__,
                result);
            break;
        }
        catch (...)
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Caught an unexpected exception\n",
                __FUNCTION__);
            result = RFID_ERROR_FAILURE;
            break;
        }
    } // while (!sawCommandEnd)

    // Indicate not busy any more
    m_isBusy = false;

    // Check to see if we need to report an error (or result status in the
    // case of a cancelled operation)
    if (RFID_STATUS_OK != result)
    {
        if ((RFID_ERROR_OPERATION_CANCELLED  != result) &&
            (RFID_ERROR_RADIO_NOT_RESPONDING != result))
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Send an ABORT request to radio\n",
                __FUNCTION__);
            m_pMac->AbortOperation();
        }

        throw RfidErrorException(result, __FUNCTION__);
    }
    else if (canBeCancelled && m_operationCancelled)
    {
        throw RfidErrorException(RFID_ERROR_OPERATION_CANCELLED, __FUNCTION__);
    }
} // Radio::ProcessOperationData

////////////////////////////////////////////////////////////////////////////////
// Name:        CancelOperation
// Description: Requests that the radio object instruct the underlying radio
//              module to cancel its current operation.  When cancelling,
//              the remaining packets will still be delivered via the
//              packet callback function.
////////////////////////////////////////////////////////////////////////////////
void Radio::CancelOperation()
{
    // Verify that the cancel is not being called from the same thread that
    // is performing the packet callback
    if (m_isBusy && (CPL_ThreadGetID() == m_busyThread))
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot cancel operation on callback thread\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_CURRENTLY_NOT_ALLOWED, __FUNCTION__);
    }

    // Indicate that a cancel is to be issued
    g_pTracer->PrintMessage(
        Tracer::RFID_LOG_SEVERITY_DEBUG,
        "%s: Cancel operation and wait until cancel completed\n",
        __FUNCTION__);
    m_shouldCancel = true;

    // Wait until the radio has completed the processing of the cancel
    CplMutexAutoLock cancelGuard(&m_cancelAbortLock);
    g_pTracer->PrintMessage(
        Tracer::RFID_LOG_SEVERITY_DEBUG,
        "%s: Cancel has completed\n",
        __FUNCTION__);
} // Radio::CancelOperation

////////////////////////////////////////////////////////////////////////////////
// Name:        AbortOperation
// Description: Requests that the radio object instruct the underlying radio
//              module to abort its current operation.  When aborting,
//              the remaining packets are discarded.
////////////////////////////////////////////////////////////////////////////////
void Radio::AbortOperation()
{
    // Verify that the cancel is not being called from the same thread that
    // is performing the packet callback
    if (m_isBusy && (CPL_ThreadGetID() == m_busyThread))
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot abort operation on callback thread\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_CURRENTLY_NOT_ALLOWED, __FUNCTION__);
    }

    // Indicate that an abort is to be issued
    g_pTracer->PrintMessage(
        Tracer::RFID_LOG_SEVERITY_DEBUG,
        "%s: Abort operation and wait until abort completed\n",
        __FUNCTION__);
    m_shouldAbort = true;

    // Wait until the radio has completed the processing of the abort
    CplMutexAutoLock abortGuard(&m_cancelAbortLock);
    g_pTracer->PrintMessage(
        Tracer::RFID_LOG_SEVERITY_DEBUG,
        "%s: Abort has completed\n",
        __FUNCTION__);
} // Radio::AbortOperation

////////////////////////////////////////////////////////////////////////////////
// Name:        SetResponseDataMode
// Description: Sets the response date mode (i.e., compact, etc.) for the
////////////////////////////////////////////////////////////////////////////////
void Radio::SetResponseDataMode(
    RFID_RESPONSE_TYPE  type,
    RFID_RESPONSE_MODE  mode
    )
{
    INT32U registerValue = 0;

    RFID_UNREFERENCED_LOCAL(type);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Based upon the mode selected, set the common diagnostics register
    // appropriately
    switch (mode)
    {
        case RFID_RESPONSE_MODE_EXTENDED:
        {
            // Set the diagnostics bit in the register
            HST_CMNDIAGS_SET_DIAGS_ENABLED(registerValue);
            // Fall through on purpose
        } // case RFID_RESPONSE_MODE_EXTENDED
        case RFID_RESPONSE_MODE_NORMAL:
        {
            // Set the status bit in the register
            HST_CMNDIAGS_SET_STATUS_ENABLED(registerValue);
            // Fall through on purpose
        } // case RFID_RESPONSE_MODE_NORMAL
        case RFID_RESPONSE_MODE_COMPACT:
        {
            // Set the inventory response bit in the register
            HST_CMNDIAGS_SET_INVRESP_ENABLED(registerValue);
            HST_CMNDIAGS_SET_COMMANDACTIVE_ENABLED(registerValue);
            break;
        } // case RFID_RESPONSE_MODE_COMPACT
        default:
        {
            throw RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            break;
        } // default
    } // switch (mode)

    // Write the common diagnostics register
    m_pMac->WriteRegister(HST_CMNDIAGS, registerValue);
} // Radio::SetResponseDataMode

////////////////////////////////////////////////////////////////////////////////
// Name:        GetResponseDataMode
// Description: Retrieves the response date mode (i.e., compact, etc.) for
//              the type of data from the radio
////////////////////////////////////////////////////////////////////////////////
RFID_RESPONSE_MODE Radio::GetResponseDataMode(
    RFID_RESPONSE_TYPE  type
    )
{
    RFID_RADIO_OPERATION_MODE mode;

    RFID_UNREFERENCED_LOCAL(type);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Read the common diagnostics register
    INT32U registerValue = m_pMac->ReadRegister(HST_CMNDIAGS);

    // DMS Bug 9035:  Changing the logic to require specific complement of bits 
    // to be set for each mode, otherwise throw an exception. 
    //    Compact  = only inv resp and command active bits are set
    //    Normal   = inv resp, command active, and status bits are  set
    //    Extended = inv resp, command actvie, status, and diag bits are set
    
    // Figure out the mode for the packets
    if(registerValue == (HST_CMNDIAGS_INVRESP_ENABLED | 
                         HST_CMNDIAGS_COMMANDACTIVE_ENABLED))
    {
        mode = RFID_RESPONSE_MODE_COMPACT;
    }
    else if (registerValue == (HST_CMNDIAGS_INVRESP_ENABLED       | 
                               HST_CMNDIAGS_COMMANDACTIVE_ENABLED | 
                               HST_CMNDIAGS_STATUS_ENABLED))
    {
        mode = RFID_RESPONSE_MODE_NORMAL;
    }
    else if (registerValue == (HST_CMNDIAGS_INVRESP_ENABLED       | 
                               HST_CMNDIAGS_COMMANDACTIVE_ENABLED | 
                               HST_CMNDIAGS_STATUS_ENABLED        |
                               HST_CMNDIAGS_DIAGS_ENABLED))
    {
        mode = RFID_RESPONSE_MODE_EXTENDED;
    }
    else
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Non-predefined set of packets are enabled\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_UNEXPECTED_VALUE, __FUNCTION__);
    }

    return mode;
} // Radio::GetResponseDataMode

////////////////////////////////////////////////////////////////////////////
// Name:        UpdateMacNonvolatileMemory
// Description: Updates the MAC's nonvolatile memory blocks specified
////////////////////////////////////////////////////////////////////////////
void Radio::UpdateMacNonvolatileMemory(
    INT32U                                  countBlocks,
    const RFID_NONVOLATILE_MEMORY_BLOCK*    pBlocks,
    INT32U                                  flags
    )
{
    RFID_STATUS status = RFID_STATUS_OK;
    INT32U rawPacketStatus    = 0;
    INT16U packetStatus    = 0;
    INT16U packetError    = 0;
    INT32U packetRetries   = 0;
    INT32U packetCrc       = 0;
    RFID_STATUS nv_error   = RFID_STATUS_OK;

    assert(countBlocks && (NULL != pBlocks));

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Send the MAC the command to put it into the nonvolatile memory update
    // state
    m_pMac->WriteRegister(HST_CMD, CMD_NV_MEM_UPDATE);

    // Process the MAC's command-response packets
    PACKET_CALLBACK_DATA callbackData;
    callbackData.pRadio  = this;
    callbackData.command = CMD_NV_MEM_UPDATE;
    callbackData.state   = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;

    this->ProcessOperationData(RFID_INVALID_RADIO_HANDLE,
                               Radio::PacketCallbackFunction,
                               &callbackData,
                               NULL,
                               false);

    // If the command failed, clear the MAC error and return an error
    if (callbackData.status)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Initiating nonvolatile memory update failed with status "
            "0x%.8x\n",
            __FUNCTION__,
            callbackData.status);

        throw RfidErrorException(RFID_ERROR_NONVOLATILE_INIT_FAILED, __FUNCTION__);
    }

    // Get the initial status packet from the MAC to ensure that it did indeed
    // enter the nonvolatile memory update state
    NVMEMPKT_STATUS responsePacket;

    m_pMac->RetrieveData(
        reinterpret_cast<INT8U *>(&responsePacket),
        sizeof(responsePacket));

    rawPacketStatus = CPL_MacToHost32(responsePacket.status);
    packetStatus    = (rawPacketStatus & 0xFFFF);
    packetError     = ((rawPacketStatus>>16) & 0xFFFF);

    if (NVMEMUPD_STAT_ENTRY_OK != packetStatus)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: MAC failed to enter non-volatile update mode (status=0x%.8x, error=0x%.8x)\n",
             __FUNCTION__, packetStatus, packetError);
        throw RfidErrorException(RFID_ERROR_NONVOLATILE_INIT_FAILED, __FUNCTION__);
    }

    try
    {
        // Create a buffer for the host-side packets.  Also overlay a structure
        // on it to make it easier to create the packets
        std::vector<INT8U>  buffer(
            callbackData.commandData.nvMemUpdate.maxPacketSize *
            NVMEMPKT_BYTES_PER_UNIT);
        NVMEMPKT_UPD_RANGE* pUpdatePacket = 
            reinterpret_cast<NVMEMPKT_UPD_RANGE *>(&buffer[0]);
        INT32U              updateFlags = 
            flags & RFID_FLAG_NVMEM_UPDATE_TEST ?
                NVMEMPKT_UPD_RANGE_TESTMODE : 0;

        pUpdatePacket->cmn.magic    = CPL_HostToMac16(NVMEMUPD_PKT_MAGIC);
        pUpdatePacket->cmn.cmd      = CPL_HostToMac16(NVMEMUPD_CMD_UPD_RANGE);
        pUpdatePacket->cmn.res0     = CPL_HostToMac16(0);

        // Now write each block
        for (INT32U block = 0; block < countBlocks; ++block)
        {
            INT32U  chunkSize   = 
                (callbackData.commandData.nvMemUpdate.maxPacketSize -
                 (NVMEMPKT_LEN_MIN_UPD_RANGE + NVMEMPKT_CMN_LEN)) *
                NVMEMPKT_BYTES_PER_UNIT;
            INT32U  address     = pBlocks[block].address;
            INT32U  length      = pBlocks[block].length;
            INT8U*  pData       = pBlocks[block].pData;

            // Except for the last update packet, we'll be sending full packets
            // so we can set the length to the max
            pUpdatePacket->cmn.pkt_len = 
                CPL_HostToMac16(
                    (INT16U) (callbackData.commandData.nvMemUpdate.maxPacketSize -
                    NVMEMPKT_CMN_LEN));

            while (length)
            {
                // Determine if this is the last chunk and it is shorter than
                // the chunk size.  If so, adjust.
                if (length < chunkSize)
                {
                    chunkSize = length;
                    pUpdatePacket->cmn.pkt_len = 
                        CPL_HostToMac16( 
                            (INT16U) (NVMEMPKT_LEN_MIN_UPD_RANGE + 
                                     (chunkSize + NVMEMPKT_BYTES_PER_UNIT - 1) / 
                                        NVMEMPKT_BYTES_PER_UNIT));
                }
                    
                // Build the rest of the packet
                INT32U paddingBytes = 
                    (NVMEMPKT_BYTES_PER_UNIT -
                        (chunkSize % NVMEMPKT_BYTES_PER_UNIT)) %
                     NVMEMPKT_BYTES_PER_UNIT;
                pUpdatePacket->abs_addr = CPL_HostToMac32(address);
                NVMEMPKT_UPD_RANGE_SET_PADDING_BYTES(
                    updateFlags,
                    paddingBytes);
                pUpdatePacket->flags = CPL_HostToMac32(updateFlags);
                memcpy(pUpdatePacket->upd_data, pData, chunkSize);
                if (paddingBytes)
                {
                    memset(
                        reinterpret_cast<INT8U *>(pUpdatePacket->upd_data) +
                        chunkSize,
                        0,
                        paddingBytes);
                }

                packetCrc = 0;
                // first get the crc upto but not including the crc field
                packetCrc = this->FastCrc32(packetCrc, 
                                            reinterpret_cast<unsigned char *>(&pUpdatePacket->cmn), 
                                            (sizeof(pUpdatePacket->cmn) + sizeof(pUpdatePacket->abs_addr)));

                // now complete the CRC from the first byte after the crc up through the last byte of data
                packetCrc = this->FastCrc32(packetCrc, 
                                            reinterpret_cast<unsigned char *>(&pUpdatePacket->flags), 
                                            (sizeof(pUpdatePacket->flags) + chunkSize));

                // now store it in the crc in MAC format
                pUpdatePacket->crc          = CPL_HostToMac32(packetCrc);

                packetError   = 0;
                packetStatus  = 0;
                packetRetries = 0;
                nv_error      = RFID_STATUS_OK;
                do
                {
                    // Send the update packet and get the MAC's response
                    m_pMac->SendData(
                        reinterpret_cast<INT8U *>(pUpdatePacket),
                        (NVMEMPKT_CMN_LEN + NVMEMPKT_LEN_MIN_UPD_RANGE) *
                        NVMEMPKT_BYTES_PER_UNIT                         +
                        chunkSize                                       +
                        paddingBytes);
                    m_pMac->RetrieveData(
                        reinterpret_cast<INT8U *>(&responsePacket),
                        sizeof(responsePacket));
    
                        rawPacketStatus = CPL_MacToHost32(responsePacket.status);
                        packetStatus    = (rawPacketStatus & 0xFFFF);
                        packetError     = ((rawPacketStatus>>16) & 0xFFFF);

                    // Act upon the return status
                    switch (packetStatus)
                    {
                        case NVMEMUPD_STAT_UPD_SUCCESS:
                        {
                                /* reset the retry counter on success packets */
                                packetRetries = 0;
                                nv_error = RFID_STATUS_OK;
                            break;
                        } // case NVMEMUPD_STAT_UPD_SUCCESS
                        
                        case NVMEMUPD_STAT_GEN_RXPKT_ERR:
                        case NVMEMUPD_STAT_RX_TO:
                        case NVMEMUPD_STAT_INT_MEM_BNDS:
                        case NVMEMUPD_STAT_WR_FAIL:
                        {

                            /* these are fatal/critical errors and the firmware will is going
                               to reset, so we error out here to let the application reconnect.
                               No retries on critical errors. */
                            g_pTracer->PrintMessage(
                                Tracer::RFID_LOG_SEVERITY_ERROR,
                                "%s: Critical error on write to nonvolatile memory (status=0x%.8x, error=0x%.8x), MAC will reset\n",
                                 __FUNCTION__, packetStatus, packetError);
                            throw 
                                RfidErrorException(
                                    RFID_ERROR_NONVOLATILE_WRITE_FAILED, __FUNCTION__);
                            break;

                        }// case critical errors
                        
                        case NVMEMUPD_STAT_CRC_ERR:
                        {
                            g_pTracer->PrintMessage(
                                Tracer::RFID_LOG_SEVERITY_ERROR,
                                "%s: CRC Error on MAC write to nonvolatile memory (status=0x%.8x, error=0x%.8x)\n",
                                 __FUNCTION__, packetStatus, packetError);

                            packetRetries++;
                            nv_error = RFID_ERROR_NONVOLATILE_CRC_FAILED;
                            break;
                        } // case NVMEMUPD_STAT_CRC_ERR
                        
                        
                        case NVMEMUPD_STAT_BNDS:
                        {
                            g_pTracer->PrintMessage(
                                    Tracer::RFID_LOG_SEVERITY_ERROR,
                                "%s: Attempted to write to invalid nonvolatile "
                                    "memory address (address=0x%.8x, length=%d, status=0x%.8x, error=0x%.8x)\n",
                                     __FUNCTION__, address, chunkSize, packetStatus, packetError);
                                packetRetries++;
                                nv_error = RFID_ERROR_NONVOLATILE_OUT_OF_BOUNDS;
                            break;
                        } // case NVMEMUPD_STAT_BNDS
                            
                        case NVMEMUPD_STAT_RXPKT_MAX:
                        {
                            g_pTracer->PrintMessage(
                                    Tracer::RFID_LOG_SEVERITY_ERROR,
                                "%s: Exceeded max packet size "
                                    "(status=0x%.8x, error=0x%.8x)\n",
                                     __FUNCTION__, packetStatus, packetError);
                                packetRetries++;
                                nv_error = RFID_ERROR_NONVOLATILE_MAX_PACKET_LENGTH;
                            break;
                        } // case NVMEMUPD_STAT_RXPKT_MAX
                        
                        case NVMEMUPD_STAT_UNK_CMD:
                        case NVMEMUPD_STAT_CMD_IGN:
                        case NVMEMUPD_STAT_MAGIC:
                        case NVMEMUPD_STAT_PKTLEN:
                        {
                            g_pTracer->PrintMessage(
                                    Tracer::RFID_LOG_SEVERITY_ERROR,
                                "%s: Unexpected values in the packet header "
                                    "(status=0x%.8x, error=0x%.8x)\n",
                                     __FUNCTION__, packetStatus, packetError);
                                packetRetries++;
                                nv_error = RFID_ERROR_NONVOLATILE_PACKET_HEADER;
                            break;
                        } // case packet header errors
                        
                        default:
                        {
                            g_pTracer->PrintMessage(
                                Tracer::RFID_LOG_SEVERITY_ERROR,
                                    "%s: MAC returned unexpected status (status=0x%.8x, error=0x%.8x)\n",
                                 __FUNCTION__,
                                     packetStatus,
                                     packetError);
                                packetRetries++;
                                nv_error = RFID_ERROR_FAILURE;
                            break;
                        } // default
                    } // switch packetStatus
                } while ((packetStatus != NVMEMUPD_STAT_UPD_SUCCESS) && (packetRetries < MAX_NV_ATTEMPTS));

                if (packetRetries >= MAX_NV_ATTEMPTS)
                {
                    g_pTracer->PrintMessage(
                        Tracer::RFID_LOG_SEVERITY_ERROR,
                        "%s: MAC exceeded NV Update retries\n",
                         __FUNCTION__);

                    throw 
                        RfidErrorException(nv_error, __FUNCTION__);
                }
                // Update the length, address, and data pointer
                length  -= chunkSize;
                address += chunkSize;
                pData   += chunkSize;
            }
        }
    }
    catch (RfidErrorException &exception)
    {
        status = exception.GetError();
    }

    if (RFID_ERROR_NONVOLATILE_WRITE_FAILED == status)
    {
        // If it was a fatal error the radio is already resetting, so we 
        // just rethrow the exception without sending the update complete packet.
        //
        throw RfidErrorException(status, __FUNCTION__);
    }
    else
    {
        // In all other cases we try to send the update complete packet.  This way, if we 
        // (the host) gave up due to successive non-fatal errors and the 
        // firmware is still waiting, the udpate complete packet will terminate the  
        // update process for the firmware and it can reset.  
        
        // Create the update complete packet, send it, then wait for the response
        NVMEMPKT_UPD_COMPLETE updateCompletePacket =
        {
            CPL_HostToMac16(NVMEMUPD_PKT_MAGIC),
            CPL_HostToMac16(NVMEMUPD_CMD_UPD_COMPLETE),
            CPL_HostToMac16(0),
            CPL_HostToMac16(0)
        };
    
        m_pMac->SendData(
            reinterpret_cast<INT8U *>(&updateCompletePacket),
            sizeof(updateCompletePacket));
        m_pMac->RetrieveData(
            reinterpret_cast<INT8U *>(&responsePacket),
            sizeof(responsePacket));
    
        rawPacketStatus = CPL_MacToHost32(responsePacket.status);
        packetStatus    = (rawPacketStatus & 0xFFFF);
        packetError     = ((rawPacketStatus>>16) & 0xFFFF);


        // If status is not ok, then we must have short circuited out of sending
        // the update range packets, so rethrow the original exception without
        // regard to the response from the update complete packet
        // 
        if (RFID_STATUS_OK != status)
        {        
            throw RfidErrorException(status, __FUNCTION__);
        }
        else 
        {
            // all the update range packets were sent succesfully, so look at
            // the final packetStatus from the update complete packet to confirm final
            // success or failure declaration
            
            // Check the return status on update complete packet
            switch (packetStatus)
            {
                case NVMEMUPD_STAT_EXIT_SUCCESS:
                case NVMEMUPD_STAT_EXIT_NOWRITES:
                {
                    // Verify that the response matches the packet set
                    if (NVMEMUPD_CMD_UPD_COMPLETE == 
                        CPL_MacToHost32(responsePacket.re_cmd))
                    {
                        g_pTracer->PrintMessage(
                            Tracer::RFID_LOG_SEVERITY_INFO,
                            "%s: MAC's nonvolatile memory updated successfully\n",
                            __FUNCTION__);
                        break;
                    }

                    // FALL THROUGH INTENTIONAL

                }
                default:
                {
                    g_pTracer->PrintMessage(
                        Tracer::RFID_LOG_SEVERITY_ERROR,
                        "%s: MAC returned unexpected status or reponse to update "
                        "complete (status=0x%.8x, error=0x%.8x, re_cmd=0x%.8x)\n",
                         __FUNCTION__,
                         packetStatus,
                         packetError,
                         CPL_MacToHost32(responsePacket.re_cmd));
                    throw RfidErrorException(RFID_ERROR_NONVOLATILE_WRITE_FAILED, __FUNCTION__);
                    break;
                }
            } // switch (packetStatus)
        } // else (status RFID_STATUS_OK)
    } // else (status RFID_ERROR_NONVOLATILE_WRITE_FAILED)
} // Radio::UpdateMacNonvolatileMemory

////////////////////////////////////////////////////////////////////////////////
// Name:        GetMacVersion
// Description: Request to retrieve the version for the MAC firmware on the
//              radio module.
////////////////////////////////////////////////////////////////////////////////
void Radio::GetMacVersion(
    RFID_VERSION* pVersion
    )
{
    assert(NULL != pVersion);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Retrieve the version from the radio
    INT32U macVersion = m_pMac->ReadRegister(MAC_VER);

    // validate the return value.  if it is all F's, then the MAC Firmware  
    // image is not valid, so the version could not be read.  This would 
    // be the case when communicating with the BootLoader and trying to read
    // the MAC firmware version.
    if (macVersion == 0xFFFFFFFF)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Invalid MAC Firmware Version\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_UNEXPECTED_VALUE, __FUNCTION__);
    }

    // Convert the MAC version into the library's representation
    pVersion->major       = MAC_VER_GET_MAJ(macVersion);
    pVersion->minor       = MAC_VER_GET_MIN(macVersion);
    pVersion->maintenance = MAC_VER_GET_MAINT(macVersion);
    pVersion->release     = MAC_VER_GET_REL(macVersion);
} // Radio::GetMacVersion


////////////////////////////////////////////////////////////////////////////////
// Name:        GetMacBootLoaderVersion
// Description: Request to retrieve the version for the BootLoader image on the
//              radio module.
////////////////////////////////////////////////////////////////////////////////
void Radio::GetMacBootLoaderVersion(
    RFID_VERSION* pVersion
    )
{
    INT32U blVersion;

    assert(NULL != pVersion);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Try catch around this call, so that calls into older firmware
    // that do not support the MAC_BL_VER register will throw 
    // RFID_ERROR_INVALID_PARAMETER and we can catch it so we throw a
    // more meaningful error for this function.
    try
    {
        // Retrieve the version from radio
        blVersion = m_pMac->ReadRegister(MAC_BL_VER);
    }
    catch (rfid::RfidErrorException& error)
    {
        if (error.GetError() == RFID_ERROR_INVALID_PARAMETER)
        {
            throw RfidErrorException(RFID_ERROR_NOT_SUPPORTED, __FUNCTION__);
        }
        else // just propagate other exceptions
        {
            throw;
        }
    }

    // validate the return value.  if it is all F's, then the BootLoader  
    // image is not valid, so the version could not be read.  This would 
    // be the case when communicating with the MAC Firmwarre and trying to read
    // the BootLoader version.
    if (blVersion == 0xFFFFFFFF)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Invalid BootLoader Version\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_UNEXPECTED_VALUE, __FUNCTION__);
    }

    // Convert the BootLoader version into the library's representation
    pVersion->major       = MAC_VER_GET_MAJ(blVersion);
    pVersion->minor       = MAC_VER_GET_MIN(blVersion);
    pVersion->maintenance = MAC_VER_GET_MAINT(blVersion);
    pVersion->release     = MAC_VER_GET_REL(blVersion);
} // Radio::GetMacBootLoaderVersion
  
////////////////////////////////////////////////////////////////////////////////
// Name:        ReadMacOemData
// Description: Reads the requested data from the MAC OEM configuration area
////////////////////////////////////////////////////////////////////////////////
void Radio::ReadMacOemData(
    INT32U  address,
    INT32U* pCount,
    INT32U* pData
    )
{
    INT32U locCount = 0;

    assert(NULL != pData);
    assert(NULL != pCount);

    try
    {
        // If the radio is busy, don't allow this operation
        if (m_isBusy)
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Cannot complete request as radio is busy\n",
                __FUNCTION__);
            throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
        }
    
        // Since the MAC only allows us to read one OEM address at a time, we'll
        // have to loop to do the read
        for (locCount=0; locCount < *pCount; locCount++)
        {
            // Tell the MAC which address we want to read
            m_pMac->WriteRegister(HST_OEM_ADDR,
                                  HST_OEM_ADDR_OEMADDR(address) |
                                  HST_OEM_ADDR_RFU1(0));
    
            // Send the read OEM command to the MAC
            m_pMac->WriteRegister(HST_CMD, CMD_RDOEM);
    
            // Process the MAC's command-response packets
            PACKET_CALLBACK_DATA callbackData;
            callbackData.pRadio  = this;
            callbackData.command = CMD_RDOEM;
            callbackData.state   = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;
    
            this->ProcessOperationData(RFID_INVALID_RADIO_HANDLE,
                                       Radio::PacketCallbackFunction,
                                       &callbackData,
                                       NULL,
                                       false);
    
            // If the command failed, clear the MAC error and return an error
            if (callbackData.status)
            {
                g_pTracer->PrintMessage(
                    Tracer::RFID_LOG_SEVERITY_ERROR,
                    "%s: OEM read failed with status 0x%.8x\n",
                    __FUNCTION__,
                    callbackData.status);
    
                this->ClearMacError();
                throw RfidErrorException(
                    IO_OEMCFG_ADDR_BOUNDS == callbackData.status ?
                        RFID_ERROR_INVALID_PARAMETER : 
                        RFID_ERROR_RADIO_FAILURE, __FUNCTION__);
            }
    
            assert(callbackData.commandData.oemRead.address == address);
    
            // Copy the data and advance address and pointers
            //
            *pData = callbackData.commandData.oemRead.value;
            address++;
            pData++;
        }
    }
    catch (...)
    {
        // need to update the pCount to reflect how much was read and 
        // propagate the exception
        *pCount = locCount;
        throw;
    }

    // update the pCount before we return
    *pCount = locCount;

} // Radio::ReadMacOemData

////////////////////////////////////////////////////////////////////////////////
// Name:        WriteMacOemData
// Description: Writes the supplied data to the MAC OEM configuration area
////////////////////////////////////////////////////////////////////////////////
void Radio::WriteMacOemData(
    INT32U          address,
    INT32U*         pCount,
    const INT32U*   pData
    )
{
    INT32U locCount = 0;

    assert(NULL != pData);
    assert(NULL != pCount);

    try
    {
        // If the radio is busy, don't allow this operation
        if (m_isBusy)
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Cannot complete request as radio is busy\n",
                __FUNCTION__);
            throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
        }
    
        // Since the MAC only allows us to write one OEM address at a time, we'll
        // have to loop to do the write
        for (locCount = 0; locCount<*pCount; locCount++)
        {
            // Tell the MAC which address we want to write to and the data to
            // write to that address.  
            // NOTE: The input data is supposed to be in MAC Format, so we need
            //       to convert to Host format, so the underlying conversion back to 
            //       MAC Format that is performed in Mac::WriteRegister will do 
            //       the right thing.
            m_pMac->WriteRegister(HST_OEM_ADDR,
                                  HST_OEM_ADDR_OEMADDR(address) |
                                  HST_OEM_ADDR_RFU1(0));
            m_pMac->WriteRegister(HST_OEM_DATA, CPL_MacToHost32(*pData));
    
            // Send the write OEM command to the MAC
            m_pMac->WriteRegister(HST_CMD, CMD_WROEM);
    
            // Process the MAC's command-response packets
            PACKET_CALLBACK_DATA callbackData;
            callbackData.pRadio  = this;
            callbackData.command = CMD_WROEM;
            callbackData.state   = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;
    
            this->ProcessOperationData(RFID_INVALID_RADIO_HANDLE,
                                       Radio::PacketCallbackFunction,
                                       &callbackData,
                                       NULL,
                                       false);
    
            // If the command failed, clear the MAC error and return an error
            if (callbackData.status)
            {
                g_pTracer->PrintMessage(
                    Tracer::RFID_LOG_SEVERITY_ERROR,
                    "%s: OEM write failed with status 0x%.8x\n",
                    __FUNCTION__,
                    callbackData.status);
    
                this->ClearMacError();
                throw RfidErrorException(
                    IO_OEMCFG_ADDR_BOUNDS == callbackData.status ?
                        RFID_ERROR_INVALID_PARAMETER : 
                        RFID_ERROR_RADIO_FAILURE, __FUNCTION__);
            }

            // Advance address and pointers
            address++;
            pData++;
        }
    }
    catch (...)
    {
        // need to update the pCount to reflect how much was written and 
        // propagate the exception
        *pCount = locCount;
        throw;
    }

    // update the pCount before we return
    *pCount = locCount;

} // Radio::WriteMacOemData

////////////////////////////////////////////////////////////////////////////////
// Name:        ResetMac
// Description: Reset the radio's MAC
////////////////////////////////////////////////////////////////////////////////
void Radio::ResetMac(
    RFID_MAC_RESET_TYPE type
    )
{
    RFID_STATUS error;

    // temporary backward compatibility logic... need to read MAC_NV_UPDATE_CONTROL register
    // as a means to know if underlying MAC firmware supports RFID_MAC_RESET_TYPE_TO_BOOTLOADER.
    // This register was introduced in the same version as the Boot Loader support, so this is
    // a good test.  Could use a version test, but MAC Firmware and Boot Loader could be versioned
    // independently, and the format of the MAC Version changed in v2.2, so it's a little tough
    // if there is pre 2.2 firmware on the hardware. 
    // The reason this check is needed is that an invalid control command (like reset) will put the
    // MAC Firmware into the FAIL state which we want to avoid.
    //
    if (RFID_MAC_RESET_TYPE_TO_BOOTLOADER == type)
    {
        // We count on an INVALID_PARAMETER exception being thrown.  Catch it and throw
        // NOT_SUPPORTED to be more clear.
        // We don't care about the value returned.
        try
        {
            this->ReadMacRegister(MAC_NV_UPDATE_CONTROL);
        }
        catch (RfidErrorException &exception)
        {
            error = exception.GetError();
            if (RFID_ERROR_INVALID_PARAMETER == error)
            {
                // rethrow as NOT_SUPPORTED
                throw RfidErrorException(RFID_ERROR_NOT_SUPPORTED, __FUNCTION__);
            }
            else
            {
                // just propagate it up
                throw;
            }
        }
    }

    // if we make it to here, we're good to go and issue the reset.
    
    // Cancel any current operation
    this->AbortOperation();
    
    // Simply tell the MAC to reset
    m_pMac->Reset(type);
} // Radio::ResetMac

////////////////////////////////////////////////////////////////////////////
// Name:        ClearMacError
// Description: Clears any error condition on the MAC
////////////////////////////////////////////////////////////////////////////
void Radio::ClearMacError()
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Send the clear error to the MAC
    m_pMac->WriteRegister(HST_CMD, CMD_CLRERR);

    // Process the MAC's command-response packets
    PACKET_CALLBACK_DATA callbackData;
    callbackData.pRadio  = this;
    callbackData.command = CMD_CLRERR;
    callbackData.state   = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;

    this->ProcessOperationData(RFID_INVALID_RADIO_HANDLE,
                               Radio::PacketCallbackFunction,
                               &callbackData,
                               NULL,
                               false);
} // Radio::ClearMacError


////////////////////////////////////////////////////////////////////////////
// Name:        GetMacError
// Description: Retrieves the current MAC_ERROR and LAST_MAC_ERROR from
//              the radio
////////////////////////////////////////////////////////////////////////////
void Radio::GetMacError(
    INT32U*             pError,
    INT32U*             pLastError
    )
{
    // If the radio is busy, don't allow this operation
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    *pError = this->ReadMacRegister(MAC_ERROR);

    if (m_preTwoTwoFirmware)
    {
        *pLastError = 0;  // feature not available prior to 2.2
    }
    else

    {
        *pLastError = this->ReadMacRegister(MAC_LAST_ERROR);
    }

} // Radio::GetMacError



////////////////////////////////////////////////////////////////////////////
// Name:        GetMacRegion
// Description: Retrieves the MAC's region of operation.
////////////////////////////////////////////////////////////////////////////
RFID_MAC_REGION Radio::GetMacRegion(
    void*   pRegionConfig
    )
{
    RFID_MAC_REGION currentRegion;

    RFID_UNREFERENCED_LOCAL(pRegionConfig);
    
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Read the MAC's state machine index (that is the region in use)
    currentRegion =  m_pMac->ReadRegister(HST_REGULATORY_REGION);

    // make sure the regulatory region is in range, and error if not
    // test against the highest defined region value
    if (RFID_MAC_REGION_ETSI_LBT < currentRegion)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Unexpected value returned for HST_REGULATORY_REGION (%d)\n",
            __FUNCTION__, currentRegion);
        throw RfidErrorException(RFID_ERROR_UNEXPECTED_VALUE, __FUNCTION__);
    }
    return currentRegion;
} // Radio::GetMacRegion

////////////////////////////////////////////////////////////////////////////////
// Name:        SetGpioPinsConfiguration
// Description: Sets the GPIO pin configuration for the radio
////////////////////////////////////////////////////////////////////////////////
void Radio::SetGpioPinsConfiguration(
    INT32U  mask,
    INT32U  configuration
    )
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // First read the GPIO config register
    INT32U registerValue = m_pMac->ReadRegister(HST_GPIO_CFG);

    // Only set the bits in the register value with correspond to 1 bits
    // in the mask (i.e., only the pins which should have configuration
    // changed)
    registerValue &= ~mask;
    registerValue |= (mask & configuration);

    m_pMac->WriteRegister(HST_GPIO_CFG, registerValue);

    // Send the config GPIO command to the MAC
    m_pMac->WriteRegister(HST_CMD, CMD_CFGGPIO);

    // Process the MAC's command-response packets
    PACKET_CALLBACK_DATA callbackData;
    callbackData.pRadio  = this;
    callbackData.command = CMD_CFGGPIO;
    callbackData.state   = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;

    this->ProcessOperationData(RFID_INVALID_RADIO_HANDLE,
                               Radio::PacketCallbackFunction,
                               &callbackData,
                               NULL,
                               false);

    // If the command failed, clear the MAC error and return an error
    if (callbackData.status)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Setting GPIO configuration failed with status 0x%.8x\n",
            __FUNCTION__,
            callbackData.status);
        //this->ClearMacError();
        throw RfidErrorException(RFID_ERROR_RADIO_FAILURE, __FUNCTION__);
    }
} // Radio::SetGpioPinsConfiguration

////////////////////////////////////////////////////////////////////////////////
// Name:        GetGpioPinsConfiguration
// Description: Retrieves the GPIO pin configuration for the radio
////////////////////////////////////////////////////////////////////////////////
INT32U Radio::GetGpioPinsConfiguration()
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Simply return the result from reading the GPIO config register
    return m_pMac->ReadRegister(HST_GPIO_CFG);
} // Radio::GetGpioPinsConfiguration

////////////////////////////////////////////////////////////////////////////////
// Name:        ReadGpioPins
// Description: Retrieves the state for the requested GPIO pins
////////////////////////////////////////////////////////////////////////////////
INT32U Radio::ReadGpioPins(
    INT32U  mask
    )
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Tell the MAC which GPIO pins to read
    m_pMac->WriteRegister(HST_GPIO_INMSK,
                          HST_GPIO_INMSK_MASK(mask) |
                          HST_GPIO_INMSK_RFU1(0));

    // Send the read GPIO command to the MAC
    m_pMac->WriteRegister(HST_CMD, CMD_RDGPIO);

    // Process the MAC's command-response packets
    PACKET_CALLBACK_DATA callbackData;
    callbackData.pRadio  = this;
    callbackData.command = CMD_RDGPIO;
    callbackData.state   = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;

    this->ProcessOperationData(RFID_INVALID_RADIO_HANDLE,
                               Radio::PacketCallbackFunction,
                               &callbackData,
                               NULL,
                               false);

    // If the command failed, clear the MAC error and return an error
    if (callbackData.status)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: GPIO read failed with status 0x%.8x\n",
            __FUNCTION__,
            callbackData.status);

        //this->ClearMacError();
        throw RfidErrorException(
            IO_INVAL_RDMASK == callbackData.status ?
                RFID_ERROR_INVALID_PARAMETER : 
                RFID_ERROR_RADIO_FAILURE, __FUNCTION__);
    }

    // Return the valus read
    return CPL_MacToHost32(callbackData.commandData.gpioRead.value);
} // Radio::ReadGpioPins

////////////////////////////////////////////////////////////////////////////////
// Name:        WriteGpioPins
// Description: Sets the state for the requested GPIO pins
// Parameters:  mask - specifies which pins (bit 0 = pin 0, etc.) are to be
//                written.  The presence of a 1 bit indicates that a pin is to
//                be read.  A 1-bit in a bit corresponding to an input pin
//                results in 
//                RfidErrorException(RFID_ERROR_INVALID_PARAMETER) being
//                thrown.
// Returns:     Nothing
////////////////////////////////////////////////////////////////////////////////
void Radio::WriteGpioPins(
    INT32U  mask,
    INT32U  value
    )
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Tell the MAC which GPIO pins to write as well as what values to write
    // to them
    m_pMac->WriteRegister(HST_GPIO_OUTMSK,
                          HST_GPIO_OUTMSK_MASK(mask)    |
                          HST_GPIO_OUTMSK_RFU1(0));
    m_pMac->WriteRegister(HST_GPIO_OUTVAL,
                          HST_GPIO_OUTVAL_VALUE(value)  |
                          HST_GPIO_OUTVAL_RFU1(0));

    // Send the write GPIO command to the MAC
    m_pMac->WriteRegister(HST_CMD, CMD_WRGPIO);

    // Process the MAC's command-response packets
    PACKET_CALLBACK_DATA callbackData;
    callbackData.pRadio  = this;
    callbackData.command = CMD_WRGPIO;
    callbackData.state   = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;

    this->ProcessOperationData(RFID_INVALID_RADIO_HANDLE,
                               Radio::PacketCallbackFunction,
                               &callbackData,
                               NULL,
                               false);

    // If the command failed, clear the MAC error and return an error
    if (callbackData.status)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: GPIO write failed with status 0x%.8x\n",
            __FUNCTION__,
            callbackData.status);

        //this->ClearMacError();
        throw RfidErrorException(
            IO_INVAL_WRMASK == callbackData.status ?
                RFID_ERROR_INVALID_PARAMETER : 
                RFID_ERROR_RADIO_FAILURE, __FUNCTION__);
    }
} // Radio::WriteGpioPins

////////////////////////////////////////////////////////////////////////////////
// Name:        WriteRadioRegister
// Description: Allows for writing directly to the radio's registers (i.e.,
//              bypassing the MAC.
////////////////////////////////////////////////////////////////////////////////
void Radio::WriteRadioRegister(
    INT16U  address,
    INT16U  value
    )
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Tell the MAC which register we want to write, what value to write to it,
    // and then send the command
    m_pMac->WriteRegister(HST_MBP_ADDR,
                          HST_MBP_ADDR_REGADDR(address) |
                          HST_MBP_ADDR_RFU1(0));
    m_pMac->WriteRegister(HST_MBP_DATA,
                          HST_MBP_DATA_REGDATA(value)   |
                          HST_MBP_DATA_RFU1(0));
    m_pMac->WriteRegister(HST_CMD, CMD_MBPWRREG);

    // Process the MAC's command-response packets
    PACKET_CALLBACK_DATA callbackData;
    callbackData.pRadio  = this;
    callbackData.command = CMD_MBPWRREG;
    callbackData.state   = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;

    this->ProcessOperationData(RFID_INVALID_RADIO_HANDLE,
                               Radio::PacketCallbackFunction,
                               &callbackData,
                               NULL,
                               false);

    // If the command failed, clear the MAC error and return an error
    if (callbackData.status)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Radio bypass register write failed with status 0x%.8x\n",
            __FUNCTION__,
            callbackData.status);

        this->ClearMacError();
        throw RfidErrorException(
            CSM_ERR_MBPWRADDR == callbackData.status ?
                RFID_ERROR_INVALID_PARAMETER :
                RFID_ERROR_RADIO_FAILURE, __FUNCTION__);
    }
} // Radio::WriteRadioRegister

////////////////////////////////////////////////////////////////////////////////
// Name:        ReadRadioRegister
// Description: Allows for reading directly from the radio's registers (i.e.,
//              bypassing the MAC.
////////////////////////////////////////////////////////////////////////////////
INT16U Radio::ReadRadioRegister(
    INT16U  address
    )
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Tell the MAC which register we want to read and send the command to read
    // it
    m_pMac->WriteRegister(HST_MBP_ADDR,
                          HST_MBP_ADDR_REGADDR(address) |
                          HST_MBP_ADDR_RFU1(0));
    m_pMac->WriteRegister(HST_CMD, CMD_MBPRDREG);

    // Process the MAC's command-response packets
    PACKET_CALLBACK_DATA callbackData;
    callbackData.pRadio  = this;
    callbackData.command = CMD_MBPRDREG;
    callbackData.state   = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;

    this->ProcessOperationData(RFID_INVALID_RADIO_HANDLE,
                               Radio::PacketCallbackFunction,
                               &callbackData,
                               NULL,
                               false);

    // If the command failed, clear the MAC error and return an error
    if (callbackData.status)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Radio bypass register write failed with status 0x%.8x\n",
            __FUNCTION__,
            callbackData.status);

        this->ClearMacError();
        throw RfidErrorException(
            CSM_ERR_MBPRDADDR == callbackData.status ?
                RFID_ERROR_INVALID_PARAMETER :
                RFID_ERROR_RADIO_FAILURE, __FUNCTION__);
    }
    // Otherwise, make a sanity check to ensure that the address of the data
    // returned matches the address of the data requested
    else if (callbackData.commandData.macByPassRead.address != address)
    {
        throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
    }

    return callbackData.commandData.macByPassRead.value;
} // Radio::ReadRadioRegister


////////////////////////////////////////////////////////////////////////////////
// Name:        ToggleCarrierWave
// Description: Turns the radio's carrier wave on or off
////////////////////////////////////////////////////////////////////////////////
void Radio::ToggleCarrierWave(
    bool toggleOn
    )
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Send the carrier wave on or off command (based upon the toggle value)
    INT32U command = (toggleOn ? CMD_CWON : CMD_CWOFF);
    m_pMac->WriteRegister(HST_CMD, command);

    // Process the MAC's command-response packets
    PACKET_CALLBACK_DATA callbackData;
    callbackData.pRadio  = this;
    callbackData.command = command;
    callbackData.state   = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;

    this->ProcessOperationData(RFID_INVALID_RADIO_HANDLE,
                               Radio::PacketCallbackFunction,
                               &callbackData,
                               NULL,
                               false);

    // If the command failed, clear the MAC error and return an error
    if (callbackData.status)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Toggle carrier wave failed with status 0x%.8x\n",
            __FUNCTION__,
            callbackData.status);
        //this->ClearMacError();
        throw RfidErrorException(RFID_ERROR_RADIO_FAILURE, __FUNCTION__);
    }
} // Radio::ToggleCarrierWave

////////////////////////////////////////////////////////////////////////////////
// Name:        IssueCommand
// Description: Issues the command to the radio object
////////////////////////////////////////////////////////////////////////////////
void Radio::IssueCommand(
    INT32U  command)
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Simply write the MAC's command register
    m_pMac->WriteRegister(HST_CMD, command);

    // Do any post-command issue work
    this->PostMacCommandIssue();
} // Radio::IssueCommand


////////////////////////////////////////////////////////////////////////////////
// Name:        RandomCarrierWave
// Description: Sets up  the CW duration register and issues the random 
//              CW command to the radio object
////////////////////////////////////////////////////////////////////////////////
void Radio::RandomCarrierWave(
    INT32U  duration)
{
    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Setup the duration
    m_pMac->WriteRegister(HST_TX_RANDOM_DATA_DURATION, duration);

    // Simply write the MAC's command register with the random CW command
    m_pMac->WriteRegister(HST_CMD, CMD_TX_RANDOM_DATA);

    // Do any post-command issue work
    this->PostMacCommandIssue();
} // Radio::RandomCarrierWave


////////////////////////////////////////////////////////////////////////////
// Name:        SetImpinjExtensions
// Description: Sets the Impinj Extensions parameters
// Parameters:  pExtensions - a pointer to the Impinj Extensions parameters
// Returns:     Nothing
////////////////////////////////////////////////////////////////////////////
void Radio::SetImpinjExtensions(
    const RFID_IMPINJ_EXTENSIONS*     pExtensions
    )
{

    assert(NULL != pExtensions);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Write the impinj extension values to the register
    m_pMac->WriteRegister(HST_IMPINJ_EXTENSIONS,
        HST_IMPINJ_EXTENSIONS_BLOCKWRITE_MODE(pExtensions->blockWriteMode) |
        HST_IMPINJ_EXTENSIONS_TAG_FOCUS(pExtensions->tagFocus) |
        HST_IMPINJ_EXTENSIONS_FAST_ID(pExtensions->fastId)   |
        HST_IMPINJ_EXTENSIONS_RFU1(0));

} // Radio::SetImpinjExtensions

////////////////////////////////////////////////////////////////////////////
// Name:        GetImpinjExtensions
// Description: Retrieves the query tag group parameters
// Parameters:  pGroup - a pointer to structure that will receive the query
//              tag group parameters
// Returns:     Nothing
////////////////////////////////////////////////////////////////////////////
void Radio::GetImpinjExtensions(
    RFID_IMPINJ_EXTENSIONS*     pExtensions
    )
{
    assert(NULL != pExtensions);

    // If the radio is busy, don't allow this operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }


    // Retrieve the Impinj Extensions  register and set the fields
    // in the structure appropriately
    INT32U registerValue        = m_pMac->ReadRegister(HST_IMPINJ_EXTENSIONS);
    pExtensions->blockWriteMode = HST_IMPINJ_EXTENSIONS_GET_BLOCKWRITE_MODE(registerValue);
    pExtensions->tagFocus = HST_IMPINJ_EXTENSIONS_GET_TAG_FOCUS(registerValue);
    pExtensions->fastId  = HST_IMPINJ_EXTENSIONS_GET_FAST_ID(registerValue);

    if (pExtensions->blockWriteMode > BLOCKWRITE_MODE_FORCE_TWO_WORD)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Invalid block write mode found (%d)\n",
            __FUNCTION__,pExtensions->blockWriteMode);
        throw RfidErrorException(RFID_ERROR_UNEXPECTED_VALUE, __FUNCTION__);
    }
} // Radio::GetImpinjExtensions

////////////////////////////////////////////////////////////////////////////////
// Name:        Start18K6CRequest
// Description: Performs the generic configuration setting needed for
//              supporting the ISO 18000-6C tag operations.
////////////////////////////////////////////////////////////////////////////////
void Radio::Start18K6CRequest(
    const RFID_18K6C_COMMON_PARMS* pParms,
    INT32U                         flags
    )
{
    assert(NULL != pParms);

    // If the radio is already busy, then don't allow another operation
    if (m_isBusy)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Cannot complete request as radio is busy\n",
            __FUNCTION__);
        throw RfidErrorException(RFID_ERROR_RADIO_BUSY, __FUNCTION__);
    }

    // Set up the rest of the HST_INV_CFG register.  First, we have to read its
    // current value
    INT32U registerValue = m_pMac->ReadRegister(HST_INV_CFG);

    // TBD - an optimization could be to only write back the register if
    // the value changes

    // Set the tag stop count and enabled flags and then write the register
    // back
    if (flags & RFID_FLAG_PERFORM_SELECT)
    {
        HST_INV_CFG_SET_AUTOSEL_ENABLED(registerValue);
    }
    else
    {
        HST_INV_CFG_SET_AUTOSEL_DISABLED(registerValue);
    }
    HST_INV_CFG_SET_REP(registerValue, pParms->tagStopCount);
    m_pMac->WriteRegister(HST_INV_CFG, registerValue);

    // Set the enabled flag in the HST_INV_EPC_MATCH_CFG register properly.  To
    // do so, have to read the register value first.  Then set the bit properly
    // and then write the register value back to the MAC.
    registerValue = m_pMac->ReadRegister(HST_INV_EPC_MATCH_CFG);
    if (flags & RFID_FLAG_PERFORM_POST_MATCH)
    {
        HST_INV_EPC_MATCH_CFG_SET_ENABLED(registerValue);
    }
    else
    {
        HST_INV_EPC_MATCH_CFG_SET_DISABLED(registerValue);
    }
    m_pMac->WriteRegister(HST_INV_EPC_MATCH_CFG, registerValue);
} // Radio::Start18K6CRequest

////////////////////////////////////////////////////////////////////////////
// Name:        PostMacCommandIssue
// Description: Performs any post-request work needed for the ISO 18000-6C
//              tag-protocol operation.
// Parameters:  None
// Returns:     Nothing
////////////////////////////////////////////////////////////////////////////
void Radio::PostMacCommandIssue()
{
    // Grab the cancel/abort lock
    CplMutexAutoLock    lockIt(&m_cancelAbortLock);

    // Now that everything is okay, indicate that the radio is busy
    m_isBusy                = true;
    m_shouldCancel          = false;
    m_shouldAbort           = false;
    m_operationCancelled    = false;

    // Save off the thread ID of this thread so that we can avoid deadlock
    // issues associated with certain functions (specifically, those involving
    // a radio cancel) being called from the packet callback
    m_busyThread = CPL_ThreadGetID();

    // Now that we are ready to finish everything, let the auto-lock relinquish
    // the lock.  We'll re-wrap the lock when we start processing packets.
    lockIt.Transfer();
} // Radio::Finish18K6CRequest

////////////////////////////////////////////////////////////////////////////////
// Name:        Radio::WriteMacMaskRegisters
// Description: Writes the MAC mask registers (select or post-singulation).
////////////////////////////////////////////////////////////////////////////////
void Radio::WriteMacMaskRegisters(
    INT16U          registerAddress,
    INT32U          bitCount,
    const INT8U*    pMask
    )
{
    // Figure out how many bytes are in the mask
    INT32U byteCount = (bitCount + BITS_PER_BYTE - 1) / BITS_PER_BYTE;

    // Now write each MAC mask register
    while (byteCount)
    {
        INT32U registerValue = 0;
        INT32U leftShift     = 0;
        INT32U loopCount     = (byteCount > BYTES_PER_REGISTER ?
                                    BYTES_PER_REGISTER : byteCount);

        // Decrement the byte count by the number of bytes put into the register
        byteCount -= loopCount;

        // Build up the register value
        for ( ; loopCount; --loopCount, leftShift += BITS_PER_BYTE, ++pMask)
        {
            registerValue |= (static_cast<INT32U>(*pMask) << leftShift);
        }

        // If it is the last byte of the mask, then we are going to zero out
        // any bits not in the mask
        if (!byteCount && (bitCount % BITS_PER_BYTE))
        {
            INT32U mask = 0xFFFFFFFF;
            mask          <<= (BITS_PER_REGISTER - (BITS_PER_BYTE - (bitCount % BITS_PER_BYTE)));
            mask          >>= (BITS_PER_REGISTER - (leftShift - (bitCount % BITS_PER_BYTE)));
            registerValue &=  ~mask;
        }

        // Write the register
        m_pMac->WriteRegister(registerAddress++, registerValue); // Generic Write
    }
} // Radio::WriteMacMaskRegisters

////////////////////////////////////////////////////////////////////////////
// Name:        ReadMacMaskRegisters
// Description: Reads the MAC mask registers (select or post-singulation).
////////////////////////////////////////////////////////////////////////////
void Radio::ReadMacMaskRegisters(
    INT16U  registerAddress,
    INT32U  bitCount,
    INT8U*  pMask
    )
{
    // Figure out how many bytes are in the mask
    INT32U byteCount = (bitCount + BITS_PER_BYTE - 1) / BITS_PER_BYTE;

    // Read the mask registers and build the mask
    while (byteCount)
    {
        INT32U registerValue = m_pMac->ReadRegister(registerAddress++); // Generic Read
        INT32U rightShift     = 0;
        INT32U loopCount     = (byteCount > BYTES_PER_REGISTER ?
                                    BYTES_PER_REGISTER : byteCount);

        // Decrement the byte count by the number of bytes to extract from the
        // register
        byteCount -= loopCount;

        // Add the register bytes to the mask
        for ( ; loopCount; --loopCount, rightShift += BITS_PER_BYTE, ++pMask)
        {
            *pMask = (INT8U)((registerValue >> rightShift) & 0x000000FF);
        }
    }

    // If the last byte isn't complete, mask off the unneeded bits
    if (bitCount % BITS_PER_BYTE)
    {
        *pMask &= 0xFF << (BITS_PER_BYTE - (bitCount % BITS_PER_BYTE));
    }
} // Radio::ReadMacMaskRegisters

////////////////////////////////////////////////////////////////////////////
// Name:        RetrieveNextPacket
// Description: Retrieves the next packet from the MAC.
////////////////////////////////////////////////////////////////////////////
void Radio::RetrieveNextPacket(
    INT32U          &bufferSize,
    PACKET_BUFFER   &buffer,
    bool            canBeCancelled
    )
{
    // Start out by only retrieving the common packet header
    bufferSize = sizeof(hostpkt_cmn);
    if (buffer.size() < bufferSize)
    {
        buffer.resize(bufferSize);
    }

    // Retrieve the common packet header first
    this->RetrieveBuffer(bufferSize, &buffer[0], canBeCancelled);

    // Now that we have the common header, figure out how many bytes remain
    // in the packet
    INT32U  remainingSize =
        RFID_PACKET_PKT_BYTE_LEN(
            CPL_MacToHost16(
                reinterpret_cast<RFID_PACKET_COMMON *>(&buffer[0])->pkt_len));

    // Increase the buffer size if necessary
    if (buffer.size() < (bufferSize + remainingSize))
    {
        buffer.resize(bufferSize + remainingSize);
    }

    // If there is data beyond the common header, retrieve it as
    // well
    if (remainingSize)
    {
        this->RetrieveBuffer(
            remainingSize,
            &buffer[bufferSize],
            canBeCancelled);
    }

    // Set the total buffer size appropriately
    bufferSize += remainingSize;
} // Radio::RetrieveNextPacket

////////////////////////////////////////////////////////////////////////////
// Name:        RetrieveBuffer
// Description: Retrieves a buffer of the specified size from the MAC.
////////////////////////////////////////////////////////////////////////////
void Radio::RetrieveBuffer(
    INT32U  bufferSize,
    INT8U*  buffer,
    bool    canBeCancelled
    )
{
    CPL_TimeSpec startTime;
    CPL_TimeSpec currentTime;
    INT32U       last_bytesAvailable;

    // Keep checking until there are enough bytes to fulfill the request.  If
    // we already know that there are enough bytes available, we aren't even
    // going to poll the transport layer.  
    // 
    // Add a timeout so we don't get stuck here. This is unlikely to ever happen
    // with USB transport, but with serial we may since there is no way to know 
    // that the radio may be disconnected.  Only timeout if  we go for 3 seconds 
    // and m_bytesAvailable is not changing.  So long as we are receiving bytes,
    // then the 3 second timeout window should be reset.
    //
    // Timeout logic was added as of v2.4.0 and depends on the MAC Firmware providing
    // the Command Active packet during long running command operations when there 
    // is no other packet traffic.  For backward compatibility, we only enable the
    // timeout checks if we are running v2.4.0 firmware or later.
    // 
    // snapshot current value of bytesAvailable and capture the startTime
    last_bytesAvailable = m_bytesAvailable;
    CPL_TimeSpecGet(&startTime);

    while (m_bytesAvailable < bufferSize)
    {   

        // Figure out how many bytes are available to consume
        m_bytesAvailable = this->RetrieveRawBytes(0, NULL, canBeCancelled);

        // if we are running against 2.4.0 firmware or later
        if (m_preTwoFourFirmware == 0)
        {
            if (m_bytesAvailable != last_bytesAvailable) 
            {
                // value was updated, so reset startTime and refresh snapshot
                last_bytesAvailable = m_bytesAvailable;
                CPL_TimeSpecGet(&startTime);
            }
            else
            {
                // value remains unchanged, check for timeout
                CPL_TimeSpecGet(&currentTime);
                CPL_TimeSpecDiff(&currentTime,&startTime);
                if (currentTime.seconds > RFID_CMD_RESPONSE_TIMEOUT)
                {
                    throw RfidErrorException(RFID_ERROR_RADIO_NOT_RESPONDING, __FUNCTION__);
                }
            }
        }
        if (m_bytesAvailable < bufferSize)
        {
            CPL_MillisecondSleep(MAC_SLEEP_MILLIS);
        }
    }

    // At this point, we know that there are enough bytes to fulfill the
    // request
    m_bytesAvailable = this->RetrieveRawBytes(bufferSize, buffer, canBeCancelled);

} // Radio::RetrieveBuffer

////////////////////////////////////////////////////////////////////////////
// Name:        RetrieveRawBytes
// Description: Retrieves raw bytes from the MAC.  Also will indicate how
//              many bytes are remaining for retrieval without blocking.
////////////////////////////////////////////////////////////////////////////
INT32U Radio::RetrieveRawBytes(
    INT32U  bufferSize,
    INT8U*  buffer,
    bool    canBeCancelled
    )
{
    assert(!bufferSize || (NULL != buffer));

    // If allowed to do so, detect to see if a cancel or an abort should
    // be issued to the MAC
    if (canBeCancelled)
    {
        if (m_shouldCancel)
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Radio operation has been cancelled\n",
                __FUNCTION__);

            // For a cancel, since we are to receive a command-end, issue
            // the cancel and then we'll continue on our merry way
            m_pMac->CancelOperation();
            m_shouldCancel       = false;
            m_operationCancelled = true;
        }
        else if (m_shouldAbort)
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Radio operation has been aborted\n",
                __FUNCTION__);

            // Since we are clearing out the pipeline, we know that there
            // will not be any bytes immediately available for consumption
            m_bytesAvailable = 0;

            // For an abort, once that is done, we want to blast out of the
            // packet receive loop
            m_pMac->AbortOperation();
            throw RfidErrorException(RFID_ERROR_OPERATION_CANCELLED, __FUNCTION__);
        }
    }

    // Now receive the bytes from the MAC
    return m_pMac->RetrieveData(buffer, bufferSize);
} // Radio::RetrieveRawBytes


////////////////////////////////////////////////////////////////////////////
// Name:        FastCrc32
// Description: Calculates the CRC for the NV Update packets
////////////////////////////////////////////////////////////////////////////
INT32U Radio::FastCrc32(INT32U sum, unsigned char *p, INT32U len)
{
    while (len--)
      sum = crc32_table[(sum >> 24) ^ *p++] ^ (sum << 8);
    return sum;
}

////////////////////////////////////////////////////////////////////////////
// Name:        PacketCallbackFunction
// Description: The callback that is invoked when a command-response packet
//              is received.  This is only for processing packets that the
//              radio object intercepts on the application's behalf.
////////////////////////////////////////////////////////////////////////////
INT32S RFID_CALLBACK Radio::PacketCallbackFunction(
    RFID_RADIO_HANDLE   handle,
    INT32U              bufferLength,
    const INT8U*        pBuffer,
    void*               context
    )
{
    PACKET_CALLBACK_DATA* pData  = static_cast<PACKET_CALLBACK_DATA *>(context);

    RFID_UNREFERENCED_LOCAL(handle);

    // Process the packets
    INT32U byteLength;
    for ( ; bufferLength; bufferLength -= byteLength)
    {
        const RFID_PACKET_COMMON* pPacket =
            reinterpret_cast<const RFID_PACKET_COMMON *>(pBuffer);

        // Calculate the total byte length of the packet
        byteLength = sizeof(RFID_PACKET_COMMON) +
                     (RFID_PACKET_BYTES_PER_UNIT *
                      CPL_MacToHost16(pPacket->pkt_len));

        assert(bufferLength >= byteLength);

        // Process the packet
        pData->pRadio->ProcessMacPacket(pData, pPacket);
    }

    return 0;
} // Radio::PacketCallbackFunction

////////////////////////////////////////////////////////////////////////////////
// Name:        ProcessMacPacket
// Description: Used to process a MAC command-response packet.  This, unlike
//              the packet callback is a member function.
////////////////////////////////////////////////////////////////////////////////
void Radio::ProcessMacPacket(
    PACKET_CALLBACK_DATA*       pData,
    const RFID_PACKET_COMMON*   pPacket
    )
{
    INT16U packetType = CPL_MacToHost16(pPacket->pkt_type);
    // Based upon the packet type, process accordingly
    switch (packetType)
    {
        case RFID_PACKET_TYPE_COMMAND_BEGIN:
        {
            // Verify that the radio is in a state that it is expecting a begin
            // packet
            if (RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN != pData->state)
            {
                g_pTracer->PrintMessage(
                    Tracer::RFID_LOG_SEVERITY_ERROR,
                    "%s: Received command begin packet, but in state %u\n",
                    __FUNCTION__,
                    pData->state);
                throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            }

            // Process the command-begin packet
            this->ProcessCommandBeginPacket(
                pData,
                reinterpret_cast<const RFID_PACKET_COMMAND_BEGIN *>(pPacket));
            break;
        } // case RFID_PACKET_TYPE_COMMAND_BEGIN
        case RFID_PACKET_TYPE_COMMAND_END:
        {
            // Process the command-end packet
            this->ProcessCommandEndPacket(
                pData,
                reinterpret_cast<const RFID_PACKET_COMMAND_END *>(pPacket));
            break;
        } // case RFID_PACKET_TYPE_COMMAND_END
        case RFID_PACKET_TYPE_OEMCFG_READ:
        {
            // Verify that the radio is in a state that it is expecting an OEM
            // configuration read
            if (RFID_RADIO_STATE_EXPECT_OEMCFG_READ != pData->state)
            {
                g_pTracer->PrintMessage(
                    Tracer::RFID_LOG_SEVERITY_ERROR,
                    "%s: Received OEM read packet, but in state %u\n",
                    __FUNCTION__,
                    pData->state);
                throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            }

            // Process the OEM configuration read packet
            this->ProcessOemcfgReadPacket(
                pData,
                reinterpret_cast<const RFID_PACKET_OEMCFG_READ *>(pPacket));
            break;
        } // case RFID_PACKET_TYPE_OEMCFG_READ
        case RFID_PACKET_TYPE_MBP_READ_REG:
        {
            // Verify that the radio is in a state that it is expecting a MAC
            // bypass read register
            if (RFID_RADIO_STATE_EXPECT_MBP_READ_REG != pData->state)
            {
                g_pTracer->PrintMessage(
                    Tracer::RFID_LOG_SEVERITY_ERROR,
                    "%s: Received radio bypass register read packet, but in "
                    "state %u\n",
                    __FUNCTION__,
                    pData->state);
                throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            }

            // Process the MAC bypass read register packet
            this->ProcessMbpReadRegPacket(
                pData,
                reinterpret_cast<const RFID_PACKET_MBP_READ *>(pPacket));
            break;
        } // case RFID_PACKET_TYPE_MBP_READ_REG
        case RFID_PACKET_TYPE_GPIO_READ:
        {
            // Verify that the radio is in a state that it is expecting a GPIO
            // read packet
            if (RFID_RADIO_STATE_EXPECT_GPIO_READ != pData->state)
            {
                g_pTracer->PrintMessage(
                    Tracer::RFID_LOG_SEVERITY_ERROR,
                    "%s: Received GPIO read packet, but in state %u\n",
                    __FUNCTION__,
                    pData->state);
                throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            }

            // Process the GPIO read packet
            this->ProcessGpioReadPacket(
                pData,
                reinterpret_cast<const RFID_PACKET_GPIO_READ *>(pPacket));
            break;
        } // case RFID_PACKET_TYPE_GPIO_READ
        case RFID_PACKET_TYPE_NVMEMUPDCFG:
        {
            // Verify that the radio is in a state that it is expecting an
            // nonvolatile memory update configuration packet
            if (RFID_RADIO_STATE_EXPECT_NVMEMUPDCFG_READ != pData->state)
            {
                g_pTracer->PrintMessage(
                    Tracer::RFID_LOG_SEVERITY_ERROR,
                    "%s: Received NVMEM update packet, but in state %u\n",
                    __FUNCTION__,
                    pData->state);
                throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            }

            // Process the nonvolatile memory upate configuration packet
            this->ProcessNonvolatileMemoryUpdateConfigPacket(
                pData,
                reinterpret_cast<const RFID_PACKET_NVMEMUPDCFG *>(pPacket));
            break;
        } // case RFID_PACKET_TYPE_NVMEMUPDCFG
        case RFID_PACKET_TYPE_LPROF_READ_REG:
        {
            // Verify that the radio is in a state that it is expecting an
            // link profile register read packet
            if (RFID_RADIO_STATE_EXPECT_LINKPROFILE_READ_REG != pData->state)
            {
                g_pTracer->PrintMessage(
                    Tracer::RFID_LOG_SEVERITY_ERROR,
                    "%s: Received link profile register read packet, but in "
                    "state %u\n",
                    __FUNCTION__,
                    pData->state);
                throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            }

            // Process the link profile register read packet
            this->ProcessLinkProfileRegisterReadPacket(
                pData,
                reinterpret_cast<const RFID_PACKET_LPROF_READ *>(pPacket));
            break;
        } // case RFID_PACKET_TYPE_LPROF_READ_REG
        case RFID_PACKET_TYPE_DEBUG:
        {
            // allow debug packets in all states
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_INFO,
                "%s: Received debug packet while in "
                "state %u\n",
                __FUNCTION__,
                pData->state);
            break;
        }
        default:
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Received unexpected packet 0x%.4x\n",
                __FUNCTION__,
                packetType);
            throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            break;
        } // default
    } // switch (packetType)
} // Radio::ProcessMacPacket

////////////////////////////////////////////////////////////////////////////////
// Name:        ProcessCommandBeginPacket
// Description: Used to process a command-begin packet.
////////////////////////////////////////////////////////////////////////////////
void Radio::ProcessCommandBeginPacket(
    PACKET_CALLBACK_DATA*               pData,
    const RFID_PACKET_COMMAND_BEGIN*    pPacket
    )
{
    INT32U command = CPL_MacToHost32(pPacket->command);

    // Ensure that the reponse is for the command we issued
    if (command != pData->command)
    {
        g_pTracer->PrintMessage(
            Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Received response for command 0x%.8x, but expected 0x%.8x\n",
            __FUNCTION__,
            command,
            pData->command);
        throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
    }

    // Based upon the command, switch to the appropriate state
    switch (command)
    {
        case CMD_RDOEM:
        {
            // We should see an OEM config read packet next
            pData->state = RFID_RADIO_STATE_EXPECT_OEMCFG_READ;
            break;
        } // case CMD_RDOEM
        case CMD_MBPRDREG:
        {
            // We should see a MAC bypass read register packet next
            pData->state = RFID_RADIO_STATE_EXPECT_MBP_READ_REG;
            break;
        } // case CMD_MBPRDREG
        case CMD_RDGPIO:
        {
            // We should see a GPIO read packet next
            pData->state = RFID_RADIO_STATE_EXPECT_GPIO_READ;
            break;
        } // case CMD_RDGPIO
        case CMD_NV_MEM_UPDATE:
        {
            // We should see a nonvolatile memory update config packet next
            pData->state = RFID_RADIO_STATE_EXPECT_NVMEMUPDCFG_READ;
            break;
        } // case CMD_NV_MEM_UPDATE
        case CMD_LPROF_RDXCVRREG:
        {
            // We should see a profile link register read packet next
            pData->state = RFID_RADIO_STATE_EXPECT_LINKPROFILE_READ_REG;
            break;
        } // case CMD_LPROF_RDXCVRREG
        case CMD_WROEM:
        case CMD_MBPWRREG:
        case CMD_WRGPIO:
        case CMD_CFGGPIO:
        case CMD_SETPWRMGMTCFG:
        case CMD_CLRERR:
        case CMD_CWON:
        case CMD_CWOFF:
        case CMD_UPDATELINKPROFILE:
        case CMD_LPROF_WRXCVRREG:
        {
            // We should see a command-end packet next
            pData->state = RFID_RADIO_STATE_EXPECT_COMMAND_END;
            break;
        } // case CMD_WROEM, etc.
        default:
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Received unexpected command response 0x%.8x\n",
                __FUNCTION__,
                command);
            throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            break;
        } // default
    } // switch (command)
} // Radio::ProcessCommandBeginPacket

////////////////////////////////////////////////////////////////////////////////
// Name:        ProcessCommandEndPacket
// Description: Used to process a command-end packet.
////////////////////////////////////////////////////////////////////////////////
void Radio::ProcessCommandEndPacket(
    PACKET_CALLBACK_DATA*           pData,
    const RFID_PACKET_COMMAND_END*  pPacket
    )
{
    // Set the status with the result from the MAC
    pData->status = CPL_MacToHost32(pPacket->status);

    // Update the state to indicate we should see a begin packet next
    pData->state = RFID_RADIO_STATE_EXPECT_COMMAND_BEGIN;
} // Radio::ProcessCommandEndPacket

////////////////////////////////////////////////////////////////////////////////
// Name:        ProcessOemcfgReadPacket
// Description: Used to process a OEM configuration read packet.
////////////////////////////////////////////////////////////////////////////////
void Radio::ProcessOemcfgReadPacket(
    PACKET_CALLBACK_DATA*           pData,
    const RFID_PACKET_OEMCFG_READ*  pPacket
    )
{
    // Ensure that the reponse is for the command we issued and that the state
    // of the radio is that we are expecting the OEM configuration read packet
    if ((CMD_RDOEM                           != pData->command) ||
        (RFID_RADIO_STATE_EXPECT_OEMCFG_READ != pData->state))
    {
        if (CMD_RDOEM != pData->command)
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Received OEM read packet, but didn't issue an OEM read "
                "request\n",
                __FUNCTION__);
        }
        else
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Received OEM read packet, but in state %u\n",
                __FUNCTION__,
                pData->state);
        }

        throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
    }

    // Copy the OEM address and data from the packet.  
    // NOTE: We return the data in MAC format (no conversion).  This
    //       allows data to be read out and stored to a file, then reloaded
    //       through the RFID_MacWriteOemData API. 
    pData->commandData.oemRead.address = CPL_MacToHost32(pPacket->addr);
    pData->commandData.oemRead.value   = pPacket->data;

    // Update the state to indicate we should see an end packet next
    pData->state = RFID_RADIO_STATE_EXPECT_COMMAND_END;
} // Radio::ProcessOemcfgReadPacket

////////////////////////////////////////////////////////////////////////////////
// Name:        ProcessMbpReadRegPacket
// Description: Used to process a MAC bypass register read packet.
////////////////////////////////////////////////////////////////////////////////
void Radio::ProcessMbpReadRegPacket(
    PACKET_CALLBACK_DATA*       pData,
    const RFID_PACKET_MBP_READ* pPacket
    )
{
    // Ensure that the reponse is for the command we issued and that the state
    // of the radio is that we are expecting the MAC bypass register read packet
    if ((CMD_MBPRDREG                         != pData->command) ||
        (RFID_RADIO_STATE_EXPECT_MBP_READ_REG != pData->state))
    {
        if (CMD_MBPRDREG != pData->command)
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Received radio bypass register read packet, but didn't "
                "issue a radio bypass register read request\n",
                __FUNCTION__);
        }
        else
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Received radio bypass register read packet, but in state "
                "%u\n",
                __FUNCTION__,
                pData->state);
        }

        throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
    }

    // Copy the register address and value from the packet
    pData->commandData.macByPassRead.address = CPL_MacToHost16(pPacket->addr);
    pData->commandData.macByPassRead.value   = CPL_MacToHost16(pPacket->data);

    // Update the state to indicate we should see an end packet next
    pData->state = RFID_RADIO_STATE_EXPECT_COMMAND_END;
} // Radio::ProcessMbpReadRegPacket

////////////////////////////////////////////////////////////////////////////////
// Name:        ProcessGpioReadPacket
// Description: Used to process a GPIO read packet.
////////////////////////////////////////////////////////////////////////////////
void Radio::ProcessGpioReadPacket(
    PACKET_CALLBACK_DATA*           pData,
    const RFID_PACKET_GPIO_READ*    pPacket
    )
{
    // Ensure that the reponse is for the command we issued and that the state
    // of the radio is that we are expecting the GPIO read packet
    if ((CMD_RDGPIO                        != pData->command) ||
        (RFID_RADIO_STATE_EXPECT_GPIO_READ != pData->state))
    {
        if (CMD_RDGPIO != pData->command)
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Received GPIO read packet, but didn't issue a GPIO read "
                "request\n",
                __FUNCTION__);
        }
        else
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Received GPIO read packet, but in state %u\n",
                __FUNCTION__,
                pData->state);
        }

        throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
    }

    // Copy the GPIO values that were read from the packet
    pData->commandData.gpioRead.value = CPL_MacToHost32(pPacket->data);

    // Update the state to indicate we should see an end packet next
    pData->state = RFID_RADIO_STATE_EXPECT_COMMAND_END;
} // Radio::ProcessGpioReadPacket

////////////////////////////////////////////////////////////////////////////
// Name:        ProcessNonvolatileMemoryUpdateConfigPacket
// Description: Used to process a nonvolatile memory update config packet.
////////////////////////////////////////////////////////////////////////////
void Radio::ProcessNonvolatileMemoryUpdateConfigPacket(
    PACKET_CALLBACK_DATA*           pData,
    const RFID_PACKET_NVMEMUPDCFG*  pPacket
    )
{
    // Ensure that the reponse is for the command we issued and that the state
    // of the radio is that we are expecting the GPIO read packet
    if ((CMD_NV_MEM_UPDATE                        != pData->command) ||
        (RFID_RADIO_STATE_EXPECT_NVMEMUPDCFG_READ != pData->state))
    {
        if (CMD_NV_MEM_UPDATE != pData->command)
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Received NVMEM update packet, but didn't issue a NVMEM "
                "update request\n",
                __FUNCTION__);
        }
        else
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Received NVMEM update packet, but in state %u\n",
                __FUNCTION__,
                pData->state);
        }

        throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
    }

    // Copy the maximum packet length from the response packet
    pData->commandData.nvMemUpdate.maxPacketSize = 
        CPL_MacToHost32(pPacket->max_pkt_len);

    // Update the state to indicate we should see an end packet next
    pData->state = RFID_RADIO_STATE_EXPECT_COMMAND_END;
} // Radio::ProcessNonvolatileMemoryUpdateConfigPacket

////////////////////////////////////////////////////////////////////////////
// Name:        ProcessLinkProfileRegisterReadPacket
// Description: Used to process a link profile register read packet.
////////////////////////////////////////////////////////////////////////////
void Radio::ProcessLinkProfileRegisterReadPacket(
    PACKET_CALLBACK_DATA*           pData,
    const RFID_PACKET_LPROF_READ*   pPacket
    )
{
    // Ensure that the reponse is for the command we issued and that the state
    // of the radio is that we are expecting the GPIO read packet
    if ((CMD_LPROF_RDXCVRREG                          != pData->command) ||
        (RFID_RADIO_STATE_EXPECT_LINKPROFILE_READ_REG != pData->state))
    {
        if (CMD_LPROF_RDXCVRREG != pData->command)
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Received link profile register read packet, but didn't "
                "issue a NVMEM update request\n",
                __FUNCTION__);
        }
        else
        {
            g_pTracer->PrintMessage(
                Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Received link profile register read packet, but in state "
                "%u\n",
                __FUNCTION__,
                pData->state);
        }

        throw RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
    }

    // Copy the values (profile, address, and data) from the packet
    pData->commandData.linkProfileRead.profile  = 
        CPL_MacToHost16(pPacket->sel);
    pData->commandData.linkProfileRead.address  = 
        CPL_MacToHost16(pPacket->addr);
    pData->commandData.linkProfileRead.data     = 
        CPL_MacToHost16(pPacket->data);

    // Update the state to indicate we should see an end packet next
    pData->state = RFID_RADIO_STATE_EXPECT_COMMAND_END;
} // Radio::ProcessLinkProfileRegisterReadPacket

} // namespace rfid
