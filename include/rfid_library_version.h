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
 * $Id: rfid_library_version.h 59732 2009-09-04 01:31:04Z dshaheen $
 * 
 * Description:
 *     This header presents the RFID library's version-specific information
 *     
 *
 *****************************************************************************
 */

#ifndef RFID_LIBRARY_VERSION_H_INCLUDED
#define RFID_LIBRARY_VERSION_H_INCLUDED

// Define the proper file type
#ifdef VER_FILETYPE
#undef VER_FILETYPE
#endif
#define VER_FILETYPE            VFT_DLL

// Define the file description
#ifdef VER_FILEDESCRIPTION_STR
#undef VER_FILEDESCRIPTION_STR
#endif
#define VER_FILEDESCRIPTION_STR "Indy RFID Reader Library\0"

// Define the internal name
#ifdef VER_INTERNALNAME_STR
#undef VER_INTERNALNAME_STR
#endif
#define VER_INTERNALNAME_STR    "Indy RFID Reader Library\0"

// Define the original file name
#ifdef VER_ORIGINALFILENAME_STR
#undef VER_ORIGINALFILENAME_STR
#endif
#define VER_ORIGINALFILENAME_STR "rfid.dll\0"

#endif // #ifndef RFID_LIBRARY_VERSION_H_INCLUDED
