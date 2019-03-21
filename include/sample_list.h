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
 * $Id: sample_list.h 59754 2009-09-08 17:07:45Z dshaheen $
 * 
 * Description:
 *     This file contains the prototype for a simple linked list.
 *     
 *     NOTE - this is not bulletproof code, but is instead a simple linked list
 *     for demonstrating sample code.
 *     
 *
 *****************************************************************************
 */

#ifndef SAMPLE_LIST_H_INCLUDED
#define SAMPLE_LIST_H_INCLUDED

#include <stdlib.h>
#include "rfid_types.h"

typedef void * LIST_HANDLE;

#define INVALID_LIST_HANDLE NULL

/******************************************************************************
 * Name: ListCreate
 *
 * Description:
 *   Creates a new List.
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   INVALID_LIST_HANDLE if creation failed, non-NULL otherwise.
 ******************************************************************************/
LIST_HANDLE ListCreate(
     void
     );

/******************************************************************************
 * Name: ListAddFront
 *
 * Description:
 *   Adds an item to the front of a list
 *
 * Parameters:
 *   list - list to which item will be added to front
 *   pValue - the item to add to the list
 *
 * Returns:
 *   0 - success, !0 - failure
 ******************************************************************************/
INT32S ListAddFront(
    LIST_HANDLE list,
    void*       pValue
    );

/******************************************************************************
 * Name: ListAddEnd
 *
 * Description:
 *   Adds an item to the end of a list
 *
 * Parameters:
 *   list - list to which item will be added to end
 *   pValue - the item to add to the list
 *
 * Returns:
 *   0 - success, !0 - failure
 ******************************************************************************/
INT32S ListAddEnd(
    LIST_HANDLE list,
    void*       pValue
    );

/******************************************************************************
 * Name: ListRemoveFront
 *
 * Description:
 *   Removes the item from the front of the list
 *
 * Parameters:
 *   list - list from which item will be removed
 *
 * Returns:
 *   The value at the front of the List.
 ******************************************************************************/
void* ListRemoveFront(
    LIST_HANDLE list
    );

/******************************************************************************
 * Name: ListRemoveEnd
 *
 * Description:
 *   Removes the item from the end of the list
 *
 * Parameters:
 *   list - list from which item will be removed
 *
 * Returns:
 *   The value at the end of the List.
 ******************************************************************************/
void* ListRemoveEnd(
    LIST_HANDLE list
    );

/******************************************************************************
 * Name: ListFront
 *
 * Description:
 *   Returns (but doesn't remove) the item at the front of the list
 *
 * Parameters:
 *   list - list from which the front item will be returned
 *
 * Returns:
 *   The value at the front of the List.
 ******************************************************************************/
void* ListFront(
    LIST_HANDLE list
    );

/******************************************************************************
 * Name: ListEnd
 *
 * Description:
 *   Returns (but doesn't remove) the item at the end of the list
 *
 * Parameters:
 *   list - list from which the end item will be returned
 *
 * Returns:
 *   The value at the end of the List.
 ******************************************************************************/
void* ListEnd(
    LIST_HANDLE list
    );

/******************************************************************************
 * Name: ListEmpty
 *
 * Description:
 *   Determines if the list is empty
 *
 * Parameters:
 *   list - list to test for emptiness
 *
 * Returns:
 *   0 - not empty, !0 - empty
 ******************************************************************************/
int ListEmpty(
    LIST_HANDLE list
    );

/******************************************************************************
 * Name: ListDestroy
 *
 * Description:
 *   Destroys the list
 *
 * Parameters:
 *   list - list to destroy
 *
 * Returns:
 *   Nothing
 ******************************************************************************/
void ListDestroy(
    LIST_HANDLE list
    );

#endif /* #ifndef SAMPLE_LIST_H_INCLUDED */
