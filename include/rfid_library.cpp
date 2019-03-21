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
 * $Id: rfid_library.cpp 67084 2011-03-24 21:50:15Z dshaheen $
 * 
 * Description:
 *     This is the main module of the RFID Library.
 *     
 *     Contents:
 *     Functions constituting the services offered by the RFID Library
 *     
 *
 *****************************************************************************
 */

#include <stddef.h>
#include <stdlib.h>
#include <memory>
#include <string>
#include <assert.h>
#include <utility>
#include "rfid_library.h"
#include "rfid_exceptions.h"
#include "compat_fildes.h"
#include "compat_mutex.h"
#include "compat_handles.h"
#include "compat_error.h"
#include "auto_handle_compat.h"
#include "auto_lock_compat.h"
#include "mac_transport_live.h"
#include "object_table.h"
#include "radio.h"
#include "rfid_extern.h"
#include "rfid_version.h"
#include "tracer_null.h"
#include "tracer_file.h"
#include "tracer_console.h"


#ifdef RFID_LIBRARY_EXTENSIONS

#include "rfid_library_ext.h"

#endif // RFID_LIBRARY_EXTENSIONS


////////////////////////////////////////////////////////////////////////////////
// Name: RadioWrapper
//
// Description: This class is used to wrap a radio object and an associated
//   helper objects as an atomic object.
////////////////////////////////////////////////////////////////////////////////
class RadioWrapper
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name:        RadioWrapper
    // Description: Initializes the radio wrapper object.
    // Parameters:  pRadio - an auto_ptr wrapped radio object.  This implies
    //                that the once the constructor returns that the wrapper
    //                is responsible for the radio object's destruction.
    //              pRadioLock - an auto_ptr wrapped lock object.  This implies
    //                that the once the constructor returns that the wrapper
    //                is responsible for the lock object's destruction.
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    RadioWrapper(
        std::auto_ptr<rfid::Radio>  pRadio,
        std::auto_ptr<CPL_Mutex>    pRadioLock
        ) :
        m_pRadio(pRadio),
        m_pRadioLock(pRadioLock)
    {
    } // RadioWrapper

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ~RadioWrapper
    // Description: Cleans up the radio wrapper object
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    ~RadioWrapper()
    {
        // Just need to ensure that we clean up the lock
        CPL_MutexDestroy(m_pRadioLock.get());
    } // ~RadioWrapper

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetRadioPointer
    // Description: Returns the raw radio pointer
    // Parameters:  None
    // Returns:     A raw radio pointer
    ////////////////////////////////////////////////////////////////////////////
    inline rfid::Radio* GetRadioPointer() const
    {
        return m_pRadio.get();
    } // GetRadioPointer

    ////////////////////////////////////////////////////////////////////////////
    // Name:        GetRadioLockHandle
    // Description: Returns the handle to the radio's lock
    // Parameters:  None
    // Returns:     The radio's lock handle
    ////////////////////////////////////////////////////////////////////////////
    inline rfid::CplMutexHandle GetRadioLockHandle() const
    {
        return m_pRadioLock.get();
    } // GetRadioLockHandle

private:
    const std::auto_ptr<rfid::Radio>    m_pRadio;
    const std::auto_ptr<CPL_Mutex>      m_pRadioLock;

    // Prevent copying of the wrapper
    RadioWrapper(const RadioWrapper&);
    const RadioWrapper& operator = (const RadioWrapper&);
};

// A typedef to simply make it easier to use the object table.  The active
// radio table stores pointers to the radio wrapper objects
typedef rfid::ObjectTable<RadioWrapper>     ActiveRadioTable;

// These typedefs are for the static functions that are used by the 
// live MAC transport.
typedef void (* RadioEnumerationFunction)(
    RFID_RADIO_ENUM*    pEnum
    );
typedef std::auto_ptr<rfid::Radio> (* RadioOpenFunction)(
    INT32U  transportHandle
    );

// Objects that we don't want to go outside the scope of this file
namespace
{
// A lock that is used to protect the library from being entered simultaneously
// by multiple threads.
std::auto_ptr<CPL_Mutex>        g_libraryLockHandle;
// A flag to indicate if the library has been successfully initialized
bool                            g_libraryIsInitialized      = false;
// A flag to indicate if the initialization is the very first one
bool                            g_firstInitialization       = true;
// A table that will be used for holding active radio objects
std::auto_ptr<ActiveRadioTable> g_pActiveRadios;

// Here are the MAC transport functions that are used for enumerating and
// opening radios.  These will be set appropriately for liver operation 
// 
RadioEnumerationFunction        g_radioEnumerationFunction  = NULL;
RadioOpenFunction               g_radioOpenFunction         = NULL;

// File name if we are logging to a file
const std::string               LOG_FILE_NAME("rfid_library.log");
} // namespace

// The tracer object for library logging
std::auto_ptr<rfid::Tracer>     g_pTracer;

// Embedded static strings that can be dumped
const char*                     COPYRIGHT   = VER_LEGALCOPYRIGHT_STR;
const char*                     VERSION     = VER_PRODUCTVERSION_STR;
const char*                     IDENT       = VER_IDENT_STRING;

// Constant flag bit for putting the library in log mode and the name of the 
// file 
enum {
    // Version information
    RFID_MAJOR_VERSION          = VER_MAJOR_NUM,
    RFID_MINOR_VERSION          = VER_MINOR_NUM,
    RFID_MAINTENANCE_VERSION    = VER_MAINTENANCE_NUM,
    RFID_RELEASE_VERSION        = VER_RELEASE_NUM,

    // Private bits for the library startup flag
    RFID_FLAG_LOG_FILE          = 0x80000000,   // Flag for logging to file
    RFID_FLAG_LOG_CONS          = 0x40000000,   // Flag for logging to console
    RFID_FLAG_LOG_LEVEL_MASK    = 0x0F000000,   // Flag bits for logging level
    RFID_FLAG_LOG_LEVEL_SHIFT   = 24            // Number of bits to shift to
                                                // get bits into low-order bits
};

////////////////////////////////////////////////////////////////////////////////
// File-scoped helper function declarations
////////////////////////////////////////////////////////////////////////////////

namespace
{
////////////////////////////////////////////////////////////////////////////////
// Name: AcquireLibraryLock
//
// Description:
//   Attempts to acquire the library lock.  If the library lock has not been
//   initialized or if after acquiring the lock, it is determined that the 
//   library is not initialized, then the lock fails and an exception is
//   thrown (rfid::RfidErrorException).
//
// Parameters:
//   none
//
// Returns:
//   The handle to the library lock.  The caller is responsible for ensuring
//   that the lock is released.
////////////////////////////////////////////////////////////////////////////////
rfid::CplMutexHandle AcquireLibraryLock();

////////////////////////////////////////////////////////////////////////////////
// Name: GetRadioObject
//
// Description:
//   Locates the pointer for the radio object that corresponds to the handle
//   provided, throwing an RFID_ERROR_INVALID_HANDLE exception if it cannot
//   be found
//
// Parameters:
//   handle - the handle for the radio to find
//
// Returns:
//   A pointer to the radio object that corresponds to the handle provided
////////////////////////////////////////////////////////////////////////////////
RadioWrapper* GetRadioObject(
    RFID_RADIO_HANDLE   handle
    );

////////////////////////////////////////////////////////////////////////////////
// Name: RetrieveAndLockRadio
//
// Description:
//   Gets the radio object and attempts to acquire the radio lock.  If the lock
//   is already held the function fails and an exception is thrown
//   (rfid::RfidErrorException).
//
// Parameters:
//   handle - the handle for the radio to lock
//
// Returns:
//   A pointer to the radio wrapper object for the radio.
////////////////////////////////////////////////////////////////////////////////
RadioWrapper* RetrieveAndLockRadio(
    RFID_RADIO_HANDLE   handle
    );

////////////////////////////////////////////////////////////////////////////////
// Name: CloseAndDeleteRadioCallback
//
// Description:
//   This function is used during shutdown to ensure that all entries in the
//   active radio table are closed and deleted.
//
//   NOTE: On entry to the function, it is assumed that the library has already
//   been locked.
//
// Parameters:
//   handle - the radio's handle
//   pRadioWrapper - a pointer to the radio wrapper object that is associated
//     with the handle
//   context - context supplied on call to function.  Ignored.
//
// Returns:
//   CALLBACK_STATUS_REMOVE_AND_CONTINUE
////////////////////////////////////////////////////////////////////////////////
ActiveRadioTable::CALLBACK_STATUS CloseAndDeleteRadioCallback(
    ActiveRadioTable::TableHandle handle,
    RadioWrapper*                 pRadioWrapper,
    INT64U                        context
    );

////////////////////////////////////////////////////////////////////////////////
// Name: CloseAndDeleteRadioObject
//
// Description:
//   This function is the one actually responsible for telling the radio object
//   it should close and for deleting the object.  Because there are a couple of
//   instances where we want to do this, we'll consolidate this code in one
//   place.
//
//   NOTE: On entry to the function, it is assumed that the library has already
//   been locked.
//
// Parameters:
//   pRadioWrapper - a pointer to the radio wrapper object for the radio that
//     will be closed and deleted
//
// Returns: 
//   Nothing.
////////////////////////////////////////////////////////////////////////////////
void CloseAndDeleteRadioObject(
    RadioWrapper*   pRadioWrapper
    );

////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CCommonParameters
//
// Description:
//   Validates the 18K6C tag-operation common parameters.  Throws an
//   RFID_ERROR_INVALID_PARAMETER exception of one of the parameters is invalid
//
// Parameters:
//   pParms - a pointer to the common parameters structure
//
// Returns:
//   Nothing.
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CCommonParameters(
    const RFID_18K6C_COMMON_PARMS* pParms
    );

////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CLockPasswordPermissions
//
// Description:
//   Validates the 18K6C lock password permissions.  Throws an
//   RFID_ERROR_INVALID_PARAMETER exception if the permission is invalid.
//
// Parameters:
//   permission - the permission value
//
// Returns:
//   Nothing.
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CLockPasswordPermissions(
    RFID_18K6C_TAG_PWD_PERM permission
    );

////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CLockMemoryPermissions
//
// Description:
//   Validates the 18K6C lock memory permissions.  Throws an
//   RFID_ERROR_INVALID_PARAMETER exception if the permission is invalid.
//
// Parameters:
//   permission - the permission value
//
// Returns:
//   Nothing.
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CLockMemoryPermissions(
    RFID_18K6C_TAG_MEM_PERM permission
    );

////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CTagGroup
//
// Description:
//   Validates the fields in the 18K6C tag group structure.  Throws an
//   RFID_ERROR_INVALID_PARAMETER exception of one of the fields is invalid
//
// Parameters:
//   pGroup - a pointer to the tag group structure
//
// Returns:
//   Nothing.
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CTagGroup(
    const RFID_18K6C_TAG_GROUP* pGroup
    );

////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CSingulationAlgorithmParameters
//
// Description:
//   Validates the fields in the 18K6C singulation algorithm structures.  Throws
//   an RFID_ERROR_INVALID_PARAMETER exception of one of the fields is invalid.
//   In addition this will validate that the algorithm is valid.
//
// Parameters:
//   algorithm - singulation algorithm
//   pParms - singulation algorithm parameters structure pointer
//
// Returns:
//   Nothing.
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CSingulationAlgorithmParameters(
    RFID_18K6C_SINGULATION_ALGORITHM    algorithm,
    const void*                         pParms
    );


////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CMemoryBank
//
// Description:
//   Validates the memory bank value against the defined set of memory banks.
//   Throws an RFID_ERROR_INVALID_PARAMETER exception if memory bank fails 
//   to validate
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CMemoryBank(
    const RFID_18K6C_MEMORY_BANK    bank
    );

////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CReadCmdParms
//
// Description:
//   Validates the fields in the 18K6C read command parameeter structure.  Throws
//   an RFID_ERROR_INVALID_PARAMETER exception of one of the fields is invalid
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CReadCmdParms(
    const RFID_18K6C_READ_CMD_PARMS*    pParms
    );


////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CWriteSequentialCmdParms
//
// Description:
//   Validates the fields in the 18K6C write sequential command parameeter 
//   structure.  Throws an RFID_ERROR_INVALID_PARAMETER exception of one of the 
//   fields is invalid
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CWriteSequentialCmdParms(
    const RFID_18K6C_WRITE_SEQUENTIAL_CMD_PARMS*    pParms
    );

////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CWriteRandomCmdParms
//
// Description:
//   Validates the fields in the 18K6C write random command parameeter 
//   structure.  Throws an RFID_ERROR_INVALID_PARAMETER exception of one of the 
//   fields is invalid
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CWriteRandomCmdParms(
    const RFID_18K6C_WRITE_RANDOM_CMD_PARMS*    pParms
    );

////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CBlocEraseCmdParms
//
// Description:
//   Validates the fields in the 18K6C block erase command parameter structure.  
//   Throws an RFID_ERROR_INVALID_PARAMETER exception of one of the fields is 
//   invalid
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CBlocEraseCmdParms(
    const RFID_18K6C_BLOCK_ERASE_CMD_PARMS*    pParms
    );

} // namespace

////////////////////////////////////////////////////////////////////////////////
// Externally-exported library functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_Startup
//
// Description:
//   Initializes the RFID Reader Library
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_Startup(
    RFID_VERSION*   pLibraryVersion, 
    INT32U          flags
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        // The first time we initialize the library, there are a few
        // housekeeping details that need to be taken care of
        if (g_firstInitialization)
        {
            // Create the library lock and intialize it
            g_libraryLockHandle =
                std::auto_ptr<CPL_Mutex>(new CPL_Mutex);
            if (CPL_MutexInit(g_libraryLockHandle.get()))
            {
                throw rfid::RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
            }

            // Create the table that is used to hold active radios.
            g_pActiveRadios = 
                std::auto_ptr<ActiveRadioTable>(new ActiveRadioTable);

            // Ensure we don't do this initialization again
            g_firstInitialization = false;
        }

        // Grab the library lock
        rfid::CplMutexAutoLock libraryLock(g_libraryLockHandle.get());

        // Only do initialization work if the library is not already
        // initialized
        if (!g_libraryIsInitialized)
        {
            rfid::Tracer::RFID_LOG_SEVERITY severityThreshold =
                (flags & RFID_FLAG_LOG_LEVEL_MASK) >> RFID_FLAG_LOG_LEVEL_SHIFT;

            // Create the appropriate tracer
            std::auto_ptr<rfid::Tracer> pTracer;
            if (flags & RFID_FLAG_LOG_FILE)
            {
                pTracer = std::auto_ptr<rfid::Tracer>(
                    new rfid::TracerFile(
                        LOG_FILE_NAME.c_str(),
                        severityThreshold));
            }
            else if (flags & RFID_FLAG_LOG_CONS)
            {
                pTracer = std::auto_ptr<rfid::Tracer>(
                    new rfid::TracerConsole(
                        severityThreshold));
            }
            else
            {
                pTracer = std::auto_ptr<rfid::Tracer>(
                    new rfid::TracerNull());
            }

            pTracer->PrintMessage(
                rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                "%s,0x%.8x,0x%.8x\n",
                __FUNCTION__,
                pLibraryVersion,
                flags);

            g_radioEnumerationFunction  = 
                rfid::MacTransportLive::EnumerateAttachedRadios;
            g_radioOpenFunction         = 
                rfid::MacTransportLive::OpenRadio;

            // Now that we are exception-safe, copy the tracer pointer and mark
            // the library as initialized
            g_pTracer               = pTracer;
            g_libraryIsInitialized  = true;
        }

        // Get the version information if the caller requested it
        if (NULL != pLibraryVersion)
        {
            pLibraryVersion->major       = RFID_MAJOR_VERSION;
            pLibraryVersion->minor       = RFID_MINOR_VERSION;
            pLibraryVersion->maintenance = RFID_MAINTENANCE_VERSION;
            pLibraryVersion->release     = RFID_RELEASE_VERSION;
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Version = %d.%d.%d.%d\n",
            __FUNCTION__,
            VER_MAJOR_NUM,
            VER_MINOR_NUM,
            VER_MAINTENANCE_NUM,
            VER_RELEASE_NUM);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (std::bad_alloc&)
    {
        status = RFID_ERROR_OUT_OF_MEMORY;
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_Startup

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_Shutdown
//
// Description:
//   Shuts down RFID Reader Library, cleaning up all resources including closing
//   all open radio handles and returning radios to idle.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_Shutdown(void)
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        // Acquire the library lock
        rfid::CplMutexAutoLock libraryLock;
        libraryLock.Assume(AcquireLibraryLock());

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s\n",
            __FUNCTION__);

        // Delete any radios that are still open - this will also close them
        g_pActiveRadios->ForEach(CloseAndDeleteRadioCallback, 0);

        // Get rid of the tracer object and indicate now that the library
        // is not initialized and 
        g_pTracer.reset();
        g_libraryIsInitialized  = false;
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_Shutdown

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RetrieveAttachedRadiosList
//
// Description:
//   Retrieves the list of radio modules attached to the system.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RetrieveAttachedRadiosList(
    RFID_RADIO_ENUM*    pBuffer,
    INT32U              flags
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    RFID_UNREFERENCED_LOCAL(flags);

    try
    {
        // Acquire the library lock
        rfid::CplMutexAutoLock libraryLock;
        libraryLock.Assume(AcquireLibraryLock());

        // Validate parameters
        if ((NULL == pBuffer) ||
            (sizeof(RFID_RADIO_ENUM) != pBuffer->length))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            pBuffer,
            pBuffer->length,
            flags);

        // Let the MAC transport enumerate the radios
        g_radioEnumerationFunction(pBuffer); 

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: %d radio(s) attached\n",
            __FUNCTION__,
            pBuffer->countRadios);

    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RetrieveAttachedRadiosList

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioOpen
//
// Description:
//   Requests explicit control of a radio.
// 
// NOTE:  The flags parameter is not currently used
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioOpen(
    INT32U              cookie,
    RFID_RADIO_HANDLE*  pHandle,
    INT32U              flags
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        // Acquire the library lock
        rfid::CplMutexAutoLock libraryLock;
        libraryLock.Assume(AcquireLibraryLock());

        // Validate parameters
        if (NULL == pHandle)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            cookie,
            pHandle,
            flags);

        *pHandle = RFID_INVALID_RADIO_HANDLE;

        // Let the MAC transport open the radio
        std::auto_ptr<rfid::Radio>  pRadio(g_radioOpenFunction(cookie));

        // Create a lock for the radio object and initialize it
        std::auto_ptr<CPL_Mutex>    pRadioLock(new CPL_Mutex);
        INT32U                      result;
        result = CPL_MutexInit(pRadioLock.get());
        if (result)
        {
            g_pTracer->PrintMessage(
                rfid::Tracer::RFID_LOG_SEVERITY_ERROR,
                "%s: Failed to create radio lock.  Result = 0x%.8x\n",
                __FUNCTION__,
                result);
            throw rfid::RfidErrorException(RFID_ERROR_FAILURE, __FUNCTION__);
        }

        // Now create the radio wrapper
        std::auto_ptr<RadioWrapper> pRadioWrapper(
            new RadioWrapper(pRadio, pRadioLock));

        // Add the radio to the active radio table and once we know that
        // we cannot fail, release the auto pointer's ownership of the wrapper
        *pHandle = g_pActiveRadios->Add(pRadioWrapper.get());
        pRadioWrapper.release();

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Radio with cookie 0x%.8x has handle 0x%.8x\n",
            __FUNCTION__,
            cookie,
            *pHandle);
    }
    catch (std::bad_alloc&)
    {
        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Error allocating memory\n",
            __FUNCTION__);
        status = RFID_ERROR_OUT_OF_MEMORY;
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioOpen

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioClose
//
// Description:
//   Release control of a previously-opened radio.  On close, any currently-
//   executing or outstanding requests are cancelled and the radio is returned
//   to idle state.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioClose(
    RFID_RADIO_HANDLE   handle
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        // Acquire the library lock
        rfid::CplMutexAutoLock libraryLock;
        libraryLock.Assume(AcquireLibraryLock());

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Get the radio object from the table and then go ahead and close and 
        // delete it
        CloseAndDeleteRadioObject(GetRadioObject(handle));

        // Remove the radio from the active radio table
        g_pActiveRadios->Remove(handle);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioClose

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacWriteRegister
//
// Description:
//   Sets the low-level cregister for the radio module.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacWriteRegister(
    RFID_RADIO_HANDLE   handle,
    INT16U              address,
    INT32U              value
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.4x,0x%.8x\n",
            __FUNCTION__,
            handle,
            address,
            value);

        // Let the radio do the necessary work
        pRadioWrapper->GetRadioPointer()->WriteMacRegister(
            address,
            value);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_MacWriteRegister

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacReadRegister
//
// Description:
//   Retrieves a low-level radio module register.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacReadRegister(
    RFID_RADIO_HANDLE   handle,
    INT16U              address,
    INT32U*             pValue
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Verify parameters
        if (NULL == pValue)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.4x\n",
            __FUNCTION__,
            handle,
            address);

        // Let the radio do the necessary work
        *pValue = 
            pRadioWrapper->GetRadioPointer()->ReadMacRegister(
                address);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_MacReadRegister



////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacReadRegisterInfo
//
// Description:
//   Retrieves information regarding the type of the specified register 
////////////////////////////////////////////////////////////////////////////////

RFID_LIBRARY_API RFID_STATUS RFID_MacReadRegisterInfo(
    RFID_RADIO_HANDLE                   handle,
    INT16U                              address,
    RFID_REGISTER_INFO                  *  pInfo
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Verify parameters
        if ((NULL == pInfo) || 
            (pInfo->length != sizeof(RFID_REGISTER_INFO)))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.4x\n",
            __FUNCTION__,
            handle,
            address);

        pRadioWrapper->GetRadioPointer()->ReadMacRegisterInfo(address, pInfo);

    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_MacReadRegisterInfo
  
////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacReadBankedRegister
//
// Description:
//   Retrieves a low-level radio module banked register.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacReadBankedRegister(
    RFID_RADIO_HANDLE   handle,
    INT16U              address,
    INT16U              bankSelector,
    INT32U*             pValue
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Verify parameters
        if (NULL == pValue)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.4x, 0x%.4X\n",
            __FUNCTION__,
            handle,
            address,
            bankSelector);

        // Let the radio do the necessary work
        *pValue = 
            pRadioWrapper->GetRadioPointer()->ReadMacBankedRegister(address, bankSelector);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_MacReadBankedRegister


////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacWriteBankedRegister
//
// Description:
//   Sets the low-level banked register for the radio module.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacWriteBankedRegister(
    RFID_RADIO_HANDLE   handle,
    INT16U              address,
    INT16U              bankSelector,
    INT32U              value
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.4x,0x%.4x,0x%.8x\n",
            __FUNCTION__,
            handle,
            address,
            bankSelector,
            value);

        // Let the radio do the necessary work
        pRadioWrapper->GetRadioPointer()->WriteMacBankedRegister(
            address,
            bankSelector,
            value);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_MacWriteBankedRegister

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioSetOperationMode
//
// Description:
//   Sets the radio's operation mode.  An RFID radio module’s operation mode
//   will remain in effect until it is explicitly changed via
//   RFID_RadioSetOperationMode.  
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioSetOperationMode(
    RFID_RADIO_HANDLE           handle,
    RFID_RADIO_OPERATION_MODE   mode
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the operation mode
        switch (mode)
        {
            // Valid operation modes
            case RFID_RADIO_OPERATION_MODE_CONTINUOUS:
            case RFID_RADIO_OPERATION_MODE_NONCONTINUOUS:
            {
                break;
            }
            // Invalid operation modes
            default:
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                break;
            }
        } // switch (mode)

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            mode);

        // Let the radio object set the operation mode
        pRadioWrapper->GetRadioPointer()->SetOperationMode(mode);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioSetOperationMode

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioGetOperationMode
//
// Description:
//   Retrieves the radio's operation mode.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioGetOperationMode(
    RFID_RADIO_HANDLE           handle,
    RFID_RADIO_OPERATION_MODE*  pMode
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());
        }

        // Validate parameters
        if (NULL == pMode)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Let the radio object retreive the operation mode
        *pMode = pRadioWrapper->GetRadioPointer()->GetOperationMode();

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s: mode=0x%.8x\n",
            __FUNCTION__,
            *pMode);

    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioGetOperationMode

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioSetPowerState
//
// Description:
//   Sets the radio module's power state (not to be confused with the antenna
//   RF power).
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioSetPowerState(
    RFID_RADIO_HANDLE       handle,
    RFID_RADIO_POWER_STATE  state
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());
        }

        // Validate the power state
        switch (state)
        {
            // Valid power states
            case RFID_RADIO_POWER_STATE_FULL:
            case RFID_RADIO_POWER_STATE_STANDBY:
            {
                break;
            }
            // Invalid power states
            default:
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                break;
            }
        } // switch (state)

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            state);

        // Let the radio object set the power state
        pRadioWrapper->GetRadioPointer()->SetPowerState(state);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioSetPowerState

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioGetPowerState
//
// Description:
//   Retrieves the radio module's power state (not to be confused with the
//   antenna RF power).
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioGetPowerState(
    RFID_RADIO_HANDLE       handle,
    RFID_RADIO_POWER_STATE* pState
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate parameters
        if (NULL == pState)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Let the radio object retrieve the power state
        *pState = pRadioWrapper->GetRadioPointer()->GetPowerState();
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioGetPowerState

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioSetCurrentLinkProfile
//
// Description:
//   Sets the current link profile for the radio module.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioSetCurrentLinkProfile(
    RFID_RADIO_HANDLE   handle,
    INT32U              profile
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            profile);

        // Let the radio object set the current link profile
        pRadioWrapper->GetRadioPointer()->SetCurrentLinkProfile(profile);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioSetCurrentLinkProfile

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioGetCurrentLinkProfile
//
// Description:
//   Retrieves the current link profile for the radio module.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioGetCurrentLinkProfile(
    RFID_RADIO_HANDLE   handle,
    INT32U*             pCurrentProfile
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate parameters
        if (NULL == pCurrentProfile)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Let the radio object retrieve the current link profile
        *pCurrentProfile = 
            pRadioWrapper->GetRadioPointer()->GetCurrentLinkProfile();
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioGetCurrentLinkProfile

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioGetLinkProfile
//
// Description:
//   Retrieves the information for the specified link profile for the radio
//   module.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioGetLinkProfile(
    RFID_RADIO_HANDLE           handle,
    INT32U                      profile,
    RFID_RADIO_LINK_PROFILE*    pProfileInfo
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate parameters
        if ((NULL == pProfileInfo) ||
            (sizeof(RFID_RADIO_LINK_PROFILE) != pProfileInfo->length))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            profile);

        // Let the radio object retrieve the link profile information
        pRadioWrapper->GetRadioPointer()->GetLinkProfile(profile, pProfileInfo);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioGetLinkProfile

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioWriteLinkProfileRegister
//
// Description:
//   Writes a valut to a link-profile register for the specified link
//   profile.  A link-profile regsiter may not be written while a radio module
//   is executing a tag-protocol operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioWriteLinkProfileRegister(
    RFID_RADIO_HANDLE   handle,
    INT32U              profile,
    INT16U              address,
    INT16U              value
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            profile);

        // Let the radio object set the link-profile register
        pRadioWrapper->GetRadioPointer()->WriteLinkProfileRegister(
            profile,
            address,
            value);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioWriteLinkProfileRegister

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioReadLinkProfileRegister
//
// Description:
//   Retrieves the contents of a link-profile register for the specified link
//   profile.  A link-profile regsiter may not be read while a radio module is
//   executing a tag-protocol operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioReadLinkProfileRegister(
    RFID_RADIO_HANDLE   handle,
    INT32U              profile,
    INT16U              address,
    INT16U*             pValue
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate parameters
        if (NULL == pValue)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            profile);

        // Let the radio object retrieve the link-profile register
        *pValue = 
            pRadioWrapper->GetRadioPointer()->ReadLinkProfileRegister(
                profile,
                address);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioReadLinkProfileRegister

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_AntennaPortGetStatus
//
// Description:
//   Retrieves the status of a radio module's antenna port.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_AntennaPortGetStatus(
    RFID_RADIO_HANDLE           handle,
    INT32U                      antennaPort,
    RFID_ANTENNA_PORT_STATUS*   pStatus
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate parameters
        if ((NULL == pStatus) ||
            (sizeof(RFID_ANTENNA_PORT_STATUS) != pStatus->length))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8\n",
            __FUNCTION__,
            handle,
            antennaPort);

        // Let the radio object retrieve the antenna-port status
        pRadioWrapper->GetRadioPointer()->GetAntennaPortStatus(
            antennaPort,
            pStatus);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_AntennaPortGetStatus

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_AntennaPortSetState
//
// Description:
//   Sets the state of a radio module's antenna port.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_AntennaPortSetState(
    RFID_RADIO_HANDLE       handle,
    INT32U                  antennaPort,
    RFID_ANTENNA_PORT_STATE state
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the antenna port state
        switch (state)
        {
            // Valid antenna port states
            case RFID_ANTENNA_PORT_STATE_DISABLED:
            case RFID_ANTENNA_PORT_STATE_ENABLED:
            {
                break;
            }
            // Invalid antenna port states
            default:
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                break;
            }
        } // switch (state)

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            antennaPort,
            state);

        // Let the radio object set the antenna-port state
        pRadioWrapper->GetRadioPointer()->SetAntennaPortState(
            antennaPort,
            state);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_AntennaPortSetState

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_AntennaPortSetConfiguration
//
// Description:
//   Sets the configuration for a radio module's antenna port.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_AntennaPortSetConfiguration(
    RFID_RADIO_HANDLE               handle,
    INT32U                          antennaPort,
    const RFID_ANTENNA_PORT_CONFIG* pConfig
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Bug 11240 - backward compatibility support
        // use localConfig structure to get around const qualifier on input arg.
        //
        RFID_ANTENNA_PORT_CONFIG localConfig;
        RFID_ANTENNA_PORT_CONFIG *pLocalConfig = &localConfig;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate parameters


        // Bug 11240 - v2.3.0 backwards compatibility for old MAC Firmware... 
        // unconditionally set physicalRxPort equal to physicalTxPort.
        // This will be removed in some future release
        //
        // Need to check NULL independently now, before we attempt to copy input structure 
        //
        if (NULL == pConfig) 
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        *pLocalConfig = *pConfig;
        pLocalConfig->physicalRxPort = pLocalConfig->physicalTxPort;

        if ((sizeof(RFID_ANTENNA_PORT_CONFIG) != pLocalConfig->length)           ||
            ((!pLocalConfig->dwellTime && !pLocalConfig->numberInventoryCycles)) ||
            (RFID_MAX_ANTENNA_PORT_PHYSICAL < pLocalConfig->physicalRxPort)      ||
            (RFID_MAX_ANTENNA_PORT_PHYSICAL < pLocalConfig->physicalTxPort)      ||
            (pLocalConfig->physicalRxPort != pLocalConfig->physicalTxPort))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            antennaPort,
            pLocalConfig->powerLevel,
            pLocalConfig->dwellTime,
            pLocalConfig->numberInventoryCycles,
            pLocalConfig->physicalRxPort,
            pLocalConfig->physicalTxPort);

        // Let the radio object set the antenna-port configuration
        pRadioWrapper->GetRadioPointer()->SetAntennaPortConfiguration(
            antennaPort,
            pLocalConfig);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_AntennaPortSetConfiguration

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_AntennaPortGetConfiguration
//
// Description:
//   Retrieves the configuration for a radio module's antenna port.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_AntennaPortGetConfiguration(
    RFID_RADIO_HANDLE           handle,
    INT32U                      antennaPort,
    RFID_ANTENNA_PORT_CONFIG*   pConfig
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate parameters
        if ((NULL == pConfig) ||
            (sizeof(RFID_ANTENNA_PORT_CONFIG) != pConfig->length))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            antennaPort);

        // Let the radio object get the antenna-port configuration
        pRadioWrapper->GetRadioPointer()->GetAntennaPortConfiguration(
            antennaPort,
            pConfig);


        // Bug 11240 - v2.3.0 backwards compatibility for old host client apps... 
        // unconditionally set physicalRxPort equal to physicalTxPort.
        // This will be removed in some future release
        //
        pConfig->physicalRxPort = pConfig->physicalTxPort;
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_AntennaPortGetConfiguration

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CSetSelectCriteria
//
// Description:
//   Configures the tag-selection criteria for the ISO 18000-6C select command.
//   The supplied tag-selection criteria will be used for any tag-protocol
//   operations (i.e., RFID_18K6CTagInventory, etc.) in which the application
//   specifies that an ISO 18000-6C select command should be issued prior to
//   executing the tag-protocol operation.  The tag-selection criteria will
//   stay in effect until the next call to RFID_18K6CSetSelectCriteria.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CSetSelectCriteria(
    RFID_RADIO_HANDLE                   handle,
    const RFID_18K6C_SELECT_CRITERIA*   pCriteria,
    INT32U                              flags
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    RFID_UNREFERENCED_LOCAL(flags);

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the parameters
        if ((NULL == pCriteria) ||
            ((0 != pCriteria->countCriteria) && (NULL == pCriteria->pCriteria)))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }


        // Validate each of the selection criteria
        INT32U                             index;
        const RFID_18K6C_SELECT_CRITERION* pCriterion;

        for (index = 0, pCriterion = pCriteria->pCriteria;
             index < pCriteria->countCriteria;
             ++index, ++pCriterion)
        {
            const RFID_18K6C_SELECT_MASK*   pMask   = &pCriterion->mask;
            const RFID_18K6C_SELECT_ACTION* pAction = &pCriterion->action;

            // Validate the mask bank
            switch (pMask->bank)
            {
                // Valid memory banks
                case RFID_18K6C_MEMORY_BANK_EPC:
                case RFID_18K6C_MEMORY_BANK_TID:
                case RFID_18K6C_MEMORY_BANK_USER:
                {
                    break;
                }
                // Invalid memory banks
                case RFID_18K6C_MEMORY_BANK_RESERVED:
                default:
                {
                    throw rfid::RfidErrorException(
                        RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                    break;
                }
            } // switch (pMask->bank)

            // Validate the mask bit count
            if (RFID_18K6C_MAX_SELECT_MASK_CNT < pMask->count)
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }

            // Validate the action target
            switch (pAction->target)
            {
                // Valid targets
                case RFID_18K6C_TARGET_INVENTORY_S0:
                case RFID_18K6C_TARGET_INVENTORY_S1:
                case RFID_18K6C_TARGET_INVENTORY_S2:
                case RFID_18K6C_TARGET_INVENTORY_S3:
                case RFID_18K6C_TARGET_SELECTED:
                {
                    break;
                }
                // Invalid targets
                default:
                {
                    throw rfid::RfidErrorException(
                        RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                    break;
                }
            } // switch (pAction->target)

            // Validate the matching/non-matching action combination.  We'll
            // arbitrarily pick the match action and then verify that the
            // non-matching action is valid when used with the matching action.
            switch (pAction->action)
            {
                // Valid actions
                case RFID_18K6C_ACTION_ASLINVA_DSLINVB:
                case RFID_18K6C_ACTION_ASLINVA_NOTHING:
                case RFID_18K6C_ACTION_NOTHING_DSLINVB: 
                case RFID_18K6C_ACTION_NSLINVS_NOTHING:
                case RFID_18K6C_ACTION_DSLINVB_ASLINVA:
                case RFID_18K6C_ACTION_DSLINVB_NOTHING:
                case RFID_18K6C_ACTION_NOTHING_ASLINVA:
                case RFID_18K6C_ACTION_NOTHING_NSLINVS:
                {
                    break;
                }
                // Invalid actions
                default:
                {
                    throw rfid::RfidErrorException(
                        RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                    break;
                }
            } // switch (pAction->action)

            // If truncate was requested, then it is only valid if the
            // memory bank is EPC, the target is the SL flag, and this is
            // the last selection criterion
            //if (pAction->enableTruncate                             &&
            //    ((RFID_18K6C_MEMORY_BANK_EPC != pMask->bank)     ||
            //     (RFID_18K6C_TARGET_SELECTED != pAction->target) ||
            //     ((pCriteria->countCriteria - 1) != index)))
            // 
            // DMS:  MAC Firmware does not yest support truncation.
            if (pAction->enableTruncate)
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }
        } // for (...)

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            pCriteria->countCriteria);

        // Now that all parameters are validated, let the radio set the ISO
        // 18000-6C tag select criteria
        pRadioWrapper->GetRadioPointer()->Set18K6CSelectCriteria(pCriteria);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CSetSelectCriteria

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CGetSelectCriteria
//
// Description:
//   Retrieves the configured tag-selection criteria for the ISO 18000-6C select
//   command.  The returned tag-selection criteria are used for any tag-protocol
//   operations (i.e., RFID_18K6CTagInventory, etc.) in which the application
//   specifies that an ISO 18000-6C select command should be issued prior to 
//   executing the tag-protocol operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CGetSelectCriteria(
    RFID_RADIO_HANDLE           handle,
    RFID_18K6C_SELECT_CRITERIA* pCriteria
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the parameters
        if ((NULL == pCriteria) ||
            (pCriteria->countCriteria && (NULL == pCriteria->pCriteria)))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Now that all parameters are validated, let the radio get the ISO
        // 18000-6C tag select criteria
        pRadioWrapper->GetRadioPointer()->Get18K6CSelectCriteria(pCriteria);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CGetSelectCriteria

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CSetPostMatchCriteria
//
// Description:
//   Configures the post-singulation match criteria to be used by the RFID
//   radio module.  The supplied post-singulation match criteria will be used
//   for any tag-protocol operations (i.e., RFID_18K6CTagInventory, etc.) in
//   which the application specifies that a post-singulation match should be
//   performed on the tags that are singulated by the tag-protocol operation.
//   The post-singulation match criteria will stay in effect until the next call
//   to RFID_18K6CSetPostMatchCriteria.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CSetPostMatchCriteria(
    RFID_RADIO_HANDLE                       handle,
    const RFID_18K6C_SINGULATION_CRITERIA*  pParms,
    INT32U                                  flags
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    RFID_UNREFERENCED_LOCAL(flags);

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the parameters
        if ((NULL == pParms) ||
            ((0 != pParms->countCriteria) && (NULL == pParms->pCriteria)) ||
            (RFID_18K6C_MAX_SINGULATION_CRITERIA_CNT < pParms->countCriteria))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        // if we have a criteria specified, validate it
        if (0 != pParms->countCriteria)
        {
            // Validate the singulation match criteria
            const RFID_18K6C_SINGULATION_MASK* pMask = &pParms->pCriteria->mask;
                
            if (RFID_18K6C_MAX_SINGULATION_MASK_CNT < pMask->count)
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            pParms->countCriteria);

        // Now that all parameters are validated, let the radio set the post-
        // singulation match criteria
        pRadioWrapper->GetRadioPointer()->Set18K6CPostSingulationMatchCriteria(pParms);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CSetPostMatchCriteria

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CGetPostMatchCriteria
//
// Description:
//   Retrieves the configured post-singulation match criteria to be used by the
//   RFID radio module.  The post-singulation match criteria is used for any
//   tag-protocol operations (i.e., RFID_18K6CTagInventory, etc.) in which the
//   application specifies that a post-singulation match should be performed on
//   the tags that are singulated by the tag-protocol operation.  Post-
//   singulation match criteria may not be retrieved while a radio module is
//   executing a tag-protocol operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CGetPostMatchCriteria(
    RFID_RADIO_HANDLE                   handle,
    RFID_18K6C_SINGULATION_CRITERIA*    pParms
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the parameters
        if ((NULL == pParms) ||
            (pParms->countCriteria && (NULL == pParms->pCriteria)))           
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Now that all parameters are validated, let the radio get the post-
        // singulation match criteria
        pRadioWrapper->GetRadioPointer()->Get18K6CPostSingulationMatchCriteria(pParms);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CGetPostMatchCriteria

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CSetQueryTagGroup
//
// Description:
//   Specifies which tag group will have subsequent tag-protocol operations
//   (e.g., inventory, tag read, etc.) applied to it.  The tag group may not be
//   changed while a radio module is executing a tag-protocol operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CSetQueryTagGroup(
    RFID_RADIO_HANDLE           handle,
    const RFID_18K6C_TAG_GROUP* pGroup
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the parameters pointer
        if (NULL == pGroup)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        // Validate the tag group
        Validate18K6CTagGroup(pGroup);

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            pGroup->selected,
            pGroup->session,
            pGroup->target);

        // Now that all parameters are validated, let the radio set the tag
        // group
        pRadioWrapper->GetRadioPointer()->Set18K6CQueryTagGroup(pGroup);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CSetQueryTagGroup

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CGetQueryTagGroup
//
// Description:
//   Retrieves the tag group that will have subsequent tag-protocol operations
//   (e.g., inventory, tag read, etc.) applied to it.  The tag group may not be
//   retrieved while a radio module is executing a tag-protocol operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CGetQueryTagGroup(
    RFID_RADIO_HANDLE       handle,
    RFID_18K6C_TAG_GROUP*   pGroup
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the parameters pointer
        if (NULL == pGroup)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Now that all parameters are validated, let the radio set the tag
        // group
        pRadioWrapper->GetRadioPointer()->Get18K6CQueryTagGroup(pGroup);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CGetQueryTagGroup

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CSetCurrentSingulationAlgorithm
//
// Description:
//   Allows the application to set the currently-active singulation algorithm
//   (i.e., the one that is used when performing a tag-protocol operation
//   (e.g., inventory, tag read, etc.)).  The currently-active singulation
//   algorithm may not be changed while a radio module is executing a tag-
//   protocol operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CSetCurrentSingulationAlgorithm(
    RFID_RADIO_HANDLE                   handle,
    RFID_18K6C_SINGULATION_ALGORITHM    algorithm
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the singulation algorithm
        switch (algorithm)
        {
            // Valid singulation algorithms
            case RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ:
            case RFID_18K6C_SINGULATION_ALGORITHM_DYNAMICQ:
            {
                break;
            } // case RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ
              // case RFID_18K6C_SINGULATION_ALGORITHM_DYNAMICQ
            default:
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                break;
            }
        } // switch (algorithm)

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            algorithm);

        // Now that all parameters are validated, let the radio set the
        // currently-active singulation algorithm
        pRadioWrapper->GetRadioPointer()->SetCurrentSingulationAlgorithm(
            algorithm);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CSetCurrentSingulationAlgorithm

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CGetCurrentSingulationAlgorithm
//
// Description:
//   Allows the application to retrieve the currently-active singulation
//   algorithm.  The currently-active singulation algorithm may not be changed
//   while a radio module is executing a tag-protocol operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CGetCurrentSingulationAlgorithm(
    RFID_RADIO_HANDLE                   handle,
    RFID_18K6C_SINGULATION_ALGORITHM*   pAlgorithm
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the singulation algorithm pointer
        if (NULL == pAlgorithm)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Get the currently-active singulation algorithm from the radio
        *pAlgorithm = 
            pRadioWrapper->GetRadioPointer()->GetCurrentSingulationAlgorithm();
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CGetCurrentSingulationAlgorithm

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CSetSingulationAlgorithmParameters
//
// Description:
//   Allows the application to configure the settings for a particular
//   singulation algorithm.  A singulation algorithm may not be configured while
//   a radio module is executing a tag-protocol operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CSetSingulationAlgorithmParameters(
    RFID_RADIO_HANDLE                   handle,
    RFID_18K6C_SINGULATION_ALGORITHM    algorithm,
    const void*                         pParms
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the parameters pointer
        if (NULL == pParms)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        // Validate the singulation algorithm and parameters
        Validate18K6CSingulationAlgorithmParameters(algorithm, pParms);

        switch (algorithm)
        {
            case RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ:
            {
                const RFID_18K6C_SINGULATION_FIXEDQ_PARMS* pSingulationParms = 
                    static_cast<const RFID_18K6C_SINGULATION_FIXEDQ_PARMS *>(
                        pParms);

                g_pTracer->PrintMessage(
                    rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                    "%s,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.8x\n",
                    __FUNCTION__,
                    handle,
                    algorithm,
                    pSingulationParms->qValue,
                    pSingulationParms->retryCount,
                    pSingulationParms->toggleTarget,
                    pSingulationParms->repeatUntilNoTags);

                // Now that all parameters are validated, let the radio set the
                // singulation-algorithm parameters
                pRadioWrapper->GetRadioPointer()->SetSingulationAlgorithmParameters(
                    pSingulationParms);
                break;
            } // case RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ
            case RFID_18K6C_SINGULATION_ALGORITHM_DYNAMICQ:
            {
                const RFID_18K6C_SINGULATION_DYNAMICQ_PARMS* 
                    pSingulationParms =
                        static_cast<const 
                            RFID_18K6C_SINGULATION_DYNAMICQ_PARMS *>(
                                pParms);

                g_pTracer->PrintMessage(
                    rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                    "%s,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.8x\n",
                    __FUNCTION__,
                    handle,
                    algorithm,
                    pSingulationParms->startQValue,
                    pSingulationParms->minQValue,
                    pSingulationParms->maxQValue,
                    pSingulationParms->retryCount,
                    pSingulationParms->toggleTarget,
                    pSingulationParms->thresholdMultiplier);

                // Now that all parameters are validated, let the radio set the
                // singulation-algorithm parameters
                pRadioWrapper->GetRadioPointer()->SetSingulationAlgorithmParameters(
                    pSingulationParms);

                break;
            } // case RFID_18K6C_SINGULATION_ALGORITHM_DYNAMICQ
            default:
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                break;
            }
        } // switch (algorithm)
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CSetSingulationAlgorithmParameters

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CGetSingulationAlgorithmParameters
//
// Description:
//   Allows the application to retrieve the settings for a particular
//   singulation algorithm.  Singulation-algorithm parameters may not be
//   retrieved while a radio module is executing a tag-protocol operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CGetSingulationAlgorithmParameters(
    RFID_RADIO_HANDLE                   handle,
    RFID_18K6C_SINGULATION_ALGORITHM    algorithm,
    void*                               pParms
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the parameters pointer
        if (NULL == pParms)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        // Validate that the structure's length is correct
        switch (algorithm)
        {
            case RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ:
            {
                RFID_18K6C_SINGULATION_FIXEDQ_PARMS* pSingulationParms = 
                    static_cast<RFID_18K6C_SINGULATION_FIXEDQ_PARMS *>(
                        pParms);

                if (sizeof(RFID_18K6C_SINGULATION_FIXEDQ_PARMS) != 
                    pSingulationParms->length)
                {
                    throw rfid::RfidErrorException(
                        RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                }

                g_pTracer->PrintMessage(
                    rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                    "%s,0x%.8x,0x%.8x\n",
                    __FUNCTION__,
                    handle,
                    algorithm);

                // Now that all parameters are validated, let the radio retrieve the
                // singulation-algorithm parameters
                pRadioWrapper->GetRadioPointer()->GetSingulationAlgorithmParameters(
                    pSingulationParms);

                break;
            } // case RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ
            case RFID_18K6C_SINGULATION_ALGORITHM_DYNAMICQ:
            {
                RFID_18K6C_SINGULATION_DYNAMICQ_PARMS* 
                    pSingulationParms = 
                        static_cast<
                            RFID_18K6C_SINGULATION_DYNAMICQ_PARMS *>(
                                pParms);

                // Verify the singulation algorithm parameter values
                if (sizeof(RFID_18K6C_SINGULATION_DYNAMICQ_PARMS) != 
                    pSingulationParms->length)
                {
                    throw rfid::RfidErrorException(
                        RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                }

                g_pTracer->PrintMessage(
                    rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                    "%s,0x%.8x,0x%.8x\n",
                    __FUNCTION__,
                    handle,
                    algorithm);

                // Now that all parameters are validated, let the radio retrieve the
                // singulation-algorithm parameters
                pRadioWrapper->GetRadioPointer()->GetSingulationAlgorithmParameters(
                    pSingulationParms);

                break;
            } // case RFID_18K6C_SINGULATION_ALGORITHM_THRESH
            default:
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                break;
        } // switch (algorithm)
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CGetSingulationAlgorithmParameters





////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CTagInventory
//
// Description:
//   Executes a tag inventory for the tags of interest.  If the
//   RFID_FLAG_PERFORM_SELECT flag is specified, the tag population is
//   partitioned (i.e., ISO 18000-6C select) prior to the inventory operation.
//   If the RFID_FLAG_PERFORM_POST_MATCH flag is specified, the post-singulation
//   match mask is applied to a singulated tag's EPC to determine if the tag
//   will be returned to the application.  The operation-response packets
//   will be returned to the application via the application-supplied callback
//   function.  An application may prematurely stop an inventory operation by
//   calling RFID_Radio{Cancel|Abort}Operation on another thread or by returning
//   a non-zero value from the callback function.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagInventory(
    RFID_RADIO_HANDLE                   handle,
    const RFID_18K6C_INVENTORY_PARMS*   pParms,
    INT32U                              flags
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create a scope so that the lock is released after the inventory is
        // started.  Once the inventory is started, we can start processing
        // packets and know that the radio object will be valid as it cannot
        // go away until it gets at least a chance to start processing packets.
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

            // Validate the parameters.
            if ((NULL == pParms) ||
                (sizeof(RFID_18K6C_INVENTORY_PARMS) != pParms->length))
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }

            // Validate the 18K6C common parameters
            Validate18K6CCommonParameters(&pParms->common);

            g_pTracer->PrintMessage(
                rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                "%s,0x%.8x,0x%.8x,0x%.8x\n",
                __FUNCTION__,
                handle,
                pParms->common.tagStopCount,
                flags);

            // Start the inventory operation
            pRadioWrapper->GetRadioPointer()->Start18K6CInventory(
                pParms,
                flags);
        }

        // Now process the operation response packets
        pRadioWrapper->GetRadioPointer()->ProcessOperationData(
            handle,
            pParms->common.pCallback,
            pParms->common.context,
            pParms->common.pCallbackCode);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CTagInventory

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CTagRead
//
// Description:
//   Executes a tag read for the tags of interest.  If the
//   RFID_FLAG_PERFORM_SELECT flag is specified, the tag population is
//   partitioned (i.e., ISO 18000-6C select) prior to the tag-read operation.
//   If the RFID_FLAG_PERFORM_POST_MATCH flag is specified, the post-singulation
//   match mask is applied to a singulated tag's EPC to determine if the tag
//   will be read from.  Reads may only be performed on 16-bit word boundaries
//   and for multiples of 16-bit words.  If one or more of the memory words
//   specified by the offset/count combination do not exist or are read-locked,
//   the read from the tag will fail and this failure will be reported through
//   the operation response packet.    The operation-response packets will
//   be returned to the application via the application-supplied callback
//   function.  Each tag-read record is grouped with its corresponding tag-
//   inventory record.  An application may prematurely stop a read operation by
//   calling RFID_Radio{Cancel|Abort}Operation on another thread or by returning
//   a non-zero value from the callback function.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagRead(
    RFID_RADIO_HANDLE               handle,
    const RFID_18K6C_READ_PARMS*    pParms,
    INT32U                          flags
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create a scope so that the lock is released after the read is
        // started.  Once the read is started, we can start processing
        // packets and know that the radio object will be valid as it cannot
        // go away until it gets at least a chance to start processing packets.
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());


            // Validate the parameters
            if ((NULL == pParms)                                                    ||
                (sizeof(RFID_18K6C_READ_PARMS) != pParms->length))
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }

            // Validate the 18K6C read command parameters
            Validate18K6CReadCmdParms(&pParms->readCmdParms);

            // Validate the 18K6C common parameters
            Validate18K6CCommonParameters(&pParms->common);

            // Start the read operation
            pRadioWrapper->GetRadioPointer()->Start18K6CRead(pParms, flags);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x,0x%.8x,0x%.4x,0x%.4x,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            pParms->common.tagStopCount,
            pParms->readCmdParms.bank,
            pParms->readCmdParms.offset,
            pParms->readCmdParms.count,
            pParms->accessPassword,
            flags);

        // Now process the operation response packets
        pRadioWrapper->GetRadioPointer()->ProcessOperationData(
            handle,
            pParms->common.pCallback,
            pParms->common.context,
            pParms->common.pCallbackCode);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CTagRead

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CTagWrite
//
// Description:
//   Executes a tag write for the tags of interest.  If the
//   RFID_FLAG_PERFORM_SELECT flag is specified, the tag population is
//   partitioned (i.e., ISO 18000-6C select) prior to the tag-write operation.
//   If the RFID_FLAG_PERFORM_POST_MATCH flag is specified, the post-singulation
//   match mask is applied to a singulated tag's EPC to determine if the tag
//   will be written to.  Writes may only be performed on 16-bit word boundaries
//   and for multiples of 16-bit words.  If one or more of the specified memory
//   words do not exist or are write-locked, the write to the tag will fail and
//   this failure will be reported through the operation-response packet.  The
//   operation-response packets will be returned to the application via
//   the application-supplied callback function.  Each tag-write record is
//   grouped with its corresponding tag-inventory record.  An application may
//   prematurely stop a write operation by calling
//   RFID_Radio{Cancel|Abort}Operation on another thread or by returning a non-
//   zero value from the callback function.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagWrite(
    RFID_RADIO_HANDLE               handle,
    const RFID_18K6C_WRITE_PARMS*   pParms,
    INT32U                          flags 
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create a scope so that the lock is released after the write is
        // started.  Once the write is started, we can start processing
        // packets and know that the radio object will be valid as it cannot
        // go away until it gets at least a chance to start processing packets.
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

            // Validate the parameters that are common to all writes.
            if ((NULL == pParms)                                    ||
                (sizeof(RFID_18K6C_WRITE_PARMS) != pParms->length))
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }

            // Validate the 18K6C common parameters
            Validate18K6CCommonParameters(&pParms->common);

            // Validate the parameters that are specific to the different types
            // of writes
            switch (pParms->writeType)
            {
                case RFID_18K6C_WRITE_TYPE_SEQUENTIAL:
                {
                    const RFID_18K6C_WRITE_SEQUENTIAL_CMD_PARMS* pWriteParms =
                        &(pParms->writeCmdParms.sequential);

                    // Validate the sequential write parameters
                    Validate18K6CWriteSequentialCmdParms(pWriteParms);

                    g_pTracer->PrintMessage(
                        rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                        "%s,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.4x,0x%.4x,"
                        "0x%.8x,0x%.8x\n",
                        __FUNCTION__,
                        handle,
                        pParms->common.tagStopCount,
                        pParms->writeType,
                        pParms->writeCmdParms.sequential.bank,
                        pParms->writeCmdParms.sequential.count,
                        pParms->writeCmdParms.sequential.offset,
                        pParms->accessPassword,
                        flags);

                    break;
                } // case RFID_18K6C_WRITE_TYPE_SEQUENTIAL

                case RFID_18K6C_WRITE_TYPE_RANDOM:
                {
                    const RFID_18K6C_WRITE_RANDOM_CMD_PARMS* pWriteParms =
                        &(pParms->writeCmdParms.random);

                    // Validate the random write parameters
                    Validate18K6CWriteRandomCmdParms(pWriteParms);

                    g_pTracer->PrintMessage(
                        rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                        "%s,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.4x,"
                        "0x%.8x,0x%.8x\n",
                        __FUNCTION__,
                        handle,
                        pParms->common.tagStopCount,
                        pParms->writeType,
                        pParms->writeCmdParms.random.bank,
                        pParms->writeCmdParms.random.count,
                        pParms->accessPassword,
                        flags);

                    break;
                } // case RFID_18K6C_WRITE_TYPE_RANDOM
                default:
                {
                    throw rfid::RfidErrorException(
                        RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                    break;
                } // default
            } // switch (pParms->writeType)

            // Start the write operation
            pRadioWrapper->GetRadioPointer()->Start18K6CWrite(pParms, flags);
        }

        // Now process the operation response packets
        pRadioWrapper->GetRadioPointer()->ProcessOperationData(
            handle,
            pParms->common.pCallback,
            pParms->common.context,
            pParms->common.pCallbackCode);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CTagWrite

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CTagKill
//
// Description:
//   Executes a tag kill for the tags of interest.  If the
//   RFID_FLAG_PERFORM_SELECT flag is specified, the tag population is
//   partitioned (i.e., ISO 18000-6C select) prior to the tag-kill operation.
//   If the RFID_FLAG_PERFORM_POST_MATCH flag is specified, the post-singulation
//   match mask is applied to a singulated tag's EPC to determine if the tag
//   will be killed.  The operation-response packets will be returned to the
//   application via the application-supplied callback function.  Each tag-kill
//   record is grouped with its corresponding tag-inventory record.  An
//   application may prematurely stop a kill operation by calling
//   RFID_Radio{Cancel|Aobrt}Operation on another thread or by returning a non-
//   zero value from the callback function.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagKill(
    RFID_RADIO_HANDLE               handle,
    const RFID_18K6C_KILL_PARMS*    pParms,
    INT32U                          flags
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create a scope so that the lock is released after the kill is
        // started.  Once the kill is started, we can start processing
        // packets and know that the radio object will be valid as it cannot
        // go away until it gets at least a chance to start processing packets.
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());


            // Valide the parameters
            if ((NULL == pParms)                                    ||
                (sizeof(RFID_18K6C_KILL_PARMS) != pParms->length)   ||
                (sizeof(RFID_18K6C_KILL_CMD_PARMS) != pParms->killCmdParms.length))
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }

            // Validate the 18K6C common parameters
            Validate18K6CCommonParameters(&pParms->common);

            g_pTracer->PrintMessage(
                rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                "%s,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.8x\n",
                __FUNCTION__,
                handle,
                pParms->common.tagStopCount,
                pParms->accessPassword,
                pParms->killCmdParms.killPassword,
                flags);

            // Start the kill operation
            pRadioWrapper->GetRadioPointer()->Start18K6CKill(pParms, flags);
        }

        // Now process the operation response packets
        pRadioWrapper->GetRadioPointer()->ProcessOperationData(
            handle,
            pParms->common.pCallback,
            pParms->common.context,
            pParms->common.pCallbackCode);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CTagKill

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CTagLock
//
// Description:
//   Executes a tag lock for the tags of interest.  If the
//   RFID_FLAG_PERFORM_SELECT flag is specified, the tag population is
//   partitioned (i.e., ISO 18000-6C select) prior to the tag-lock operation.
//   If the RFID_FLAG_PERFORM_POST_MATCH flag is specified, the post-singulation
//   match mask is applied to a singulated tag's EPC to determine if the tag
//   will be locked.  The operation-response packets will be returned to the
//   application via the application-supplied callback function.  Each tag-lock
//   record is grouped with its corresponding tag-inventory record.  An
//   application may prematurely stop a lock operation by calling
//   RFID_Radio{Cancel|Abort}Operation on another thread or by returning a non-
//   zero value from the callback function.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagLock(
    RFID_RADIO_HANDLE               handle,
    const RFID_18K6C_LOCK_PARMS*    pParms,
    INT32U                          flags
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create a scope so that the lock is released after the lock is
        // started.  Once the lock is started, we can start processing
        // packets and know that the radio object will be valid as it cannot
        // go away until it gets at least a chance to start processing packets.
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

            // Validate parameters
            if ((NULL == pParms)    ||
                (sizeof(RFID_18K6C_LOCK_PARMS) != pParms->length) ||
                (sizeof(RFID_18K6C_LOCK_CMD_PARMS) != pParms->lockCmdParms.length))
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }

            // Validate the password and memory persmissions
            Validate18K6CLockPasswordPermissions(
                pParms->lockCmdParms.permissions.killPasswordPermissions);
            Validate18K6CLockPasswordPermissions(
                pParms->lockCmdParms.permissions.accessPasswordPermissions);
            Validate18K6CLockMemoryPermissions(
                pParms->lockCmdParms.permissions.epcMemoryBankPermissions);
            Validate18K6CLockMemoryPermissions(
                pParms->lockCmdParms.permissions.userMemoryBankPermissions);
            Validate18K6CLockMemoryPermissions(
                pParms->lockCmdParms.permissions.tidMemoryBankPermissions);

            // Validate the 18K6C common parameters
            Validate18K6CCommonParameters(&pParms->common);

            g_pTracer->PrintMessage(
                rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                "%s,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.8x,"
                "0x%.8x\n",
                __FUNCTION__,
                handle,
                pParms->common.tagStopCount,
                pParms->lockCmdParms.permissions.killPasswordPermissions,
                pParms->lockCmdParms.permissions.accessPasswordPermissions,
                pParms->lockCmdParms.permissions.epcMemoryBankPermissions,
                pParms->lockCmdParms.permissions.userMemoryBankPermissions,
                pParms->lockCmdParms.permissions.tidMemoryBankPermissions,
                pParms->accessPassword,
                flags);

            // Start the lock operation
            pRadioWrapper->GetRadioPointer()->Start18K6CLock(pParms, flags);
        }

        // Now process the operation response packets
        pRadioWrapper->GetRadioPointer()->ProcessOperationData(
            handle,
            pParms->common.pCallback,
            pParms->common.context,
            pParms->common.pCallbackCode);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CTagLock

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CTagBlockWrite
//
// Description:
//   Executes a block write for the tags of interest.  If the
//   RFID_FLAG_PERFORM_SELECT flag is specified, the tag population is
//   partitioned (i.e., ISO 18000-6C select) prior to the block-write operation.
//   If the RFID_FLAG_PERFORM_POST_MATCH flag is specified, the post-singulation
//   match mask is applied to a singulated tag's EPC to determine if the tag
//   will be written to.  Writes may only be performed on 16-bit word boundaries
//   and for multiples of 16-bit words.  If one or more of the specified memory
//   words do not exist or are write-locked, the write to the tag will fail and
//   this failure will be reported through the operation-response packet.  The
//   operation-response packets will be returned to the application via
//   the application-supplied callback function.  Each block-write record is
//   grouped with its corresponding tag-inventory record.  An application may
//   prematurely stop a write operation by calling
//   RFID_Radio{Cancel|Abort}Operation on another thread or by returning a non-
//   zero value from the callback function.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagBlockWrite(
    RFID_RADIO_HANDLE                     handle,
    const RFID_18K6C_BLOCK_WRITE_PARMS*   pBWParms,
    INT32U                                flags 
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create a scope so that the lock is released after the write is
        // started.  Once the write is started, we can start processing
        // packets and know that the radio object will be valid as it cannot
        // go away until it gets at least a chance to start processing packets.
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

            // Validate the block write specific parameters.
            if ((NULL == pBWParms)                                                                ||
                (sizeof(RFID_18K6C_BLOCK_WRITE_PARMS) != pBWParms->length)                        ||
                (sizeof(RFID_18K6C_BLOCK_WRITE_CMD_PARMS) != pBWParms->blockWriteCmdParms.length) ||
                (0 == pBWParms->blockWriteCmdParms.count)                                         ||
                (RFID_18K6C_MAX_BLOCK_WRITE_COUNT < pBWParms->blockWriteCmdParms.count)           ||
                (NULL == pBWParms->blockWriteCmdParms.pData))
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }
    
            // Validate the 18K6C common parameters
            Validate18K6CCommonParameters(&pBWParms->common);
    
    
    
            // Validate the memory bank
            switch (pBWParms->blockWriteCmdParms.bank)
            {
                // Valid memory bank values
                case RFID_18K6C_MEMORY_BANK_RESERVED:
                case RFID_18K6C_MEMORY_BANK_EPC:
                case RFID_18K6C_MEMORY_BANK_TID:
                case RFID_18K6C_MEMORY_BANK_USER:
                {
                    break;
                }
                default:
                {
                    throw rfid::RfidErrorException(
                        RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                    break;
                }
            } // switch (pBWParms->bank)
    
            g_pTracer->PrintMessage(
                    rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                    "%s,0x%.8x,0x%.8x,0x%.8x,0x%.8x,0x%.4x,"
                    "0x%.8x,0x%.8x\n",
                    __FUNCTION__,
                    handle,
                    pBWParms->common.tagStopCount,
                    pBWParms->blockWriteCmdParms.bank,
                    pBWParms->blockWriteCmdParms.count,
                    pBWParms->accessPassword,
                    flags);
        
            // Start the write operation
            pRadioWrapper->GetRadioPointer()->Start18K6CBlockWrite(pBWParms, flags);
        }

        // Now process the operation response packets
        pRadioWrapper->GetRadioPointer()->ProcessOperationData(
            handle,
            pBWParms->common.pCallback,
            pBWParms->common.context,
            pBWParms->common.pCallbackCode);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CTagBlockWrite


////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CTagQT
//
// Description:
//   Executes a tag QT command with actions to perform upon the tag QT control 
//   word, optionally performs a Read or Write(s) of one or more 16-bit words 
//   for all tags of interest.  If the RFID_FLAG_PERFORM_SELECT flag is 
//   specified, the tag population will be partitioned (i.e., one or more ISO 
//   18000-6C select commands are issued) before the tag-QT operation.  If the
//   RFID_FLAG_PERFORM_POST_MATCH flag is specified, a singulated tag's EPC 
//   will be matched against post-singulation match mask to determine if QT is 
//   to be performed on the tag. Refer to the tag read and write section for 
//   the optionally performed Read or Write(s) commands for QT.  Failures will 
//   be reported through the operation-response packet.  The function will 
//   return the operation-response packets via the application-supplied 
//   callback function.  The function will return when the tag-QT operation has 
//   completed or has been cancelled.  An application may prematurely stop a 
//   tag-QT operation by calling RFID_RadioCancelOperation or 
//   RFID_RadioAbortOperation on another thread or by returning a non-zero 
//   value from the callback function.  A tag QT operation may not be requested 
//   while a radio module is executing a tag-protocol operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagQT(
    RFID_RADIO_HANDLE               handle,
    const RFID_18K6C_QT_PARMS*      pParms,
    INT32U                          flags 
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create a scope so that the lock is released after the QT is
        // started.  Once the QT is started, we can start processing
        // packets and know that the radio object will be valid as it cannot
        // go away until it gets at least a chance to start processing packets.
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

            // Validate the parameters that are common to all QT operations.
            // Note:  Can't check the 
            if ((NULL == pParms)                                                         ||
                (sizeof(RFID_18K6C_QT_PARMS) != pParms->length)                          ||
                (sizeof(RFID_18K6C_QT_CMD_PARMS) != pParms->qtCmdParms.length)           ||
                (pParms->optCmdType < RFID_18K6C_QT_OPT_NONE)                            ||
                (pParms->optCmdType > RFID_18K6C_QT_OPT_WRITE_TYPE_RANDOM)               ||
                (pParms->qtCmdParms.qtMemoryMap < RFID_18K6C_QT_MEMMAP_PRIVATE)          ||
                (pParms->qtCmdParms.qtMemoryMap > RFID_18K6C_QT_MEMMAP_PUBLIC)           ||
                (pParms->qtCmdParms.qtShortRange < RFID_18K6C_QT_SR_DISABLE)             ||
                (pParms->qtCmdParms.qtShortRange > RFID_18K6C_QT_SR_ENABLE)              ||
                (pParms->qtCmdParms.qtPersistence < RFID_18K6C_QT_PERSISTENCE_TEMPORARY) ||
                (pParms->qtCmdParms.qtPersistence > RFID_18K6C_QT_PERSISTENCE_PERMANENT) ||
                (pParms->qtCmdParms.qtReadWrite < RFID_18K6C_QT_CTRL_READ)               ||
                (pParms->qtCmdParms.qtReadWrite > RFID_18K6C_QT_CTRL_WRITE))
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }

            // Validate the 18K6C common parameters
            Validate18K6CCommonParameters(&pParms->common);

            // Validate the parameters that are specific to the different types
            // of optional access commands
            switch (pParms->optCmdType)
            {
                // no optional access command, there's nothing else to do
                case RFID_18K6C_QT_OPT_NONE:
                {
                    break; 
                }

                case RFID_18K6C_QT_OPT_READ:
                {
                    const RFID_18K6C_READ_CMD_PARMS* pReadParms = 
                                        &(pParms->parameters.readCmdParms);

                    // Validate the 18K6C read command parameters
                    Validate18K6CReadCmdParms(&pParms->parameters.readCmdParms);


                    g_pTracer->PrintMessage(
                        rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                        "%s,0x%.8x,0x%.8x,0x%.8x,0x%.4x,0x%.4x\n",
                        __FUNCTION__,
                        handle,
                        pParms->optCmdType,
                        pReadParms->bank,
                        pReadParms->offset,
                        pReadParms->count);

                    break;
                }

                case RFID_18K6C_QT_OPT_WRITE_TYPE_SEQUENTIAL:
                {
                    const RFID_18K6C_WRITE_SEQUENTIAL_CMD_PARMS* pWriteParms = 
                                                    &(pParms->parameters.seqWriteCmdParms);

                    // Validate the sequential write parameters
                    Validate18K6CWriteSequentialCmdParms(pWriteParms);

                    g_pTracer->PrintMessage(
                        rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                        "%s,0x%.8x,,0x%.8x,0x%.8x,0x%.4x,0x%.4x\n",
                        __FUNCTION__,
                        handle,
                        pParms->optCmdType,
                        pWriteParms->bank,
                        pWriteParms->count,
                        pWriteParms->offset);

                    break;
                } // case RFID_18K6C_WRITE_TYPE_SEQUENTIAL

                case RFID_18K6C_QT_OPT_WRITE_TYPE_RANDOM:
                {
                    const RFID_18K6C_WRITE_RANDOM_CMD_PARMS* pWriteParms =
                                                 &(pParms->parameters.randWriteCmdParms);

                    // Validate the random write parameters
                    Validate18K6CWriteRandomCmdParms(pWriteParms);

                    g_pTracer->PrintMessage(
                        rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                        "%s,0x%.8x,0x%.8x,0x%.8x,0x%.4x\n",
                        __FUNCTION__,
                        handle,
                        pParms->optCmdType,
                        pWriteParms->bank,
                        pWriteParms->count);

                    break;
                } // case RFID_18K6C_WRITE_TYPE_RANDOM
                default:
                {
                    throw rfid::RfidErrorException(
                        RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                    break;
                } // default
            } // switch (pParms->optCmdType)

            // Start the QT operation
            
            g_pTracer->PrintMessage(
                rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                "%s,0x%.8x,0x%.8x,0x%.4x,0x%.4x,0x%.4x,0x%.4x,0x%.8x,0x%.8x\n",
                __FUNCTION__,
                handle,
                pParms->common.tagStopCount,
                pParms->qtCmdParms.qtMemoryMap,
                pParms->qtCmdParms.qtShortRange,
                pParms->qtCmdParms.qtPersistence,
                pParms->qtCmdParms.qtReadWrite,
                pParms->accessPassword,
                flags);

            pRadioWrapper->GetRadioPointer()->Start18K6CQT(pParms, flags);
        }

        // Now process the operation response packets
        pRadioWrapper->GetRadioPointer()->ProcessOperationData(
            handle,
            pParms->common.pCallback,
            pParms->common.context,
            pParms->common.pCallbackCode);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CTagQT


////////////////////////////////////////////////////////////////////////////////
// Name: RFID_18K6CTagBlockErase
//
// Description:
//   Executes a tag block erase for the tags of interest.  If the
//   RFID_FLAG_PERFORM_SELECT flag is specified, the tag population is
//   partitioned (i.e., ISO 18000-6C select) prior to the tag-read operation.
//   If the RFID_FLAG_PERFORM_POST_MATCH flag is specified, the post-singulation
//   match mask is applied to a singulated tag's EPC to determine if the tag 
//   memory will be erased.  Block erase may only be performed on 16-bit word 
//   boundaries and for multiples of 16-bit words.  If one or more of the memory 
//   words specified by the offset/count combination do not exist or are 
//   write-locked, the block erase of the tag will fail and this failure will 
//   be reported through the operation response packet.    The 
//   operation-response packets will be returned to the application via the 
//   application-supplied callback function.  Each tag-read record is grouped 
//   with its corresponding tag-inventory record.  An application may 
//   prematurely stop a block erase operation by calling 
//   RFID_Radio{Cancel|Abort}Operation on another thread or by returning
//   a non-zero value from the callback function.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_18K6CTagBlockErase(
    RFID_RADIO_HANDLE                      handle,
    const RFID_18K6C_BLOCK_ERASE_PARMS*    pParms,
    INT32U                                 flags
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create a scope so that the lock is released after the block erase is
        // started.  Once the read is started, we can start processing
        // packets and know that the radio object will be valid as it cannot
        // go away until it gets at least a chance to start processing packets.
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());


            // Validate the parameters
            if ((NULL == pParms)                                                    ||
                (sizeof(RFID_18K6C_BLOCK_ERASE_PARMS) != pParms->length))
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }

            // Validate the 18K6C block erase command parameters
            Validate18K6CBlocEraseCmdParms(&pParms->blockEraseCmdParms);

            // Validate the 18K6C common parameters
            Validate18K6CCommonParameters(&pParms->common);

            // Start the read operation
            pRadioWrapper->GetRadioPointer()->Start18K6CBlockErase(pParms, flags);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x,0x%.8x,0x%.4x,0x%.4x,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            pParms->common.tagStopCount,
            pParms->blockEraseCmdParms.bank,
            pParms->blockEraseCmdParms.offset,
            pParms->blockEraseCmdParms.count,
            pParms->accessPassword,
            flags);

        // Now process the operation response packets
        pRadioWrapper->GetRadioPointer()->ProcessOperationData(
            handle,
            pParms->common.pCallback,
            pParms->common.context,
            pParms->common.pCallbackCode);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_18K6CTagBlockErase


////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioCancelOperation
//
// Description:
//   Stops a currently-executing tag-protocol operation on a radio module.  The
//   packet callback function will be executed until the command-end packet is
//   received from the MAC or the packet callback returns a non-zero result.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioCancelOperation(
    RFID_RADIO_HANDLE   handle,
    INT32U              flags
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    RFID_UNREFERENCED_LOCAL(flags);

    try
    {
        // Acquire the library lock
        rfid::CplMutexAutoLock libraryLock;
        libraryLock.Assume(AcquireLibraryLock());

        // Get the radio object from the table.
        RadioWrapper* pRadioWrapper = GetRadioObject(handle);

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            flags);

        // Let the radio object cancel the operation
        pRadioWrapper->GetRadioPointer()->CancelOperation();
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioCancelOperation

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioAbortOperation
//
// Description:
//   Stops a currently-executing tag-protocol operation on a radio module and
//   discards all remaining command-reponse packets.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioAbortOperation(
    RFID_RADIO_HANDLE   handle,
    INT32U              flags
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    RFID_UNREFERENCED_LOCAL(flags);

    try
    {
        // Acquire the library lock
        rfid::CplMutexAutoLock libraryLock;
        libraryLock.Assume(AcquireLibraryLock());

        // Get the radio object from the table.
        RadioWrapper* pRadioWrapper = GetRadioObject(handle);

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            flags);

        // Let the radio object abort the operation
        pRadioWrapper->GetRadioPointer()->AbortOperation();
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioAbortOperation

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioSetResponseDataMode
//
// Description:
//   Sets the operation response data reporting mode for tag-protocol
//   operations.  By default, the reporting mode is set to "normal".  The 
//   reporting mode will remain in effect until a subsequent call to
//   RFID_RadioSetResponseDataMode.  The mode may not be changed while the
//   radio is executing a tag-protocol operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioSetResponseDataMode(
    RFID_RADIO_HANDLE   handle,
    RFID_RESPONSE_TYPE  responseType,
    RFID_RESPONSE_MODE  responseMode
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the response type
        if (RFID_RESPONSE_TYPE_DATA != responseType)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        // Validate the reponse mode
        switch (responseMode)
        {
            // Valid response modes
            case RFID_RESPONSE_MODE_COMPACT:
            case RFID_RESPONSE_MODE_NORMAL:
            case RFID_RESPONSE_MODE_EXTENDED:
            {
                break;
            }
            // Invalid response modes
            default:
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
                break;
            }
        } // switch (responseMode)

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            responseType,
            responseMode);

        // Let the radio object set the response mode
        pRadioWrapper->GetRadioPointer()->SetResponseDataMode(
            responseType,
            responseMode);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioSetResponseDataMode

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioGetResponseDataMode
//
// Description:
//   Retrieves the operation response data reporting mode for tag-protocol
//   operations.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioGetResponseDataMode(
    RFID_RADIO_HANDLE   handle,
    RFID_RESPONSE_TYPE  responseType,
    RFID_RESPONSE_MODE* pResponseMode
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate parameters
        if ((RFID_RESPONSE_TYPE_DATA != responseType)   ||
            (NULL == pResponseMode))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            responseType);

        // Let the radio object retrieve the response mode
        *pResponseMode = 
            pRadioWrapper->GetRadioPointer()->GetResponseDataMode(responseType);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioSetResponseDataMode

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacUpdateNonvolatileMemory
//
// Description:
//   Attempts to write the specified data to the radio module’s nonvolatile-
//   memory block(s).
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacUpdateNonvolatileMemory(
    RFID_RADIO_HANDLE                       handle,
    INT32U                                  countBlocks,
    const RFID_NONVOLATILE_MEMORY_BLOCK*    pBlocks,
    INT32U                                  flags
    )
{
    RFID_STATUS status = RFID_STATUS_OK;
    RadioWrapper*           pRadioWrapper   = NULL;

    try
    {
        rfid::CplMutexAutoLock  radioLock;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }


        // Validate parameters
        if (!countBlocks || (NULL == pBlocks))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        for (INT32U index = 0; index < countBlocks; ++index)
        {
            if (!pBlocks[index].length || (NULL == pBlocks[index].pData))
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            countBlocks,
            flags);

        // Let the radio object update the MAC firmware
        pRadioWrapper->GetRadioPointer()->UpdateMacNonvolatileMemory(
            countBlocks,
            pBlocks,
            flags);


        // no matter what happens (error or success) we remove the radio to force the
        // application to reconnect.  This simplifies behavior in the long run.
        //
        g_pActiveRadios->Remove(handle);
        delete pRadioWrapper;
        pRadioWrapper = NULL;

    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    // if pointer is not NULL, we took an exception path, so need to do the cleanup
    // that normally happens in the succes path.
    //
    if (pRadioWrapper != NULL)
    {
        // no matter what happens (error or success) we remove the radio to force the
        // application to reconnect.  This simplifies behavior in the long run.
        //
        g_pActiveRadios->Remove(handle);
        delete pRadioWrapper;
    }

    return status;
} // RFID_MacUpdateNonvolatileMemory

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacGetVersion
//
// Description:
//   Retrieves the radio module's MAC firmware version information.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacGetVersion(
    RFID_RADIO_HANDLE   handle,
    RFID_VERSION*       pVersion
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Verify the parameters
        if (NULL == pVersion)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Get the MAC version
        pRadioWrapper->GetRadioPointer()->GetMacVersion(pVersion);

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: MAC Version = %d.%d.%d.%d\n",
            __FUNCTION__,
            pVersion->major,
            pVersion->minor,
            pVersion->maintenance,
            pVersion->release);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_MacGetVersion


////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacGetBootLoaderVersion
//
// Description:
//   Retrieves the radio module's BootLoader firmware version information.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacGetBootLoaderVersion(
    RFID_RADIO_HANDLE   handle,
    RFID_VERSION*       pVersion
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Verify the parameters
        if (NULL == pVersion)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Get the MAC version
        pRadioWrapper->GetRadioPointer()->GetMacBootLoaderVersion(pVersion);

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: MAC Version = %d.%d.%d.%d\n",
            __FUNCTION__,
            pVersion->major,
            pVersion->minor,
            pVersion->maintenance,
            pVersion->release);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_MacGetBootLoaderVersion
  
////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacReadOemData
//
// Description:
//   Reads one or more 32-bit words from the MAC's OEM configuration data
//   area.
// NOTE:
//   pData is loaded with data in MAC Format (little endian)
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacReadOemData(
    RFID_RADIO_HANDLE   handle,
    INT32U              address,
    INT32U*             pCount,
    INT32U*             pData
    )
{
    RFID_STATUS status   = RFID_STATUS_OK;
    INT32U      locCount = 0;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate parameters
        if ((NULL == pCount) || (0 == *pCount) || (NULL == pData))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            address,
            *pCount);

        // Let the radio ojbect read the OEM data
        locCount = *pCount;
        pRadioWrapper->GetRadioPointer()->ReadMacOemData(address, &locCount, pData);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    *pCount = locCount;
    return status;
} // RFID_MacReadOemData

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacWriteOemData
//
// Description:
//   Writes one or more 32-bit words to the MAC's OEM configuration data
//   area.  
// NOTE:  
//   pData must point to data that is already in MAC format (little endian)
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacWriteOemData(
    RFID_RADIO_HANDLE   handle,
    INT32U              address,
    INT32U*             pCount,
    const INT32U*       pData
    )
{
    RFID_STATUS status   = RFID_STATUS_OK;
    INT32U      locCount = 0;
    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate parameters
        if ((NULL == pCount) || (0 == *pCount) || (NULL == pData))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            address,
            *pCount);

        locCount = *pCount;
        // Let the radio ojbect write the OEM data
        pRadioWrapper->GetRadioPointer()->WriteMacOemData(
            address,
            &locCount,
            pData);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    // update the caller's count on return
    *pCount = locCount;
    return status;

} // RFID_MacWriteOemData

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacReset
//
// Description:
//   Instructs the radio module's MAC firmware to perform the specified reset.
//   Any currently executing tag-protocol operations will be aborted, any
//   unconsumed data will be discarded, and tag-protocol operation functions
//   (i.e., RFID_18K6CTagInventory, etc.) will return immediately with an
//   error of RFID_ERROR_OPERATION_CANCELLED.
//   Upon reset, the connection to the radio module is lost and the handle
//   to the radio is invalid.  To obtain control of the radio module after it
//   has been reset, the application must re-enumerate the radio modules, via
//   RFID_RetrieveAttachedRadiosList, and request control via RFID_RadioOpen.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacReset(
    RFID_RADIO_HANDLE   handle,
    RFID_MAC_RESET_TYPE resetType
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        // Acquire the library lock
        rfid::CplMutexAutoLock libraryLock;
        libraryLock.Assume(AcquireLibraryLock());

        // Get the radio object
        RadioWrapper* pRadioWrapper = GetRadioObject(handle);

        // Validate the parameters
        if ((RFID_MAC_RESET_TYPE_SOFT          != resetType) &&
            (RFID_MAC_RESET_TYPE_TO_BOOTLOADER != resetType))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            resetType);

        // Let the radio object reset the MAC
        pRadioWrapper->GetRadioPointer()->ResetMac(resetType);

        // Remove the radio from the active radio table and delete the radio
        // object
        g_pActiveRadios->Remove(handle);
        delete pRadioWrapper;
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_MacReset

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacClearError
//
// Description:
//   Attempts to clear the error state for the radio module’s MAC firmware.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacClearError(
    RFID_RADIO_HANDLE   handle
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Let the radio object attempt to clear the MAC error state
        pRadioWrapper->GetRadioPointer()->ClearMacError();
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_MacClearError


////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacGetError
//
// Description:
//   Retrieves the current value of the MAC_ERROR and MAC_LAST_ERROR registers
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacGetError(
    RFID_RADIO_HANDLE   handle,
    INT32U*             pError,
    INT32U*             pLastError
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Verify parameters
        if ((NULL == pError) ||
            (NULL == pLastError))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Let the radio do the necessary work
        pRadioWrapper->GetRadioPointer()->GetMacError(pError, pLastError);
   
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_MacGetError
  // 
////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacBypassWriteRegister
//
// Description:
//   Allows for direct writing of registers on the radio (i.e., bypassing the
//   MAC).
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacBypassWriteRegister(
    RFID_RADIO_HANDLE   handle,
    INT16U              address,
    INT16U              value
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.4x,0x%.4x\n",
            __FUNCTION__,
            handle,
            address,
            value);

        // Let the radio object write the radio register value
        pRadioWrapper->GetRadioPointer()->WriteRadioRegister(address, value);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_MacBypassWriteRegister

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacBypassReadRegister
//
// Description:
//   Allows for direct reading of registers on the radio (i.e., bypassing the
//   MAC).
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacBypassReadRegister(
    RFID_RADIO_HANDLE   handle,
    INT16U              address,
    INT16U*             pValue
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());
        }

        // Validate the parmaters
        if (NULL == pValue)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            address);

        // Let the radio object write the radio register value
        *pValue = pRadioWrapper->GetRadioPointer()->ReadRadioRegister(address);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_MacBypassReadRegister


////////////////////////////////////////////////////////////////////////////////
// Name: RFID_MacGetRegion
//
// Description:
//   Retrieves the regulatory mode region for the MAC's operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_MacGetRegion(
    RFID_RADIO_HANDLE   handle,
    RFID_MAC_REGION*    pRegion,
    void*               pRegionConfig
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate parameters
        if ((NULL == pRegion) || (NULL != pRegionConfig))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Let the radio object attempt to retrieve the regulatory mode region
        *pRegion = pRadioWrapper->GetRadioPointer()->GetMacRegion(pRegionConfig);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_MacGetRegion

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioSetGpioPinsConfiguration
//
// Description:
//   Configures the specified radio module's GPIO pins.  For version 1.0 of the
//   library, only GPIO pins 0-3 are valid.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioSetGpioPinsConfiguration(
    RFID_RADIO_HANDLE   handle,
    INT32U              mask,
    INT32U              configuration
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            mask,
            configuration);

        // Let the radio configure the GPIO pins
        pRadioWrapper->GetRadioPointer()->SetGpioPinsConfiguration(
            mask,
            configuration);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioSetGpioPinsConfiguration

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioGetGpioPinsConfiguration
//
// Description:
//   Retrieves the configuration for the radio module's GPIO pins.  For version
//   1.0 of the library, only GPIO pins 0-3 are valid.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioGetGpioPinsConfiguration(
    RFID_RADIO_HANDLE   handle,
    INT32U*             pConfiguration
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate parameters
        if (NULL == pConfiguration)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Let the radio object retrieve the GPIO pin configuration
        *pConfiguration = 
            pRadioWrapper->GetRadioPointer()->GetGpioPinsConfiguration();
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioGetGpioPinsConfiguration

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioReadGpioPins
//
// Description:
//   Reads the specified radio module's GPIO pins.  Attempting to read from an
//   output GPIO pin results in an error.  For version 1.0 of the library, only
//   GPIO pins 0-3 are valid.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioReadGpioPins(
    RFID_RADIO_HANDLE   handle,
    INT32U              mask,
    INT32U*             pValue
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate parameters
        if (NULL == pValue)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Let the radio object read the GPIO pins
        *pValue = pRadioWrapper->GetRadioPointer()->ReadGpioPins(mask);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioReadGpioPins

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioWriteGpioPins
//
// Description:
//   Writes the specified radio module's GPIO pins.  Attempting to write to an
//   input GPIO pin results in an error.  For version 1.0 of the library, only
//   GPIO pins 0-3 are valid.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioWriteGpioPins(
    RFID_RADIO_HANDLE   handle,
    INT32U              mask,
    INT32U              value
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            mask,
            value);

        // Let the radio object write the GPIO pins
        pRadioWrapper->GetRadioPointer()->WriteGpioPins(mask, value);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioWriteGpioPins


////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioTurnCarrierWaveOn
//
// Description:
//   Turns on the radio's RF carrier wave.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioTurnCarrierWaveOn(
    RFID_RADIO_HANDLE   handle
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());
        }

        // Let the radio object turn on the carrier wave
        pRadioWrapper->GetRadioPointer()->ToggleCarrierWave(true);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioTurnCarrierWaveOn

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioTurnCarrierWaveOff
//
// Description:
//   Turns off the radio's RF carrier wave.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioTurnCarrierWaveOff(
    RFID_RADIO_HANDLE   handle
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Let the radio object turn off the carrier wave
        pRadioWrapper->GetRadioPointer()->ToggleCarrierWave(false);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioTurnCarrierWaveOff

////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioIssueCommand
//
// Description:
//   Allows an application to issue any radio command.  It is the application's
//   responsibility to process the resulting packets and verify that the
//   command was successful.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioIssueCommand(
    RFID_RADIO_HANDLE                       handle,
    const RFID_ISSUE_RADIO_COMMAND_PARMS*   pParms
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create a scope so that the lock is released after the command is
        // started.  Once the command is started, we can start processing
        // packets and know that the radio object will be valid as it cannot
        // go away until it gets at least a chance to start processing packets.
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());


            // Validate the parameters.
            if ((NULL == pParms) ||
                (sizeof(RFID_ISSUE_RADIO_COMMAND_PARMS) != pParms->length))
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }

            g_pTracer->PrintMessage(
                rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
                "%s,0x%.8x,0x%.8x\n",
                __FUNCTION__,
                handle,
                pParms->command);

            // Start the operation
            pRadioWrapper->GetRadioPointer()->IssueCommand(pParms->command);
        }

        // Now process the operation response packets
        pRadioWrapper->GetRadioPointer()->ProcessOperationData(
            handle,
            pParms->pCallback,
            pParms->context,
            pParms->pCallbackCode);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioIssueCommand


////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioTurnCarrierWaveOnRandom
//
// 
// Description:
//   Turns on the radio's RF carrier wave with random transmit data for the 
//   specified duration.  It is the application's responsibility to process the 
//   resulting packets and verify that the command was successful.
// 
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioTurnCarrierWaveOnRandom(
    RFID_RADIO_HANDLE                       handle,
    const RFID_RANDOM_CW_PARMS*             pParms
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create a scope so that the lock is released after the command is
        // started.  Once the command is started, we can start processing
        // packets and know that the radio object will be valid as it cannot
        // go away until it gets at least a chance to start processing packets.
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

            // Validate the parameters.
            if ((NULL == pParms) ||
                (sizeof(RFID_RANDOM_CW_PARMS) != pParms->length))
            {
                throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }

            // Let the radio object setup the registers and start the random CW operation
            pRadioWrapper->GetRadioPointer()->RandomCarrierWave(pParms->duration);
        }

        // Now process the operation response packets
        pRadioWrapper->GetRadioPointer()->ProcessOperationData(
            handle,
            pParms->pCallback,
            pParms->context,
            pParms->pCallbackCode);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioTurnCarrierWaveOnRandom



////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioSetImpinjExtensions
//
// Description:
//   
//   Specifies which Impinj Extensions are enabled.  By default, only the auto
//   two word BlockWrite extension is enabled. The Impinj Extensions will remain 
//   in effect until a subsequent call to RFID_RadioSetImpinjExtensions.  The 
//   Impinj Extensions may not be changed while the radio is executing a 
//   tag-protocol operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioSetImpinjExtensions(
    RFID_RADIO_HANDLE                    handle,
    const RFID_IMPINJ_EXTENSIONS*        pExtensions)

{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the parameters
        if ((pExtensions->blockWriteMode  > RFID_BLOCKWRITE_MODE_FORCE_TWO)     ||
            (pExtensions->tagFocus  > RFID_TAG_FOCUS_ENABLED)        ||
            (pExtensions->fastId   > RFID_FAST_ID_ENABLED))
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }


        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x,0x%.8x,0x%.8x,0x%.8x\n",
            __FUNCTION__,
            handle,
            pExtensions->blockWriteMode,
            pExtensions->tagFocus,
            pExtensions->fastId);

        // Let the radio object set the impinj extensions
        pRadioWrapper->GetRadioPointer()->SetImpinjExtensions(
            pExtensions);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioSetImpinjExtensions


////////////////////////////////////////////////////////////////////////////////
// Name: RFID_RadioGetImpinjExtensions
//
// Description:
//   Retrieves the Impinj Extensions from the Radio.  The extensions may not be
//   retrieved while a radio module is executing a tag-protocol operation.
////////////////////////////////////////////////////////////////////////////////
RFID_LIBRARY_API RFID_STATUS RFID_RadioGetImpinjExtensions(
    RFID_RADIO_HANDLE         handle,
    RFID_IMPINJ_EXTENSIONS*   pExtensions
    )
{
    RFID_STATUS status = RFID_STATUS_OK;

    try
    {
        rfid::CplMutexAutoLock  radioLock;
        RadioWrapper*           pRadioWrapper;

        // Create an explicit scope so that we release the library lock as soon
        // as we have the radio lock
        {
            // Acquire the library lock
            rfid::CplMutexAutoLock libraryLock;
            libraryLock.Assume(AcquireLibraryLock());

            // Get the radio object and wrap the lock so it is automatically
            // released
            pRadioWrapper = RetrieveAndLockRadio(handle);
            radioLock.Assume(pRadioWrapper->GetRadioLockHandle());

        }

        // Validate the parameters pointer
        if (NULL == pExtensions)
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
        }

        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_TRACE,
            "%s,0x%.8x\n",
            __FUNCTION__,
            handle);

        // Now that all parameters are validated, let the radio set the impinj extensions
        pRadioWrapper->GetRadioPointer()->GetImpinjExtensions(pExtensions);
    }
    catch (rfid::RfidErrorException& error)
    {
        status = error.GetError();
    }
    catch (...)
    {
        status = RFID_ERROR_FAILURE;
    }

    return status;
} // RFID_RadioGetImpinjExtensions
  


////////////////////////////////////////////////////////////////////////////////
// File-scoped helper function defintions
////////////////////////////////////////////////////////////////////////////////

namespace
{

////////////////////////////////////////////////////////////////////////////////
// Name: AcquireLibraryLock
//
// Description:
//   Attempts to acquire the library lock.  If the library lock has not been
//   initialized or if after acquiring the lock, it is determined that the 
//   library is not initialized, then the lock fails and an exception is
//   thrown (rfid::RfidErrorException).
////////////////////////////////////////////////////////////////////////////////
rfid::CplMutexHandle AcquireLibraryLock()
{
    // Make sure that the library has been initialized at least once
    if (NULL == g_libraryLockHandle.get())
    {
        throw rfid::RfidErrorException(RFID_ERROR_NOT_INITIALIZED, __FUNCTION__);
    }

    // Lock the library and verify it is initialized
    rfid::CplMutexAutoLock libraryLock(g_libraryLockHandle.get());
    if (!g_libraryIsInitialized)
    {
        throw rfid::RfidErrorException(RFID_ERROR_NOT_INITIALIZED, __FUNCTION__);
    }

    // Now return the lock so that the caller can unlock when they are done
    return libraryLock.Transfer();
} // AcquireLibraryLock

////////////////////////////////////////////////////////////////////////////////
// Name: GetRadioObject
//
// Description:
//   Locates the pointer for the radio object that corresponds to the handle
//   provided, throwing an RFID_ERROR_INVALID_HANDLE exception if it cannot
//   be found
////////////////////////////////////////////////////////////////////////////////
RadioWrapper* GetRadioObject(
    RFID_RADIO_HANDLE   handle
    )
{
    // Get the radio object from the table and verify that there
    // actually is one.
    RadioWrapper* pRadioWrapper = g_pActiveRadios->Get(handle);
    if (NULL == pRadioWrapper)
    {
        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_INFO,
            "%s: Unable to find radio 0x%.8x\n",
            __FUNCTION__,
            handle);
        throw rfid::RfidErrorException(RFID_ERROR_INVALID_HANDLE, __FUNCTION__);
    }

    return pRadioWrapper;
} // GetRadioObject

////////////////////////////////////////////////////////////////////////////////
// Name: RetrieveAndLockRadio
//
// Description:
//   Gets the radio object and attempts to acquire the radio lock.  If the lock
//   is already held the function fails and an exception is thrown
//   (rfid::RfidErrorException).
////////////////////////////////////////////////////////////////////////////////
RadioWrapper* RetrieveAndLockRadio(
    RFID_RADIO_HANDLE   handle
    )
{
    // First get the wrapper object for the radio
    RadioWrapper* pRadioWrapper = GetRadioObject(handle);

    // Now, attempt to lock the radio
    INT32U status;
    status = CPL_MutexTryLock(pRadioWrapper->GetRadioLockHandle());
    if (status)
    {
        g_pTracer->PrintMessage(
            rfid::Tracer::RFID_LOG_SEVERITY_ERROR,
            "%s: Unable to obtain lock for radio 0x%.8x\n",
            __FUNCTION__,
            handle);
        throw rfid::RfidErrorException(CPL_WARN_WOULDBLOCK == status ?
                                            RFID_ERROR_RADIO_BUSY :
                                            RFID_ERROR_FAILURE, __FUNCTION__);
    }

    return pRadioWrapper;
} // RetrieveAndLockRadio

////////////////////////////////////////////////////////////////////////////////
// Name: CloseAndDeleteRadioCallback
//
// Description:
//   This function is used during shutdown to ensure that all entries in the
//   active radio table are closed and deleted.
//
//   NOTE: On entry to the function, it is assumed that the library has already
//   been locked.
//
////////////////////////////////////////////////////////////////////////////////
ActiveRadioTable::CALLBACK_STATUS CloseAndDeleteRadioCallback(
    ActiveRadioTable::TableHandle handle,
    RadioWrapper*                 pRadioWrapper,
    INT64U                        context
    )
{
    RFID_UNREFERENCED_LOCAL(context);

    g_pTracer->PrintMessage(
        rfid::Tracer::RFID_LOG_SEVERITY_INFO,
        "%s: Close and delete radio 0x%.8x\n",
        __FUNCTION__,
        handle);

    // Close the radio and delete the object
    CloseAndDeleteRadioObject(pRadioWrapper);

    // Let the active radios table know that it can remove the entry as we have
    // deleted the object
    return ActiveRadioTable::CALLBACK_STATUS_REMOVE_AND_CONTINUE;
} // CloseAndDeleteRadioCallback

////////////////////////////////////////////////////////////////////////////////
// Name: CloseAndDeleteRadioObject
//
// Description:
//   This function is the one actually responsible for telling the radio object
//   it should close and for deleting the object.
//
//   NOTE: On entry to the function, it is assumed that the library has already
//   been locked.
//
////////////////////////////////////////////////////////////////////////////////
void CloseAndDeleteRadioObject(
    RadioWrapper*   pRadioWrapper
    )
{
    // Tell the radio to close
    try
    {
        pRadioWrapper->GetRadioPointer()->Close();
    }
    catch (rfid::RfidErrorException& error)
    {
        // If we are not allowed to close the radio, then let this exception
        // percolate up
        if (RFID_ERROR_CURRENTLY_NOT_ALLOWED == error.GetError())
        {
            throw;
        }
    }
    catch (...)
    {
    }

    // Wrap the radio wrapper with an auto_ptr to ensure that it gets destroyed
    // on exit from the function (no matter what).  The wrapper will ensure that
    // the radio and its lock are properly deleted.
    std::auto_ptr<RadioWrapper> pWrapper(pRadioWrapper);

    // Wait for the radio lock to ensure that the radio is not in use any more.
    // We only care about threads that may have retrieved and locked the radio
    // pointer before this function was called.  During this callback, the
    // library is locked so other threads will be blocked until the callback is
    // finished.
    try
    {
        rfid::CplMutexAutoLock lock(pRadioWrapper->GetRadioLockHandle());
    }
    catch (...)
    {
    }
} // CloseAndDeleteRadioObject

////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CCommonParameters
//
// Description:
//   Validates the 18K6C tag-operation common parameters.  Throws an
//   RFID_ERROR_INVALID_PARAMETER exception of one of the parameters is invalid
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CCommonParameters(
    const RFID_18K6C_COMMON_PARMS* pParms
    )
{
    assert(NULL != pParms);

    // Validate the parameters
    if ((NULL == pParms->pCallback) ||
        (RFID_18K6C_MAX_TAGSTOP < pParms->tagStopCount))
    {
        throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }
} // Validate18K6CCommonParameters

////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CLockPasswordPermissions
//
// Description:
//   Validates the 18K6C lock password permissions.  Throws an
//   RFID_ERROR_INVALID_PARAMETER exception if the permission is invalid.
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CLockPasswordPermissions(
    RFID_18K6C_TAG_PWD_PERM permission
    )
{
    // Validate the password permission
    switch (permission)
    {
        // Valid password permissions
        case RFID_18K6C_TAG_PWD_PERM_ACCESSIBLE:
        case RFID_18K6C_TAG_PWD_PERM_ALWAYS_ACCESSIBLE:
        case RFID_18K6C_TAG_PWD_PERM_SECURED_ACCESSIBLE:
        case RFID_18K6C_TAG_PWD_PERM_ALWAYS_NOT_ACCESSIBLE:
        case RFID_18K6C_TAG_PWD_PERM_NO_CHANGE:
        {
            break;
        }
        default:
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            break;
        }
    } // switch (pParms->permissions.killPasswordPermissions)
} // Validate18K6CLockPasswordPermissions

////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CLockMemoryPermissions
//
// Description:
//   Validates the 18K6C lock memory permissions.  Throws an
//   RFID_ERROR_INVALID_PARAMETER exception if the permission is invalid.
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CLockMemoryPermissions(
    RFID_18K6C_TAG_MEM_PERM permission
    )
{

    // Validate the memory persmission
    switch (permission)
    {
        // Valid memory permissions
        case RFID_18K6C_TAG_PWD_PERM_ACCESSIBLE:
        case RFID_18K6C_TAG_PWD_PERM_ALWAYS_ACCESSIBLE:
        case RFID_18K6C_TAG_PWD_PERM_SECURED_ACCESSIBLE:
        case RFID_18K6C_TAG_PWD_PERM_ALWAYS_NOT_ACCESSIBLE:
        case RFID_18K6C_TAG_PWD_PERM_NO_CHANGE:
        {
            break;
        }
        default:
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            break;
        }
    } // switch (pParms->permissions.killPasswordPermissions)
} // Validate18K6CLockMemoryPermissions

////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CTagGroup
//
// Description:
//   Validates the fields in the 18K6C tag group structure.  Throws an
//   RFID_ERROR_INVALID_PARAMETER exception of one of the fields is invalid
//
// Parameters:
//   pGroup - a pointer to the tag group structure
//
// Returns:
//   Nothing.
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CTagGroup(
    const RFID_18K6C_TAG_GROUP* pGroup
    )
{
    // Verify the selected flag setting
    switch (pGroup->selected)
    {
        // Valid selected flag settings
        case RFID_18K6C_SELECT_ALL:
        case RFID_18K6C_SELECT_DEASSERTED:
        case RFID_18K6C_SELECT_ASSERTED:
        {
            break;
        }
        default:
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            break;
        }
    } // switch (pGroup->selected)

    // Verify the inventory session
    switch (pGroup->session)
    {
        // Valid inventory sessions
        case RFID_18K6C_INVENTORY_SESSION_S0:
        case RFID_18K6C_INVENTORY_SESSION_S1:
        case RFID_18K6C_INVENTORY_SESSION_S2:
        case RFID_18K6C_INVENTORY_SESSION_S3:
        {
            break;
        }
        default:
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            break;
        }
    } // switch (pGroup->session)

    // Verify the inventory target
    switch (pGroup->target)
    {
        // Valid inventory targets
        case RFID_18K6C_INVENTORY_SESSION_TARGET_A:
        case RFID_18K6C_INVENTORY_SESSION_TARGET_B:
        {
            break;
        }
        default:
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            break;
        }
    } // switch (pGroup->target)
}

////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CSingulationAlgorithmParameters
//
// Description:
//   Validates the fields in the 18K6C singulation algorithm structures.  Throws
//   an RFID_ERROR_INVALID_PARAMETER exception of one of the fields is invalid
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CSingulationAlgorithmParameters(
    RFID_18K6C_SINGULATION_ALGORITHM    algorithm,
    const void*                         pParms
    )
{
    // Verify the singulation algorithm information
    switch (algorithm)
    {
        // Verify the fixed Q singulation algorithm parameters
        case RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ:
        {
            const RFID_18K6C_SINGULATION_FIXEDQ_PARMS* pSingulationParms = 
                static_cast<const RFID_18K6C_SINGULATION_FIXEDQ_PARMS *>(
                    pParms);

            // Verify the singulation algorithm parameter values
            if ((sizeof(RFID_18K6C_SINGULATION_FIXEDQ_PARMS) != 
                 pSingulationParms->length)                         ||
                 (RFID_18K6C_MAX_Q < pSingulationParms->qValue)     ||
                 (RFID_18K6C_MAX_RETRY < pSingulationParms->retryCount))
            {
                throw rfid::RfidErrorException(
                    RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }

            break;
        } // case RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ
        // Verify the dynamic Q with threshold adjustment singulation algorithm
        // parameters
        case RFID_18K6C_SINGULATION_ALGORITHM_DYNAMICQ:
        {
            const RFID_18K6C_SINGULATION_DYNAMICQ_PARMS* 
                pSingulationParms =
                    static_cast<const 
                        RFID_18K6C_SINGULATION_DYNAMICQ_PARMS *>(
                            pParms);

            // Verify the singulation algorithm parameter values
            if ((sizeof(RFID_18K6C_SINGULATION_DYNAMICQ_PARMS) !=
                 pSingulationParms->length)                             ||
                 (RFID_18K6C_MAX_Q < pSingulationParms->startQValue)    ||
                 (RFID_18K6C_MAX_Q < pSingulationParms->minQValue)      ||
                 (RFID_18K6C_MAX_Q < pSingulationParms->maxQValue)      ||
                 (pSingulationParms->startQValue < 
                  pSingulationParms->minQValue)                         ||
                 (pSingulationParms->startQValue >
                  pSingulationParms->maxQValue)                         ||
                 (RFID_18K6C_MAX_RETRY < pSingulationParms->retryCount) ||
                 (RFID_18K6C_MAX_THRESH_MULTIPLIER <
                  pSingulationParms->thresholdMultiplier))
            {
                throw rfid::RfidErrorException(
                    RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            }

            break;
        } // case RFID_18K6C_SINGULATION_ALGORITHM_DYNAMICQ
        default:
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            break;
        }
    } // switch (algorithm)
} // Validate18K6CSingulationAlgorithmParameters



////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CMemoryBank
//
// Description:
//   Validates the memory bank value against the defined set of memory banks.
//   Throws an RFID_ERROR_INVALID_PARAMETER exception if memory bank fails 
//   to validate
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CMemoryBank(
    const RFID_18K6C_MEMORY_BANK    bank
    )
{

    switch (bank)
    {
        // Valid memory banks
        case RFID_18K6C_MEMORY_BANK_RESERVED:
        case RFID_18K6C_MEMORY_BANK_EPC:
        case RFID_18K6C_MEMORY_BANK_TID:
        case RFID_18K6C_MEMORY_BANK_USER:
        {
            break;
        }
        // Invalid memory banks
        default:
        {
            throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
            break;
        }
    } // switch (bank)
}


////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CReadCmdParms
//
// Description:
//   Validates the fields in the 18K6C read command parameter structure.  Throws
//   an RFID_ERROR_INVALID_PARAMETER exception of one of the fields is invalid
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CReadCmdParms(
    const RFID_18K6C_READ_CMD_PARMS*    pParms
    )
{
    if ((NULL == pParms)                                            ||
        (sizeof(RFID_18K6C_READ_CMD_PARMS) != pParms->length)       ||
        (RFID_18K6C_MAX_READ_COUNT < pParms->count))
    {
        throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    // Validate the memory bank
    Validate18K6CMemoryBank(pParms->bank);    
    
} // Validate18K6CReadCmdParms



////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CWriteSequentialCmdParms
//
// Description:
//   Validates the fields in the 18K6C write sequential command parameter 
//   structure.  Throws an RFID_ERROR_INVALID_PARAMETER exception of one of the 
//   fields is invalid
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CWriteSequentialCmdParms(
    const RFID_18K6C_WRITE_SEQUENTIAL_CMD_PARMS*    pParms
    )
{

    // Validate the sequential write parameters
    if ((NULL == pParms)                                                   ||
        (sizeof(RFID_18K6C_WRITE_SEQUENTIAL_CMD_PARMS) != pParms->length)  ||
        (0 == pParms->count)                                               ||
        (RFID_18K6C_MAX_WRITE_COUNT < pParms->count)                       ||
        (NULL == pParms->pData))
    {
        throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    // Validate the memory bank
    Validate18K6CMemoryBank(pParms->bank);        

} // Validate18K6CWriteSequentialCmdParms


////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CWriteRandomCmdParms
//
// Description:
//   Validates the fields in the 18K6C write random command parameter 
//   structure.  Throws an RFID_ERROR_INVALID_PARAMETER exception of one of the 
//   fields is invalid
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CWriteRandomCmdParms(
    const RFID_18K6C_WRITE_RANDOM_CMD_PARMS*    pParms
    )
{

    // Validate the random write parameters
    if ((NULL == pParms)                                                   ||
        (sizeof(RFID_18K6C_WRITE_RANDOM_CMD_PARMS) != pParms->length) ||
        (0 == pParms->count)                                          ||
        (RFID_18K6C_MAX_WRITE_COUNT < pParms->count)                  ||
        (NULL == pParms->pOffset)                                     ||
        (NULL == pParms->pData))
    {
        throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    // Validate the memory bank
    Validate18K6CMemoryBank(pParms->bank);        

} // Validate18K6CWriteRandomCmdParms
 

////////////////////////////////////////////////////////////////////////////////
// Name: Validate18K6CBlocEraseCmdParms
//
// Description:
//   Validates the fields in the 18K6C block erase command parameter structure.  
//   Throws an RFID_ERROR_INVALID_PARAMETER exception of one of the fields is 
//   invalid
////////////////////////////////////////////////////////////////////////////////
void Validate18K6CBlocEraseCmdParms(
    const RFID_18K6C_BLOCK_ERASE_CMD_PARMS*    pParms
    )
{
    if ((NULL == pParms)                                             ||
        (sizeof(RFID_18K6C_BLOCK_ERASE_CMD_PARMS) != pParms->length) ||
        (0 == pParms->count)                                         ||
        (RFID_18K6C_MAX_BLOCK_ERASE_COUNT < pParms->count))
    {
        throw rfid::RfidErrorException(RFID_ERROR_INVALID_PARAMETER, __FUNCTION__);
    }

    // Validate the memory bank
    Validate18K6CMemoryBank(pParms->bank);    
    
} // Validate18K6CBlocEraseCmdParms

} // namespace
