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
 * $Id: auto_handle.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header presents a template class that can be used to ensure that
 *     resource handles are not leaked in the face of exceptions, etc.
 *     
 *
 *****************************************************************************
 */

#ifndef AUTO_HANDLE_H_INCLUDED
#define AUTO_HANDLE_H_INCLUDED

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: AutoHandle
//
// Description: A class that can be used to wrap a handle for the purpose of
//     ensuring that when the wrapper goes out of scope, it will automatically
//     close the handle.  This is especially useful for ensuring that if the
//     code leaves a scope (for example, in the case of an exception), the
//     handle is cleaned up automatically and the resource is not leaked.
//
//     The class also allows for the handle to be transferred - i.e., the object
//     will not close the handle on destruction (this is useful for when you
//     want to open a resource and have the resource automatically cleaned up
//     if there is an error, but once the code reaches a point where an error
//     cannot occur take over ownership of the handle).
//
// Template Parameters:
//      HandleType - the type of handle which is being wrapped
//      Traits - the traits for the handle
////////////////////////////////////////////////////////////////////////////////
template <typename HandleType, typename Traits = HandleTraits<HandleType> >
class AutoHandle
{
public:

    ////////////////////////////////////////////////////////////////////////////
    // Name:        AutoHandle
    // Description: Initializes an AutoHandle that does not own a handle.
    //              Assume() needs to be used to assign a handle to the object.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    AutoHandle() :
        m_handle(Traits::InvalidHandle())
    {
    } // AutoHandle::AutoHandle

    ////////////////////////////////////////////////////////////////////////////
    // Name:        AutoHandle
    // Description: Initializes an AutoHandle object and takes ownership of the
    //              handle provided.
    // Parameters:  handle - the handle to take ownership of
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    explicit AutoHandle(
        HandleType handle
        ) :
        m_handle(handle)
    {
    } // AutoHandle::AutoHandle

    ////////////////////////////////////////////////////////////////////////////
    // Name:        AutoHandle (copy constructor)
    // Description: Initializes an AutoHandle object and transfers ownership of
    //              of the AutoHandle passed in to this object
    // Parameters:  other - the AutoHandle object from which ownership of the
    //                  underlying handle will be transferred
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    AutoHandle(
        AutoHandle<HandleType, Traits>& other
        ) :
        m_handle(other.Transfer())
    {
    } // AutoHandle::AutoHandle

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ~AutoHandle
    // Description: Cleans up an AutoHandle object, including closing the
    //              underlying handle.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    ~AutoHandle()
    {
        this->Cleanup();
    } // AutoHandle::~AutoHandle

    ////////////////////////////////////////////////////////////////////////////
    // Name:        operator =
    // Description: Assigns an AutoHandle to this object.  The results are that
    //              this object will close the underlying handle it already owns
    //              and will transfer ownership of the other AutoHandle object's
    //              handle to this object.
    // Parameters:  other - the AutoHandle that will be assigned to this object
    // Returns:     This object.
    ////////////////////////////////////////////////////////////////////////////
    AutoHandle<HandleType, Traits>& operator = (
        AutoHandle<HandleType, Traits>& other
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        operator HandleType
    // Description: Allows this object to be cast as the underlying handle type.
    // Parameters:  None
    // Returns:     The underlying handle.
    ////////////////////////////////////////////////////////////////////////////
    operator HandleType () const
    {
        return this->Get();
    } // operator HandleType

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Get
    // Description: Retrieves the underlying handle.
    // Parameters:  None
    // Returns:     The underlying handle.
    ////////////////////////////////////////////////////////////////////////////
    HandleType Get() const
    {
        return m_handle;
    } // AutoHandle::Get

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Transfer
    // Description: Forces this object to relinquish ownership of the underlying
    //              handle.  This means that when the object is destroyed, it
    //              will not close the handle.
    // Parameters:  None
    // Returns:     The underlying handle.
    ////////////////////////////////////////////////////////////////////////////
    HandleType Transfer();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Assume
    // Description: Allows the object to assume ownership of the raw handle
    //              (i.e., upon destruction will clean up the handle).  If the
    //              object already owns a handle, it will close it before
    //              assuming ownership of the new handle.
    // Parameters:  handle - the raw handle to assume ownership of.
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Assume(
        HandleType handle
        );

private:
    // The underlying handle that this object owns
    HandleType m_handle;

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Cleanup
    // Description: Closes the underlying handle
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Cleanup();
}; // class AutoHandle

////////////////////////////////////////////////////////////////////////////////
// Name:        AutoHandle::operator =
// Description: Assigns an AutoHandle to this object.  The results are that
//              this object will close the underlying handle it already owns
//              and will transfer ownership of the other AutoHandle object's
//              handle to this object.
////////////////////////////////////////////////////////////////////////////////
template <typename HandleType, typename Traits>
AutoHandle<HandleType, Traits>& AutoHandle<HandleType, Traits>::operator = (
    AutoHandle<HandleType, Traits>& other
    )
{
    // Only perform if it is a different handle
    if (m_handle != other.m_handle)
    {
        this->Cleanup();
        m_handle = other.Transfer();
    }

    return *this;
} // AutoHandle::operator =

////////////////////////////////////////////////////////////////////////////////
// Name:        AutoHandle::Transfer
// Description: Forces this object to relinquish ownership of the underlying
//              handle.  This means that when the object is destroyed, it
//              will not close the handle.
////////////////////////////////////////////////////////////////////////////////
template <typename HandleType, typename Traits>
HandleType AutoHandle<HandleType, Traits>::Transfer()
{
    HandleType handle = m_handle;
    m_handle = Traits::InvalidHandle();
    return handle;
} // AutoHandle::Transfer

////////////////////////////////////////////////////////////////////////////////
// Name:        AutoHandle::Assume
// Description: Allows the object to assume ownership of the raw handle
//              (i.e., upon destruction will clean up the handle).  If the
//              object already owns a handle, it will close it before
//              assuming ownership of the new handle.
////////////////////////////////////////////////////////////////////////////////
template <typename HandleType, typename Traits>
void AutoHandle<HandleType, Traits>::Assume(
    HandleType handle
    )
{
    // Only do some real work if the handles are different
    if (m_handle != handle)
    {
        // Clean up the resource currently holding and assume ownership of
        // the one passed in
        this->Cleanup();
        m_handle = handle;
    }
} // AutoLock::Assume

////////////////////////////////////////////////////////////////////////////////
// Name:        AutoHandle::Cleanup
// Description: Closes the underlying handle
////////////////////////////////////////////////////////////////////////////////
template <typename HandleType, typename Traits>
void AutoHandle<HandleType, Traits>::Cleanup()
{
    try
    {
        // Only bother closing the handle if it is valid
        if (Traits::InvalidHandle() != m_handle)
        {
            Traits::CloseHandle(m_handle);
            m_handle = Traits::InvalidHandle();
        }
    }
    catch (...)
    {
        // Intentionally left blank
    }
} // AutoHandle::Cleanup

} // namespace rfid

#endif // #ifndef AUTO_HANDLE_H_INCLUDED
