
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include "rfid_library.h"
#include "network.h"
#include "r2000.h"


RFID_STATUS                 status;
//RFID_RADIO_ENUM*            pEnum;
//RFID_RADIO_HANDLE           handle;
RFID_RADIO_POWER_STATE		pstate;
INT32U                      antenna;
RFID_ANTENNA_PORT_STATUS    antennaStatus;
RFID_ANTENNA_PORT_CONFIG    antennaConfig;
RFID_VERSION				macVersion;


void initializeRFID(RFID_RADIO_HANDLE handle, RFID_RADIO_ENUM* pEnum) {
	/* Initialialize the RFID library                                         */
	status = RFID_Startup(NULL, 0);
	if (RFID_STATUS_OK != status)
	{
		fprintf(stderr, "ERROR: RFID_Startup returned 0x%.8x\n", status);
	}

	/* Create an initial structure for enumerating the radios.  We'll adjust  */
	/* it once we know how big to make it.                                    */
	/*pEnum = (RFID_RADIO_ENUM *)malloc(sizeof(RFID_RADIO_ENUM));*/
	if (NULL == pEnum)
	{
		fprintf(stderr, "ERROR: Failed to allocate memory\n");
	}
	pEnum->length =
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
		fprintf(
			stderr,
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

 }

int getAntennaPower(RFID_RADIO_HANDLE handle) {

	printf("GET ANTENNA POWER\n");

	double power = 0.0;

	/* Get the state of each antenna.  For enabled antennas, get the configuration for that antenna.  */
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

		antennaConfig.length = sizeof(RFID_ANTENNA_PORT_CONFIG);
		status = RFID_AntennaPortGetConfiguration(handle, antenna, &antennaConfig);
		if (RFID_STATUS_OK != status)
		{
			fprintf(stderr,
				"ERROR: RFID_AntennaPortGetConfiguration returned 0x%.8x\n",
				status);
			exit(1);
		}
		power = (double)antennaConfig.powerLevel / 10;
		printf("\tPower Level: %.1f dBm\n", power);
	}

	return power;
}

 int setAntennaPower(RFID_RADIO_HANDLE handle, INT32U antenna, double power) {

	 printf("SET ANTENNA POWER\n");

	/* Get the state of each antenna.  For enabled antennas, get the configuration for that antenna.  */

		antennaStatus.length = sizeof(RFID_ANTENNA_PORT_STATUS);
		status = RFID_AntennaPortGetStatus(handle, antenna, &antennaStatus);
		if (RFID_STATUS_OK != status)
		{
			exit(1);
		}
		printf("Antenna #%d:\n", antenna);
		printf("\tState: %s\n",
			(antennaStatus.state == RFID_ANTENNA_PORT_STATE_DISABLED ?
				"disabled" : "enabled"));

		printf("\tLast Antenna Sense Value: %u ohms\n",
			antennaStatus.antennaSenseValue);

		antennaConfig.length = sizeof(RFID_ANTENNA_PORT_CONFIG);
		status = RFID_AntennaPortGetConfiguration(handle, antenna, &antennaConfig);
		if (RFID_STATUS_OK != status)
		{
			fprintf(stderr,
				"ERROR: RFID_AntennaPortGetConfiguration returned 0x%.8x\n",
				status);
			exit(1);
		}
 		//power = (int)antennaConfig.powerLevel * 10;
		//printf("\tPower Level: %.1f dBm\n", power);

		antennaConfig.powerLevel = power;
		status = RFID_AntennaPortSetConfiguration(handle, 0, &antennaConfig);
}

 void getReaderInfo(RFID_RADIO_HANDLE handle, char *inf) {

	 /* Attempt to get the MAC version for the radio */
	 if (RFID_STATUS_OK != (status = RFID_MacGetVersion(handle, &macVersion)))
	 {
		 printf("RFID_MacGetVersion failed: RC = %d\n", status);
	 }
	 else
	 {
		 //printf("\tMAC Version: %s\n", status);
		 //printf("\t\tMajor: %u\n", macVersion.major);
		 //printf("\t\tMinor: %u\n", macVersion.minor);
		 //printf("\t\tMaintenance: %u\n", macVersion.maintenance);
		 //printf("\t\tRelease: %u\n", macVersion.release);
	 }
	 sprintf(inf, "%d.%d.%d.%d", macVersion.major, macVersion.minor, macVersion.maintenance, macVersion.release);
	 //printf("Version: %s\n", inf);

 }