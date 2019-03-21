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
 * $Id: mapfile.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This file contains the prototype for a functions to map and unmap files
 *     onto/from memory.
 *     
 *     NOTE - this is not bulletproof code, but simply a way to expose file
 *     mapping across platforms.  If you pass bad parameters to the functions,
 *     bad things could happen.
 *     
 *
 *****************************************************************************
 */

#ifndef MAPFILE_H_INCLUDED
#define MAPFILE_H_INCLUDED

#include <stdlib.h>
#include "rfid_types.h"

typedef void * MEMMAP_HANDLE;

#define INVALID_MEMMAP_HANDLE NULL

/******************************************************************************
 * Name: MapFileOntoMemory
 *
 * Description:
 *   Maps a file onto memory.
 *
 * Parameters:
 *   pFileName - the name of the file that will be mapped onto memory
 *
 * Returns:
 *   Returns INVALID_MEMMAP_HANDLE if the mapping failed.  Otherwise, returns
 *   the handle the file mapping.
 ******************************************************************************/
MEMMAP_HANDLE MapFileOntoMemory(
    const char* pFileName
    );

/******************************************************************************
 * Name: GetFileMappingStartAddress
 *
 * Description:
 *   Returns the starting address for the file mapping.
 *
 * Parameters:
 *   handle - the handle to the memory mapping for which the starting address is
 *     required.  The handle is returned from a successfull call to
 *     MapFileOntoMemory.
 *
 * Returns:
 *   Returns NULL if the handle is not valid.  Otherwise returns the starting
 *   address for the file mapping.
 ******************************************************************************/
void* GetFileMappingStartAddress(
    MEMMAP_HANDLE   handle
    );

/******************************************************************************
 * Name: GetFileMappingSize
 *
 * Description:
 *   Returns the size, in bytes, for the file mapping.
 *
 * Parameters:
 *   handle - the handle to the memory mapping for which the size is
 *     required.  The handle is returned from a successfull call to
 *     MapFileOntoMemory.
 *
 * Returns:
 *   Returns the size of the file mapping.
 ******************************************************************************/
INT32U GetFileMappingSize(
    MEMMAP_HANDLE   handle
    );

/******************************************************************************
 * Name: UnmapFileFromMemory
 *
 * Description:
 *   Unmaps a file from memory.
 *
 * Parameters:
 *   handle - the handle to the memory mapping to unmap.  The handle is returned
 *     from a successfull call to MapFileOntoMemory.
 *
 * Returns:
 *   Returns INVALID_MEMMAP_HANDLE if the mapping failed.  Otherwise, returns
 *   the handle the memory mapping.
 ******************************************************************************/
void UnmapFileFromMemory(
    MEMMAP_HANDLE   handle
    );

#endif /* #ifndef MAPFILE_H_INCLUDED */
