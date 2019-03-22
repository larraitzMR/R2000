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
  * $Id: antenna.c 61503 2009-12-18 01:00:57Z dshaheen $
  *
  * Description:
  *     This is a simple sample application for performing the following
  *     functions:
  *     - Start up the RFID Reader Library
  *     - Enumerate the attached radios
  *     - Open the first enumerated radio and
  *       - Retrieve the current antenna configuration for each of the
			enabled antennas
  *     - Close the radio
  *     - Shut down the library
  *****************************************************************************
  */

#include "pch.h"

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <iostream>
#include "rfid_library.h"
#include "network.h"


#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "rfid.lib")

int main(
	int     argc,
	char**  argv
)
{
	RFID_STATUS                 status;
	RFID_RADIO_ENUM*            pEnum;
	RFID_RADIO_HANDLE           handle;
	INT32U                      antenna;
	RFID_ANTENNA_PORT_STATUS    antennaStatus;
	RFID_ANTENNA_PORT_CONFIG    antennaConfig;

	RFID_UNREFERENCED_LOCAL(argc);
	RFID_UNREFERENCED_LOCAL(argv);

	/* Initialize the RFID library                                         */
	status = RFID_Startup(NULL, 0);
	if (RFID_STATUS_OK != status)
	{
		fprintf(stderr, "ERROR: RFID_Startup returned 0x%.8x\n", status);
	}

	/* Create an initial structure for enumerating the radios.  We'll adjust  */
	/* it once we know how big to make it.                                    */
	pEnum = (RFID_RADIO_ENUM *)malloc(sizeof(RFID_RADIO_ENUM));
	if (NULL == pEnum)
	{
		fprintf(stderr, "ERROR: Failed to allocate memory\n");
	}
	pEnum->length = sizeof(RFID_RADIO_ENUM);
	pEnum->totalLength = sizeof(RFID_RADIO_ENUM);

	/* Enumerate the radios                                                   */
	while (RFID_ERROR_BUFFER_TOO_SMALL ==
		(status = RFID_RetrieveAttachedRadiosList(pEnum, 0)))
	{
		RFID_RADIO_ENUM* pNewEnum =
			(RFID_RADIO_ENUM *)realloc(pEnum, pEnum->totalLength);
		if (NULL == pNewEnum)
		{
			fprintf(stderr, "ERROR: Failed to allocate memory\n");
		}
		pEnum = pNewEnum;
	}
	if (RFID_STATUS_OK != status)
	{
		fprintf(stderr,
			"ERROR: RFID_RetrieveAttachedRadiosList returned 0x%.8x\n",
			status);
	}

	/* Open up the first radio - if more than one radio is attached, so be it */
	if (!pEnum->countRadios)
	{
		fprintf(stderr, "ERROR: No radios attached to the system\n");
	}
	status = RFID_RadioOpen(pEnum->ppRadioInfo[0]->cookie, &handle, 0);
	if (RFID_STATUS_OK != status)
	{
		fprintf(stderr, "ERROR: RFID_RadioOpen returned 0x%.8x\n", status);
	}

	printf("Current antenna configuration:\n");

	/* Get the state of each antenna.  For enabled antennas, get the          */
	/* configuration for that antenna.                                        */
	for (antenna = 0; RFID_STATUS_OK == status; ++antenna)
	{
		antennaStatus.length = sizeof(RFID_ANTENNA_PORT_STATUS);
		status = RFID_AntennaPortGetStatus(handle, antenna, &antennaStatus);
		if (RFID_STATUS_OK != status)
		{
			break;
		}
		printf("Antenna #%d:\n", antenna);
		printf("\tState: %s\n",
			(antennaStatus.state == RFID_ANTENNA_PORT_STATE_DISABLED ?
				"disabled" : "enabled"));

		if (RFID_ANTENNA_PORT_STATE_DISABLED == antennaStatus.state)
		{
			continue;
		}

		printf("\tLast Antenna Sense Value: %u ohms\n",
			antennaStatus.antennaSenseValue);

		antennaConfig.length = sizeof(RFID_ANTENNA_PORT_CONFIG);
		status = RFID_AntennaPortGetConfiguration(handle, antenna, &antennaConfig);
		if (RFID_STATUS_OK != status)
		{
			fprintf(stderr,
				"ERROR: RFID_AntennaPortGetConfiguration returned 0x%.8x\n",
				status);
		}

		printf("\tPower Level: %.1f dBm\n", (double)antennaConfig.powerLevel / 10);
		printf("\tDwell Time: %u ms\n", antennaConfig.dwellTime);
		printf("\tNumber Inventory Cycles: %u\n",
			antennaConfig.numberInventoryCycles);
		printf("\tPhysical RX: %u\n", antennaConfig.physicalRxPort);
		printf("\tPhysical TX: %u\n", antennaConfig.physicalTxPort);
		printf("\tAntenna Sense Threshold (global): %u ohms\n",
			antennaConfig.antennaSenseThreshold);
	}

	/* If we are here, we broke out of loop after RFID_AntennaPortGetStatus call. */
	/* We exepect an invalid parameter error when we request an invalid antenna   */
	/* port, which will happen when the loop exhausts the set of valid antennas   */
	/* Any other error is unexpected and is reported.                             */
	if (RFID_ERROR_INVALID_PARAMETER != status)
	{
		fprintf(stderr,
			"ERROR: RFID_AntennaPortGetStatus returned 0x%.8x for antenna %d\n",
			status,
			antenna);
	}



	return 0;
} /* main */
