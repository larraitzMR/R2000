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
 * $Id: sample_stack.h 59754 2009-09-08 17:07:45Z dshaheen $
 * 
 * Description:
 *     This file contains the prototype for a simple stack.
 *     
 *     NOTE - this is not bulletproof code, but is instead a simple stack for
 *     demonstrating nonvolatile memory update.
 *     
 *
 *****************************************************************************
 */

#ifndef SAMPLE_STACK_H_INCLUDED
#define SAMPLE_STACK_H_INCLUDED

#include <stdlib.h>
#include "rfid_types.h"

typedef void * STACK_HANDLE;

#define INVALID_STACK_HANDLE NULL

/******************************************************************************
 * Name: StackCreate
 *
 * Description:
 *   Creates a new stack.
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   INVALID_STACK_HANDLE if creation failed, non-NULL otherwise.
 ******************************************************************************/
STACK_HANDLE StackCreate(
     void
     );

/******************************************************************************
 * Name: StackPush
 *
 * Description:
 *   Pushes an item on the stack
 *
 * Parameters:
 *   stack - stack onto which item will be pushed
 *   pValue - the item to push onto the stack
 *
 * Returns:
 *   0 - success, !0 - failure
 ******************************************************************************/
INT32S StackPush(
    STACK_HANDLE    stack,
    void*           pValue
    );

/******************************************************************************
 * Name: StackPop
 *
 * Description:
 *   Pops the top item from the stack
 *
 * Parameters:
 *   stack - stack from which item will be popped
 *
 * Returns:
 *   The value on top of the stack.
 ******************************************************************************/
void* StackPop(
    STACK_HANDLE    stack
    );

/******************************************************************************
 * Name: StackTop
 *
 * Description:
 *   Returns (but doesn't pop) the top item from the stack
 *
 * Parameters:
 *   stack - stack from which the to item will be returned
 *
 * Returns:
 *   The value on top of the stack.
 ******************************************************************************/
void* StackTop(
    STACK_HANDLE    stack
    );

/******************************************************************************
 * Name: StackEmpty
 *
 * Description:
 *   Determines if the stack is empty
 *
 * Parameters:
 *   stack - stack to test for emptiness
 *
 * Returns:
 *   0 - not empty, !0 - empty
 ******************************************************************************/
int StackEmpty(
    STACK_HANDLE    stack
    );

/******************************************************************************
 * Name: StackDestroy
 *
 * Description:
 *   Destroys the stack
 *
 * Parameters:
 *   stack - stack to destroy
 *
 * Returns:
 *   Nothing
 ******************************************************************************/
void StackDestroy(
    STACK_HANDLE    stack
    );

#endif /* #ifndef SAMPLE_STACK_H_INCLUDED */
