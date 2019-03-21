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
 * $Id: compat_error.h 65946 2010-11-08 00:41:15Z dshaheen $
 * 
 * Description: Defines the error-handling functions and the portable error
 *     codes.
 *
 *****************************************************************************
 */

#ifndef COMPAT_ERROR_
#define COMPAT_ERROR_

//#include <winsock2.h>

#include <winerror.h>
#include <windows.h>


#include "compat_lib.h"

enum {
    CPL_SUCCESS = NO_ERROR,
    CPL_ERROR_ACCESSDENIED = ERROR_ACCESS_DENIED,
    CPL_ERROR_BUFFERTOOSMALL = ERROR_INSUFFICIENT_BUFFER,
    CPL_ERROR_BUSY = ERROR_BUSY,
    CPL_ERROR_DEVICEFAILURE = ERROR_ADAP_HDW_ERR,
    CPL_ERROR_DEVICEGONE = ERROR_DEVICE_NOT_CONNECTED,
    CPL_ERROR_EXISTS = ERROR_FILE_EXISTS,
    CPL_ERROR_INVALID = ERROR_INVALID_DATA,
    CPL_ERROR_NOMEMORY = ERROR_OUTOFMEMORY,
    CPL_ERROR_NOTFOUND = ERROR_FILE_NOT_FOUND,
    CPL_ERROR_INVALID_FUNC = ERROR_INVALID_FUNCTION,
    CPL_ERROR_TOOMUCHDATA = 0xDA7A2816,
    CPL_ERROR_RXOVERFLOW  = 0xDA7A2817,
    CPL_WARN_CANCELLED = 0xCA2CE1D,
    CPL_WARN_ENDOFLIST = 0x32D0F17,
    CPL_WARN_NEWDEVICE = 0x15EE02E,
    CPL_WARN_TIMEOUT = WAIT_TIMEOUT,
    CPL_WARN_WOULDBLOCK = WAIT_TIMEOUT,

    // network & socket specific errors

    CPL_ENET_INTERRUPT	            = WSAEINTR,
    CPL_ENET_BAD_FILE	            = WSAEBADF,
    CPL_ENET_ACCESS	                = WSAEACCES,
    CPL_ENET_FAULT	                = WSAEFAULT,
    CPL_ENET_INVAL	                = WSAEINVAL,
    CPL_ENET_TOO_MANY_FILES	        = WSAEMFILE,
    CPL_ENET_WOULD_BLOCK	        = WSAEWOULDBLOCK,
    CPL_ENET_IN_PROGRESS	        = WSAEINPROGRESS,
    CPL_ENET_ALREADY	            = WSAEALREADY,
    CPL_ENET_NOT_SOCK	            = WSAENOTSOCK,	
    CPL_ENET_DEST_ADDR_REQUIRED	    = WSAEDESTADDRREQ,
    CPL_ENET_MSG_SIZE	            = WSAEMSGSIZE,
    CPL_ENET_PROTO	                = WSAEPROTOTYPE,
    CPL_ENET_PROTO_OPTION	        = WSAENOPROTOOPT,	
    CPL_ENET_PROTO_NOSUPPORT	    = WSAEPROTONOSUPPORT,
    CPL_ENET_SOCK_NOSUPPORT	        = WSAESOCKTNOSUPPORT,
    CPL_ENET_OP_NOSUPPORT	        = WSAEOPNOTSUPP,
    CPL_ENET_PF_NOSUPPORT	        = WSAEPFNOSUPPORT,
    CPL_ENET_AF_NOSUPPORT	        = WSAEAFNOSUPPORT,
    CPL_ENET_ADDR_INUSE	            = WSAEADDRINUSE,
    CPL_ENET_ADDR_NOTAVAILABLE	    = WSAEADDRNOTAVAIL,
    CPL_ENET_DOWN	                = WSAENETDOWN,
    CPL_ENET_NET_UNREACHABLE	    = WSAENETUNREACH,
    CPL_ENET_NET_RESET	            = WSAENETRESET,
    CPL_ENET_CONN_ABORTED	        = WSAECONNABORTED,
    CPL_ENET_NO_BUFFS	            = WSAENOBUFS,
    CPL_ENET_IS_CONNECTED	        = WSAEISCONN,
    CPL_ENET_NOT_CONNECTED	        = WSAENOTCONN,
    CPL_ENET_SHUTDOWN	            = WSAESHUTDOWN,
    CPL_ENET_TOO_MANY_REFS	        = WSAETOOMANYREFS,
    CPL_ENET_TIMED_OUT	            = WSAETIMEDOUT,
    CPL_ENET_CONN_REFUSED	        = WSAECONNREFUSED,
    CPL_ENET_LOOP	                = WSAELOOP,
    CPL_ENET_NAME_TOO_LONG	        = WSAENAMETOOLONG,
    CPL_ENET_HOST_DOWN	            = WSAEHOSTDOWN,
    CPL_ENET_HOST_UNREACHABLE	    = WSAEHOSTUNREACH,
    CPL_ENET_HOST_NOT_FOUND	        = WSAHOST_NOT_FOUND,
    CPL_ENET_HOST_TRY_AGAIN	        = WSATRY_AGAIN,
    CPL_ENET_HOST_NO_RECOVERY	    = WSANO_RECOVERY,
    CPL_ENET_HOST_NO_DATA	        = WSANO_DATA

};

/****************************************************************************
 *  Function:    CPL_GetError
 *  Description: Returns the most recent system-level error for this thread.
 *  Parameters:  None.
 *  Returns:     The most recent error code.
 ****************************************************************************/
inline INT32U CPL_GetError(void) { return GetLastError(); }

/****************************************************************************
 *  Function:    CPL_SetError
 *  Description: Sets the system-level error code for the current thread.
 *  Parameters:  errCode   [IN] The value to set the error to.
 *  Returns:     Its argument, for convenience.
 ****************************************************************************/
inline INT32U CPL_SetError(INT32U err) { SetLastError(err); return err;}

#endif /* COMPAT_ERROR_ */
