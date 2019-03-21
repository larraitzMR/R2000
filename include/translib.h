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
 * $Id: translib.h 65946 2010-11-08 00:41:15Z dshaheen $
 * 
 * Description:
 *     Public definitions for the RFID Transport Library 
 *     
 *
 *****************************************************************************
 */


#ifndef TRANSLIB_H_
#define TRANSLIB_H_

#include "oswrappers.h"
#include <compat_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TransHandle is what we return for Radio handles */
typedef INT32U TransHandle, *PTransHandle;

/* Library status codes */
typedef INT32U TransStatus, *PTransStatus;


#define RTENUM_CONTEXT_SIZE     sizeof(INT32U)  /* keep synch'd with internal structure */

/* Opaque Context supplied to library for radio enumaration */
typedef struct RadioEnumContext_t  {
	unsigned char pvt[RTENUM_CONTEXT_SIZE];
} RadioEnumContext, *PRadioEnumContext;


/* The individual Radio Enumeration Actions */

typedef enum EnumAction_t {
EA_FirstRadio,	 /* This action will start an enumeration sequence.      */
EA_SameRadio,	 /* This action allows additional calls to be made for   */
                 /*   the same radio. This is useful for determining the */
                 /*   exact size needed to hold the radio serial number  */
                 /*   before getting the serial number.                  */
EA_NextRadio	 /* This action advances the enumeration sequence to the */
                 /*  next radio.*/
}EnumAction, *PEnumAction;

/* IDs to identify various transports */
typedef enum TransportID_t{
    TPID_UNDEFINED, /* transport is unknown */
    TPID_USB,	    /* Transport is USB.  */
    TPID_SERIAL,	/* Transport is a serial-based protocol */
} TransportID, *PTransportID;

/* A common version structure */
typedef struct TransportVersion_t {
	INT32U	major;
	INT32U	minor;
	INT32U	maintenance;
	INT32U	release;
} TransportVersion;
typedef const TransportVersion *PTransportVersion;


/* the individual characteristics of each transport instance */
typedef struct TransportCharacteristics_t {
	INT32U 		     structSize;		/* size of this structure */
	TransportID 	 transportID;		/* ID for this particular transport */
	TransportVersion transportVersion;	/* Version number for transport driver */
	INT32U  		 maxTransfer;		/* the maximum size transfer that can be accommodated */
	INT32U  		 maxPacket;		    /* the maximum packet size used on the "wire" */
	/*
	    Transport-specific characteristics extend common structure
	*/
} TransportCharacteristics, *PTransportCharacteristics;

/*===========================================================================
    Function Prototypes
============================================================================*/

TRANSLIBAPI
PTransportVersion
RfTrans_GetVersion(
        void
        );

        
TRANSLIBAPI
TransStatus
RfTrans_EnumerateRadios(
			TransHandle      *radioHandle,
            INT8U            *serialNumberBuffer,
            INT32U           *bufferSize,
            RadioEnumContext *context,
            EnumAction       enumAction
            );


TRANSLIBAPI
TransStatus
RfTrans_GetRadioTransportCharacteristics(
			TransHandle              radioHandle,
			TransportCharacteristics *characteristicsBuf,
			INT32U                   *bufferSize
			);


TRANSLIBAPI
TransStatus
RfTrans_OpenRadio(
		TransHandle     radioHandle
		);

TRANSLIBAPI
TransStatus
RfTrans_CloseRadio(
		TransHandle     radioHandle
		);

TRANSLIBAPI
TransStatus
RfTrans_TransportMgmt(
		TransHandle radioHandle,
		INT32U      transportCommandCode,
		INT8U       *commandResponseBuffer,
		INT32U      *bufferLength
		);


TRANSLIBAPI
TransStatus
RfTrans_WriteRadio(
		TransHandle radioHandle,
		INT8U       *dataBuffer,
		INT32U      dataBufferSize
		);
 

TRANSLIBAPI
TransStatus
RfTrans_ReadRadio(
		TransHandle radioHandle,
		INT8U       *dataBuffer,
		INT32U      *dataBufferSize,
		INT32U      *remaining,
        INT32U      flags
		);
#define RFFLAG_ANY_DATA_OK  1
		

TRANSLIBAPI
TransStatus
RfTrans_CancelRadio(
		TransHandle radioHandle,
		INT32U	    flags
		);		
#define CANCELRADIO_WRITE	0x01
#define CANCELRADIO_READ	0x02


TRANSLIBAPI
TransStatus
RfTrans_ResetRadio(
		TransHandle radioHandle,
        INT32U      resetType
		);		
#define RESETRADIO_SOFT	    0x00
#define RESETRADIO_TO_BL	0x01


TRANSLIBAPI
TransStatus
RfTrans_AbortRadio(
		TransHandle radioHandle
		);		

#ifdef __cplusplus
}
#endif

#endif /* TRANSLIB_H_ */


