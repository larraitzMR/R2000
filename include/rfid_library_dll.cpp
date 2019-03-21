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
 * $Id: rfid_library_dll.cpp 62064 2010-01-26 22:13:33Z dshaheen $
 * 
 * Description:
 *     This file contains the required entry point for the DLL.
 *     
 * Contents:
 *     DllMain() - main DLL entry point
 *     
 *
 *****************************************************************************
 */

#include <windows.h>
#include "rfid_platform_types.h"

////////////////////////////////////////////////////////////////////////////////
// Name: DllMain
//
// Description:
//   Main entry point of the DLL
//
// Parameters:
//   hModule    - the handle to the DLL module
//   ulReason   - specifies the reason why the DllMain is called
//   lpReserved - reserved
//
// Returns:
//   TRUE on success, FALSE otherwise
//   - If ulReason has value of DLL_PROCESS_ATTACH:
//     If the DLL is loaded using a LoadLibrary() call, FALSE will cause
//     the LoadLibrary() function to return NULL.  If the DLL is loaded
//     during process initialization, the process terminates with an error.
//   - If ulReason has any other value, the return value from DllMain()
//     is ignored.
////////////////////////////////////////////////////////////////////////////////
BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  ulReason, 
                      LPVOID lpReserved)
{
    RFID_UNREFERENCED_LOCAL(hModule);
    RFID_UNREFERENCED_LOCAL(lpReserved);

    switch (ulReason)
    {
        case DLL_PROCESS_ATTACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
} // DllMain
