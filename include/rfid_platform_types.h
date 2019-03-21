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
 * $Id: rfid_platform_types.h 62064 2010-01-26 22:13:33Z dshaheen $
 * 
 * Description:
 *
 *     This header file defines the cross-platform types in terms of types for
 *     the Microsoft compilers
 *
 *****************************************************************************
 */

#ifndef RFID_PLATFORM_TYPES_H_INCLUDED
#define RFID_PLATFORM_TYPES_H_INCLUDED

typedef signed   __int8     INT8S;
typedef unsigned __int8     INT8U;
typedef signed   __int16    INT16S;
typedef unsigned __int16    INT16U;
typedef signed   __int32    INT32S;
typedef unsigned __int32    INT32U;
typedef signed   __int64    INT64S;
typedef unsigned __int64    INT64U;
typedef signed   __int32    BOOL32;
typedef unsigned __int32	HANDLE32;
typedef unsigned __int64	HANDLE64;

/* Specify the calling convention for function callbacks */
#define RFID_CALLBACK __cdecl

/* Macro to use to resolve warnings for unreferenced variables */
#define RFID_UNREFERENCED_LOCAL(v)   v;

#endif  /* #ifndef RFID_PLATFORM_TYPES_H_INCLUDED */
