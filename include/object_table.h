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
 * $Id: object_table.h 60276 2009-10-19 18:21:43Z dshaheen $
 * 
 * Description:
 *     This header presents a template class that can be used to store object
 *     pointers and receive handles in return.
 *     
 *
 *****************************************************************************
 */

#ifndef OBJECT_TABLE_H_INCLUDED
#define OBJECT_TABLE_H_INCLUDED

#include <vector>
#include <stdexcept>
#include "rfid_platform_types.h"

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: ObjectTable
//
// Description: This class is used to hold object pointers.  Upon insertion it
//     will issue a handle that can be used for subsequent retrieval.  The table
//     does NOT provide cleanup of the objects.
//
// Template Parameters:
//      Object - the type of objects that will have pointers stored in this
//        table
////////////////////////////////////////////////////////////////////////////////
template <typename Object>
class ObjectTable
{
public:
    // A useful typedef for table object handles
    typedef INT32U TableHandle;

    enum {
        // A suitable default size for the object table
        DEFAULT_INITIAL_SIZE = 8,
        // A suitable default size to grow the table when inserting and it is
        // full
        DEFAULT_GROW_SIZE    = 4
    };

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ObjectTable
    // Description: Initializes the object table.
    // Parameters:  initialSize - the initial size of the table.  Picking a
    //              value that is reasonable prevents unnecessary table
    //              resizing.
    //              growSize - the number of new entries to add to the table
    //              when it is necessary to grow the table.  Must be greater
    //              than zero.
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    ObjectTable(
        INT16U initialSize = DEFAULT_INITIAL_SIZE,
        INT16U growSize    = DEFAULT_GROW_SIZE
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Add
    // Description: Adds a new object to the table.  If the table is full, it
    //              will be enlarged to accommodate the new object pointer.
    // Parameters:  pObject - a pointer to the object to add
    // Returns:     A handle that can be used for subsequent retrieval of the
    //              object pointer.
    ////////////////////////////////////////////////////////////////////////////
    TableHandle Add(
        Object* pObject
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Get
    // Description: Retrieves an object pointer from the table
    // Parameters:  handle - the handle, received from a previous call to Add,
    //              of the object pointer to retrieve
    // Returns:     The object pointer, or NULL if the handle doesn't reference
    //              a valid object.
    ////////////////////////////////////////////////////////////////////////////
    Object* Get(
        TableHandle handle
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Remove
    // Description: Removes an object pointer from the table.  Note that removal
    //              does not cause the object to be deleted.  It is the
    //              responsibility of the caller to ensure that the object is
    //              cleaned up.
    // Parameters:  handle - the handle, received from a previous call to Add,
    //              of the object pointer to remove
    // Returns:     The object pointer, or NULL if the handle doesn't reference
    //              a valid object.
    ////////////////////////////////////////////////////////////////////////////
    Object* Remove(
        TableHandle handle
        );

    // These are the return codes for the application-defined callback.  See
    // the declaration of FunctionCallback for a description of their meanings.
    typedef enum
    {
        CALLBACK_STATUS_CONTINUE            = 0,
        CALLBACK_STATUS_REMOVE_AND_CONTINUE = 1,
        CALLBACK_STATUS_ABORT               = 2,
        CALLBACK_STATUS_REMOVE_AND_ABORT    = 3
    } CALLBACK_STATUS;

    ////////////////////////////////////////////////////////////////////////////
    // Name:        FunctionCallback
    // Description: A caller-defined function that can be used for callbacks
    //              from the object table (for example, when iterating over all
    //              entries in the table).
    // Parameters:  handle - the handle, received from a previous call to Add,
    //              of the object pointer provided in the callback
    //              pObject - the pointer to the object
    //              context - application-supplied context for the callback
    // Returns:     CALLBACK_STATUS_CONTINUE - continue executing the function
    //              that is invoking the callback
    //              CALLBACK_STATUS_REMOVE_AND_CONTINUE - remove the object
    //              pointer referenced by handle from the table (does not delete
    //              the object - that is the application's responsibility) and
    //              continue executing the function that is invoking the
    //              callback
    //              CALLBACK_STATUS_ABORT - stop executing the function that is
    //              invoking the callback
    //              CALLBACK_STATUS_REMOVE_AND_ABORT - remove the object
    //              pointer referenced by handle from the table (does not delete
    //              the object - that is the application's responsibility) and
    //              stop executing the function that is invoking the callback
    ////////////////////////////////////////////////////////////////////////////
    typedef CALLBACK_STATUS (* FunctionCallback)(
        TableHandle handle,
        Object*     pObject,
        INT64U      context
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ForEach
    // Description: A request to invoke the specified callback function for
    //              each entry in the table.  The application can prematurely
    //              abort the iteration using the return code from the callback.
    // Parameters:  pCallback - the function to invoke upon the table entry
    //              context - application-supplied context which is passed
    //              unchanged to the callback function
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void ForEach(
        FunctionCallback pCallback,
        INT64U           context
        );

private:
    enum {
        // A signature that will never be used for a table entry
        INVALID_SIGNATURE = 0
    };

    ////////////////////////////////////////////////////////////////////////////
    // Name: ObjectEntry
    //
    // Description: An entry in the table for an object pointer
    ////////////////////////////////////////////////////////////////////////////
    typedef struct ObjectEntry
    {
        ////////////////////////////////////////////////////////////////////////
        // Name: ObjectEntry
        // Description: Initializes an object table entry
        // Parameters:  signature - the signature for the entry.  This helps to
        //              detect when an application is using a stale handle.
        //              nextFree - the index of the next free table entry.
        //              pObject - the object pointer to be held in the entry
        // Returns:     Nothing
        ////////////////////////////////////////////////////////////////////////
        ObjectEntry(
            INT16U  signature = INVALID_SIGNATURE,
            INT16U  nextFree  = 0,
            Object* pObject   = NULL
            ) :
            m_signature(signature),
            m_nextFree(nextFree),
            m_pObject(pObject),
            m_used(false)
        {
        } // ObjectEntry::ObjectEntry

        // The table entry's signature
        INT16U  m_signature;
        // The index of the next free entry.  This is only useful if the entry
        // is currently unused.
        INT16U  m_nextFree;
        // A pointer to the object for the table entry
        Object* m_pObject;
        // A flag to indicate if the table entry is in use or not
        bool    m_used;
    } ObjectEntry;

    // The underlying data structure that holds the table entries
    std::vector<ObjectEntry>    m_objectTable;
    // The first free entry in the table
    INT16U                      m_firstFree;
    // The number of free entries in the table
    INT16U                      m_numberFree;
    // The amount to grow the table on resize
    INT16U                      m_growSize;

    ////////////////////////////////////////////////////////////////////////////
    // Name:        TableHandleIsValid
    // Description: Determines if the table handle references a valid entry in
    //              the table.  Validity is determined by checking to see if the
    //              index is within the table, the signature in the handle
    //              matches the signature in the table entry, and the entry is
    //              in use.
    // Parameters:  handle - the handle to check for validity
    // Returns:     true - handle is valid.  false - handle is invalid.
    ////////////////////////////////////////////////////////////////////////////
    bool TableHandleIsValid(
        INT32U handle
        ) const;

// Useful macros for dealing with handles
// CREATE_HANDLE - takes a signature (s) and a table index (i) and creates a
// table handle
#define CREATE_HANDLE(s, i)  ((((INT32U) (0xFFFF & (s))) << 16) | \
                              ((i) & 0xFFFF))
// EXTRACT_SIGNATURE - takes a handle (h) and extracts the signature
#define EXTRACT_SIGNATURE(h) ((INT16U) (((h) >> 16) & 0xFFFF))
// EXTRACT_INDEX - takes a handle (h) and extracts the table index
#define EXTRACT_INDEX(h)     ((INT16U) ((h) & 0xFFFF))
}; // class ObjectTable

////////////////////////////////////////////////////////////////////////////////
// Name:        ObjectTable
// Description: Initializes the object table.
////////////////////////////////////////////////////////////////////////////////
template <typename Object>
ObjectTable<Object>::ObjectTable(
    INT16U initialSize,
    INT16U growSize
    ) :
    m_objectTable(initialSize),
    m_firstFree(0),
    m_numberFree(initialSize),
    m_growSize(growSize)
{
    // Verify that the grow size > 0
    if (!growSize)
    {
        throw std::invalid_argument("Object table grow size == 0");
    }

    // Go through and initialize the entries; next free fields "link"
    // together the table entries
    for (INT16U index = 0; index < initialSize; ++index)
    {
        m_objectTable[index].m_nextFree = index + 1;
    }
} // ObjectTable::ObjectTable

////////////////////////////////////////////////////////////////////////////////
// Name:        Add
// Description: Adds a new object to the table.  If the table is full, it
//              will be enlarged to accommodate the new object pointer.
////////////////////////////////////////////////////////////////////////////////
template <typename Object>
typename ObjectTable<Object>::TableHandle ObjectTable<Object>::Add(
    Object* pObject
    )
{
    // If the table is full, then we need to expand it
    if (!m_numberFree)
    {
        INT16U oldSize = (INT16U) m_objectTable.size();
        INT16U newSize = oldSize + m_growSize;

        // Reserve space for the new entries
        m_objectTable.reserve(newSize);

        // Create and initialize the new entries
        for (INT16U index = oldSize; index < newSize; ++index)
        {
            m_objectTable.push_back(
                ObjectEntry(INVALID_SIGNATURE, index + 1, NULL));
        }

        m_firstFree     = oldSize;
        m_numberFree    = m_growSize;
    }

    // Take the first free entry for this object
    INT16U index = m_firstFree;

    // Fix up the first and number free
    m_firstFree = m_objectTable[index].m_nextFree;
    --m_numberFree;

    // Fix the table entry
    ++(m_objectTable[index].m_signature);
    if (INVALID_SIGNATURE == m_objectTable[index].m_signature)
    {
        ++(m_objectTable[index].m_signature);
    }
    m_objectTable[index].m_pObject = pObject;
    m_objectTable[index].m_used    = true;

    return CREATE_HANDLE(m_objectTable[index].m_signature, index);
} // ObjectTable::Add

////////////////////////////////////////////////////////////////////////////////
// Name:        Get
// Description: Retrieves an object pointer from the table
////////////////////////////////////////////////////////////////////////////////
template <typename Object>
Object* ObjectTable<Object>::Get(
    TableHandle handle
    )
{
    return (this->TableHandleIsValid(handle) ?
            m_objectTable[EXTRACT_INDEX(handle)].m_pObject : NULL);
} // ObjectTable::Get

////////////////////////////////////////////////////////////////////////////////
// Name:        Remove
// Description: Removes an object pointer from the table.  Note that removal
//              does not cause the object to be deleted.  It is the
//              responsibility of the caller to ensure that the object is
//              cleaned up.
////////////////////////////////////////////////////////////////////////////////
template <typename Object>
Object* ObjectTable<Object>::Remove(
    TableHandle handle
    )
{
    Object* pObject   = NULL;

    if (this->TableHandleIsValid(handle))
    {
        INT16U index = EXTRACT_INDEX(handle);

        // Mark the entry as unused
        m_objectTable[index].m_used = false;

        // Get object pointer for caller
        pObject = m_objectTable[index].m_pObject;

        // Fix up the free entry list
        m_objectTable[index].m_nextFree = m_firstFree;
        m_firstFree                     = index;
        ++m_numberFree;
    }

    return pObject;
} // ObjectTable::Remove

////////////////////////////////////////////////////////////////////////////////
// Name:        ForEach
// Description: A request to invoke the specified callback function for
//              each entry in the table.  The application can prematurely
//              abort the iteration using the return code from the callback.
////////////////////////////////////////////////////////////////////////////////
template <typename Object>
void ObjectTable<Object>::ForEach(
    FunctionCallback pCallback,
    INT64U           context
    )
{
    INT16U index;

    // For each used entry, invoke the callback
    for (index = 0; index < m_objectTable.size(); ++index)
    {
        if (m_objectTable[index].m_used)
        {
            INT32U handle = CREATE_HANDLE(m_objectTable[index].m_signature,
                                          index);
            CALLBACK_STATUS status = 
                pCallback(handle, m_objectTable[index].m_pObject, context);

            // If we are supposed to remove the object from the table, do so
            if ((CALLBACK_STATUS_REMOVE_AND_CONTINUE == status) ||
                (CALLBACK_STATUS_REMOVE_AND_ABORT    == status))
            {
                this->Remove(handle);
            }

            // If we are supposed to abort, do so
            if ((CALLBACK_STATUS_ABORT            == status) ||
                (CALLBACK_STATUS_REMOVE_AND_ABORT == status))
            {
                break;
            }
        }
    }
} // ObjectTable::ForEach

////////////////////////////////////////////////////////////////////////////////
// Name:        TableHandleIsValid
// Description: Determines if the table handle references a valid entry in
//              the table.  Validity is determined by checking to see if the
//              index is within the table, the signature in the handle
//              matches the signature in the table entry, and the entry is
//              in use.
////////////////////////////////////////////////////////////////////////////////
template <typename Object>
bool ObjectTable<Object>::TableHandleIsValid(
    TableHandle handle
    ) const
{
    // Get the signature and table index from the handle
    INT16U  signature = EXTRACT_SIGNATURE(handle);
    INT16U  index     = EXTRACT_INDEX(handle);

    // A table handle has the following properties:
    // - the index is within the table
    // - the table entry is used
    // - the signature matches the signature in the entry
    return ((index < m_objectTable.size()) &&
            m_objectTable[index].m_used    &&
            (m_objectTable[index].m_signature == signature));
} // ObjectTable::TableHandleIsValid

} // namespace rfid

#endif // #ifndef OBJECT_TABLE_H_INCLUDED
