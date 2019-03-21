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
 * $Id: rfid_library_export.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header file defines the appropriate macro for exporting/importing a
 *     library built as a Windows DLL
 *     
 *
 *****************************************************************************
 */

#ifndef RFID_LIBRARY_EXPORT_H_INCLUDED
#define RFID_LIBRARY_EXPORT_H_INCLUDED

/******************************************************************************
 * The following ifdef block is the standard way of creating macros which make
 * exporting from a DLL simpler.  RFID_LIBRARY_EXPORTS should only be defined
 * when actually building the RFID Reader Library DLL as that is the only
 * code that is actually exporting the RFID API functions.
 ******************************************************************************/
#ifdef RFID_LIBRARY_EXPORTS
#define RFID_LIBRARY_API __declspec(dllexport)
#else
#define RFID_LIBRARY_API __declspec(dllimport)
#endif

#endif  /* #ifndef RFID_LIBRARY_EXPORT_H_INCLUDED */
