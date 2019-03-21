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
 * $Id: auto_lock.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description:
 *     This header presents a template class that can be used to allow for
 *     automatically obtaining a lock upon construction and ensuring that locks
 *     are released upon destruction (even in the face of exceptions).
 *     
 *
 *****************************************************************************
 */

#ifndef AUTO_LOCK_H_INCLUDED
#define AUTO_LOCK_H_INCLUDED

namespace rfid
{

////////////////////////////////////////////////////////////////////////////////
// Name: LockWaitException
//
// Description: An exception to indicate that the lock failed.
////////////////////////////////////////////////////////////////////////////////
class LockWaitException
{
};

////////////////////////////////////////////////////////////////////////////////
// Name: AutoLock
//
// Description: A class that can be used to wrap a lock for the purpose of
//     automatically acquire the lock and ensuring that when the wrapper goes
//     out of scope, it will automatically release the lock.  This is especially
//     useful for ensuring that if the code leaves a scope (for example, in the
//     case of an exception), the lock is released.
//
//     The class also allows for the lock to be transferred - i.e., the object
//     will not release the lock destruction (this is useful for when you
//     want the lock to remain locked after the object goes out of scope).
//
// Template Parameters:
//      HandleType - the type of lock handle which is being wrapped
//      Traits - the traits for the handle
////////////////////////////////////////////////////////////////////////////////
template <typename HandleType, typename Traits = LockTraits<HandleType> >
class AutoLock
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Name:        AutoLock
    // Description: Initializes an AutoLock object that doesn't initially own a
    //              lock.  Assume() needs to be used to make this object take
    //              ownership of a lock
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    AutoLock() :
        m_handle(Traits::InvalidHandle())
    {
    } // AutoLock::AutoLock

    ////////////////////////////////////////////////////////////////////////////
    // Name:        AutoLock
    // Description: Initializes an AutoLock object and automatically obtains the
    //              lock provided.  The lock is released when this object is
    //              destroyed.
    // Parameters:  handle - the lock to obtain and own
    // Returns:     Nothing
    //
    // Throws LockWaitException if the lock fails.
    ////////////////////////////////////////////////////////////////////////////
    explicit AutoLock(
        HandleType handle
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        AutoLock (copy constructor)
    // Description: Initializes an AutoLock object by transferring ownership of
    //              the lock from the AutoLock object passed in to this one.
    //              The lock will only be released when this object is
    //              destroyed.
    // Parameters:  other - the AutoLock from which control will be assumed.
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    AutoLock(
        AutoLock<HandleType, Traits>& other
        ) :
        m_handle(other.Transfer())
    {
    } // AutoLock::AutoLock

    ////////////////////////////////////////////////////////////////////////////
    // Name:        ~AutoLock
    // Description: Destroys the AutoLock object, releasing the lock if it owns
    //              it.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    ~AutoLock()
    {
        this->Cleanup();
    } // AutoLock::~AutoLock

    ////////////////////////////////////////////////////////////////////////////
    // Name:        operator = 
    // Description: Transfers the ownership of the lock from other to this
    //              object.  If this object already owns a lock, it releases it.
    // Parameters:  other - the AutoLock object that will transfer its lock
    //              ownership to this object
    // Returns:     this object
    ////////////////////////////////////////////////////////////////////////////
    AutoLock<HandleType, Traits>& operator = (
        AutoLock<HandleType, Traits>& other
        );

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Transfer
    // Description: Causes the object to relinquish ownership the lock.  The
    //              underlying lock is not released - it is the responsibility
    //              of the caller to ensure that the lock is unlocked.
    // Parameters:  None
    // Returns:     The raw lock handle for the lock that ownership is being
    //              relinquished.
    ////////////////////////////////////////////////////////////////////////////
    HandleType Transfer();

    ////////////////////////////////////////////////////////////////////////////
    // Name:        Assume
    // Description: Causes the object to assume ownership (and therefore auto-
    //              matically release) the lock provided.  It is assumed that
    //              the underlying lock is already locked and the intention is
    //              that the object now has the responsibility of unlocking it.
    //              If the object already owns a lock, that lock will be
    //              released before assuming control of the lock passed in.
    // Parameters:  handle - the handle to the lock whose ownership will be
    //              assumed
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Assume(
        HandleType handle
        );

private:
    // The handle to the raw lock that the object owns
    HandleType m_handle;
    
    ////////////////////////////////////////////////////////////////////////////
    // Name:        Cleanup
    // Description: Causes the object to release (i.e., unlock) the underlying
    //              lock.
    // Parameters:  None
    // Returns:     Nothing
    ////////////////////////////////////////////////////////////////////////////
    void Cleanup();
}; // class AutoLock

////////////////////////////////////////////////////////////////////////////////
// Name:        AutoLock::AutoLock
// Description: Initializes an AutoLock object and automatically obtains the
//              lock provided.  The lock is released when this object is
//              destroyed.
////////////////////////////////////////////////////////////////////////////////
template <typename HandleType, typename Traits>
AutoLock<HandleType, Traits>::AutoLock(
    HandleType handle
    ) : m_handle(handle)
{
    // Verify that the lock handle is valid, and if so, then attempt the lock
    if ((Traits::InvalidHandle() != m_handle) &&
        !Traits::Lock(m_handle))
    {
        throw LockWaitException();
    }
} // AutoLock::AutoLock

////////////////////////////////////////////////////////////////////////////////
// Name:        AutoLock::operator = 
// Description: Transfers the ownership of the lock from other to this
//              object.  If this object already owns a lock, it releases it.
////////////////////////////////////////////////////////////////////////////////
template <typename HandleType, typename Traits>
AutoLock<HandleType, Traits>& AutoLock<HandleType, Traits>::operator = (
    AutoLock<HandleType, Traits>& other
    )
{
    // Only copy if different handles
    if (m_handle != other.m_handle)
    {
        this->Cleanup();
        m_handle = other.Transfer();
    }

    return *this;
} // AutoLock::operator =

////////////////////////////////////////////////////////////////////////////////
// Name:        AutoLock::Transfer
// Description: Causes the object to relinquish ownership of the lock.  The
//              underlying lock is not released - it is the responsibility
//              of the caller to ensure that the lock is unlocked.
////////////////////////////////////////////////////////////////////////////////
template <typename HandleType, typename Traits>
HandleType AutoLock<HandleType, Traits>::Transfer()
{
    HandleType handle = m_handle;
    m_handle = Traits::InvalidHandle();
    return handle;
} // AutoLock::Transfer

////////////////////////////////////////////////////////////////////////////
// Name:        AutoLock::Assume
// Description: Causes the object to assume ownership (and therefore auto-
//              matically release) the lock provided.  It is assumed that
//              the underlying lock is already locked and the intention is
//              that the object now has the responsibility of unlocking it.
//              If the object already owns a lock, that lock will be
//              released before assuming control of the lock passed in.
////////////////////////////////////////////////////////////////////////////
template <typename HandleType, typename Traits>
void AutoLock<HandleType, Traits>::Assume(
    HandleType handle
    )
{
    // Only do some real work if the handles are different
    if (m_handle != handle)
    {
        // Relinquish the lock currently held, and assume ownership of the
        // lock passed in
        this->Cleanup();
        m_handle = handle;
    }
} // AutoLock::Assume

////////////////////////////////////////////////////////////////////////////
// Name:        AutoLock::Cleanup
// Description: Causes the object to release (i.e., unlock) the underlying
//              lock.
////////////////////////////////////////////////////////////////////////////
template <typename HandleType, typename Traits>
void AutoLock<HandleType, Traits>::Cleanup()
{
    try
    {
        // If we have a valid lock handle, then unlock it
        if (Traits::InvalidHandle() != m_handle)
        {
            Traits::Unlock(m_handle);
            m_handle = Traits::InvalidHandle();
        }
    }
    catch (...)
    {
        // Intentionally left blank
    }
} // AutoLock::Cleanup

} // namespace rfid

#endif // AUTO_LOCK_H_INCLUDED
