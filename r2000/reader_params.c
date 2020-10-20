
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include "rfid_library.h"
#include "network.h"
#include "r2000.h"
#include "byte_swap.h"
#include "oemcfg.h"


//#define OEMCFG_USBD_32BIT_LEN           40
//#define OEMCFG_USBD_16BIT_LEN           OEMCFG_USBD_32BIT_LEN * 2
//#define OEMCFGADDR_PBA_NUM_32BIT_LEN    4


//typedef struct
//{
//	INT8U   byteLength;
//	INT8U   usbDescriptorType;
//	INT16U  name[OEMCFG_USBD_16BIT_LEN - 1];
//} USB_STRING_DESCRIPTOR;


RFID_STATUS                 status;
//RFID_RADIO_ENUM*            pEnum;
//RFID_RADIO_HANDLE           handle;
RFID_RADIO_POWER_STATE		pstate;
INT32U                      antenna;
RFID_ANTENNA_PORT_STATUS    antennaStatus;
RFID_ANTENNA_PORT_CONFIG    antennaConfig;
RFID_VERSION				macVersion;
RFID_18K6C_TAG_GROUP		pGroup;
void*						pRegionConfig;
OEMCFG_AREA_MAP				oemConfig;


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

	double power = 5.0;

	/* Get the state of each antenna.  For enabled antennas, get the configuration for that antenna.  */
	for (antenna = 1; antenna < 5; ++antenna)
	{
		antennaStatus.length = sizeof(RFID_ANTENNA_PORT_STATUS);
		status = RFID_AntennaPortGetStatus(handle, antenna, &antennaStatus);
		if (RFID_STATUS_OK != status)
		{
			break;
		}
		if (RFID_ANTENNA_PORT_STATE_DISABLED == antennaStatus.state)
		{
			break;
		}
		else {
			printf("Antenna #%d:\n", antenna);
			antennaConfig.length = sizeof(RFID_ANTENNA_PORT_CONFIG);
			status = RFID_AntennaPortGetConfiguration(handle, antenna, &antennaConfig);
			printf("\tPower: %d dBm\n", antennaConfig.powerLevel);
			if (RFID_STATUS_OK != status)
			{
				fprintf(stderr,
					"ERROR: RFID_AntennaPortGetConfiguration returned 0x%.8x\n",
					status);
				exit(1);
			}
			//power = antennaConfig.powerLevel / 10.0f;
			power = antennaConfig.powerLevel;
			printf("\tPower Level: %.1f dBm\n", power);
		}
		
	}
	return power;
}

int setAntennaPower(RFID_RADIO_HANDLE handle, double power) {

	/* Get the state of each antenna.  For enabled antennas, get the configuration for that antenna.  */
	for (antenna = 1; antenna < 5; ++antenna)
	{
		antennaStatus.length = sizeof(RFID_ANTENNA_PORT_STATUS);
		status = RFID_AntennaPortGetStatus(handle, antenna, &antennaStatus);
		if (RFID_STATUS_OK != status)
		{
			fprintf(stderr,
				"ERROR: RFID_STATUS 0x%.8x\n",
				status);
			exit(1);
		}
		if (RFID_ANTENNA_PORT_STATE_DISABLED == antennaStatus.state)
		{
			break;
		}
		else {
			antennaConfig.length = sizeof(RFID_ANTENNA_PORT_CONFIG);
			status = RFID_AntennaPortGetConfiguration(handle, antenna, &antennaConfig);

			if (RFID_STATUS_OK != status)
			{
				fprintf(stderr,
					"ERROR: RFID_AntennaPortGetConfiguration returned 0x%.8x\n",
					status);
				exit(1);
			}
			printf("Antenna #%d:\n", antenna);
			printf("\tPower Level: %.1f dBm\n", power);
			int pow = power * 10;
			//printf("\tPow: %d dBm\n", pow);
			antennaConfig.powerLevel = pow;
			status = RFID_AntennaPortSetConfiguration(handle, antenna, &antennaConfig);
			//printf("\tPower Level: %.1f dBm\n", antennaConfig.powerLevel);
		}

	}

	
}


void getConnectedAntennaPorts(RFID_RADIO_HANDLE handle, char ant[4]) {

	char sAnt[4];

	memset(sAnt, '\0', sizeof(sAnt));
	memset(ant, '\0', sizeof(ant));
	for (antenna = 1; antenna < 5; ++antenna)
	{
		antennaStatus.length = sizeof(RFID_ANTENNA_PORT_STATUS);
		status = RFID_AntennaPortGetStatus(handle, antenna, &antennaStatus);
		if (RFID_STATUS_OK != status)
		{
			break;
		}

		if (RFID_ANTENNA_PORT_STATE_DISABLED == antennaStatus.state)
		{
			continue;
		}
		else {
			printf("Antenna %d\n", antenna);
			sprintf(sAnt, "%d", antenna);
			strcat(ant, sAnt);
		}
	}
}

void getEnabledAntena(RFID_RADIO_HANDLE handle, char ant[4]) {
	
	for (antenna = 1; antenna < 5; ++antenna)
	{
		antennaStatus.length = sizeof(RFID_ANTENNA_PORT_STATUS);
		status = RFID_AntennaPortGetStatus(handle, antenna, &antennaStatus);
		
		if (RFID_STATUS_OK != status)
		{
			break;
		}

		if (RFID_ANTENNA_PORT_STATE_DISABLED == antennaStatus.state)
		{
			ant[antenna-1] = "0";
			continue;
		}
		else {
			printf("Antenna #%d\n", antenna);
			ant[antenna-1] = antenna;
			printf("#%d\n", ant[antenna-1]);
		}
	}
	for (int i = 0; i < 4; i++) {
		printf("%s\n", &ant[i]);
	}
}

void setSelectedAntena(RFID_RADIO_HANDLE handle, char *nuevoDato) {
	//INT32U value = atoi(nuevoDato);
	int conectadas[4];
	int numElementos = 0;
	INT32U value = 0;
	char* p;
	int j = 0;

	memset(conectadas, '\0', sizeof(conectadas));

	if (nuevoDato == NULL) {
		for (int i = 1; i < 5; i++)
		{
			antennaStatus.length = sizeof(RFID_ANTENNA_PORT_STATUS);
			status = RFID_AntennaPortGetStatus(handle, i, &antennaStatus);
			status = RFID_AntennaPortSetState(handle, i, RFID_ANTENNA_PORT_STATE_DISABLED);
			status = RFID_AntennaPortSetConfiguration(handle, i, &antennaConfig);
		}
	} else {
		if (strlen(nuevoDato) == 1) {
			value = atoi(nuevoDato);
			conectadas[j] = value;
			numElementos = 1;
		}
		else {
			for (p = strtok(nuevoDato, " "); p; p = strtok(NULL, " ")) {
				value = atoi(p);

				if (value == 0) {}
				else {
					conectadas[j] = value;
					j++;
					numElementos++;
				}
			}
		}

		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < numElementos; j++)
			{
				//printf("i: %d j: %d, conectadas[j]: %d\n", i, j, conectadas[j]);
				if (conectadas[j] == i) {
					//setEnabledAntena(handle, nuevoDato);
					antennaStatus.length = sizeof(RFID_ANTENNA_PORT_STATUS);
					status = RFID_AntennaPortGetStatus(handle, i, &antennaStatus);
					if (RFID_ANTENNA_PORT_STATE_DISABLED == antennaStatus.state) {
						//printf("ENABLED\n");
						status = RFID_AntennaPortSetState(handle, i, RFID_ANTENNA_PORT_STATE_ENABLED);
						status = RFID_AntennaPortSetConfiguration(handle, i, &antennaConfig);
					}
					break;
				}
				else {
					//printf("DISABLED\n");
					status = RFID_AntennaPortSetState(handle, i, RFID_ANTENNA_PORT_STATE_DISABLED);
					status = RFID_AntennaPortSetConfiguration(handle, i, &antennaConfig);
				}
			}
		}
	}
	memset(conectadas, '\0', sizeof(conectadas));
}



void ConvertUsbString(
	char*           destination,
	const INT16U*   source,
	INT32U          length
)
{
	/* Cheap way to do Unicode to ASCII - this assumes that the Unicode string*/
	/* really just represents an ASCII string and we are simply removing the  */
	/* upper nibble (which is 0x00).                                          */
	while (length && (*source != 0))
	{
		length--;
		*destination++ = (char)*source++;
	}

} /* ConvertUsbString */

//void OemConfigMacToHost(
//	OEMCFG_AREA_MAP* pConfig
//)
//{
//
//	USB_STRING_DESCRIPTOR*   pManufacturerName;
//	USB_STRING_DESCRIPTOR*   pProductName;
//	USB_STRING_DESCRIPTOR*   pSerialNumber;
//
//	/* Convert the 32-bit integers up to the manufacturer name first and then */
//	/* convert the 32-bit integers after the serial number through the end.   */
//	Int32ArrayMacToHost(
//		(INT32U *)pConfig,
//		((INT32U *)&pConfig->mfg_name) - ((INT32U *)pConfig));
//	Int32ArrayMacToHost(
//		&pConfig->ant_avail,
//		((INT32U *)(pConfig + 1)) - &pConfig->ant_avail);
//
//	/* Convert the USB Unicode strings to host format.                        */
//	pManufacturerName = (USB_STRING_DESCRIPTOR*)pConfig->mfg_name;
//	pProductName = (USB_STRING_DESCRIPTOR*)pConfig->prod_name;
//	pSerialNumber = (USB_STRING_DESCRIPTOR*)pConfig->serial_num;
//	Int16ArrayMacToHost(
//		pManufacturerName->name,
//		(pManufacturerName->byteLength - 2) / 2);
//	Int16ArrayMacToHost(
//		pProductName->name,
//		(pProductName->byteLength - 2) / 2);
//	Int16ArrayMacToHost(
//		pSerialNumber->name,
//		(pSerialNumber->byteLength - 2) / 2);
//} /* OemConfigMacToHost */

void Int32ArrayMacToHost(
	INT32U*         pBegin,
	INT32U          numberInt32
)
{
	for (; numberInt32; --numberInt32, ++pBegin)
	{
		*pBegin = MacToHost32(*pBegin);
	}
} /* Int32ArrayMacToHost */

void Int16ArrayMacToHost(
	INT16U*         pBegin,
	INT32U          numberInt16
)
{
	for (; numberInt16; --numberInt16, ++pBegin)
	{
		*pBegin = MacToHost16(*pBegin);
	}
}

void getReaderInfo(RFID_RADIO_HANDLE handle, char inf[9]) {

	//char						tempString[OEMCFG_USBD_32BIT_LEN * 4];
	//char*						regionString;
	/*INT32U						numToRead = ((INT32U)&oemConfig.hw_options4 - (INT32U)&oemConfig) / sizeof(INT32U);
	USB_STRING_DESCRIPTOR*		pManufacturerName;
	USB_STRING_DESCRIPTOR*		pProductName;
	USB_STRING_DESCRIPTOR*		pSerialNumber;*/

	char version[15];

	/* Attempt to get the MAC version for the radio */
	if (RFID_STATUS_OK != (status = RFID_MacGetVersion(handle, &macVersion)))
	{
		printf("RFID_MacGetVersion failed: RC = %d\n", status);
	}
	else
	{
		sprintf(inf, "%d.%d.%d.%d", macVersion.major, macVersion.minor, macVersion.maintenance, macVersion.release);
	}

	/*pRegion = (RFID_MAC_REGION *)malloc(sizeof(RFID_MAC_REGION));
	pRegionConfig = NULL;
	status = RFID_MacGetRegion(handle, pRegion, pRegionConfig);*/

	//UINT32 currentLinkProfile = 0;


	//char manu[20];
	//char product[20];
	//char serial[40];

	///* Read the OEM configuration area and then convert it to the host-native */
	///* format.  Note, if we were going to write this back to the MAC, then    */
	///* we would have to convert it to MAC-native format before writing it.    */
	//status = RFID_MacReadOemData(handle, 0, &numToRead, (INT32U *)&oemConfig);
	//if (RFID_STATUS_OK != status)
	//{
	//	fprintf(stderr, "ERROR: RFID_MacReadOemData returned 0x%.8x\n", status);
	//}
	//OemConfigMacToHost(&oemConfig);
	//pManufacturerName = (USB_STRING_DESCRIPTOR*)oemConfig.mfg_name;
	//ConvertUsbString(tempString, pManufacturerName->name, (pManufacturerName->byteLength - 2) / sizeof(INT16U));
	////printf("Manufacturer:\t\t\t%.*s\n", (pManufacturerName->byteLength - 2) / sizeof(INT16U), tempString);
	//sprintf(manu, "%.*s", (pManufacturerName->byteLength - 2) / sizeof(INT16U), tempString);
	////printf("Manu: %s\n", manu);

	//pProductName = (USB_STRING_DESCRIPTOR*)oemConfig.prod_name;
	//ConvertUsbString(tempString, pProductName->name, (pProductName->byteLength - 2) / sizeof(INT16U));
	////printf("Product Name:\t\t\t%.*s\n", (pProductName->byteLength - 2) / sizeof(INT16U), tempString);
	//sprintf(product, "%.*s", (pProductName->byteLength - 2) / sizeof(INT16U), tempString);
	////printf("Prod: %s\n", product);

	////pSerialNumber = (USB_STRING_DESCRIPTOR*)oemConfig.serial_num;
	////ConvertUsbString(tempString, pSerialNumber->name, (pSerialNumber->byteLength - 2) / sizeof(INT16U));
	////printf("Serial Number:\t\t\t%.*s\n", (pSerialNumber->byteLength - 2) / sizeof(INT16U), tempString);
	////sprintf(serial, "%.*s", (pProductName->byteLength - 2) / sizeof(INT16U), tempString);
	////printf("serial: %s\n", serial);

	////printf("RSSI Threshod:\t\t\t%d dBm\n", oemConfig.rssi_threshold);

	//sprintf(inf, "%s\n%s\n%s\n", version, manu, product);



	///* Clear out the oemConfig structure, since we only load/read a fraction of it */
	//memset(&oemConfig, 0, sizeof(oemConfig));

}

void getAdvancedOptions(RFID_RADIO_HANDLE handle, char inf[40]) 
{
	//INT32U	numToRead = ((INT32U)&oemConfig.hw_options4 - (INT32U)&oemConfig) / sizeof(INT32U);
	//status = RFID_MacReadOemData(handle, 0, &numToRead, (INT32U *)&oemConfig);
	//if (RFID_STATUS_OK != status)
	//{
	//	fprintf(stderr, "ERROR: RFID_MacReadOemData returned 0x%.8x\n", status);
	//}
	//OemConfigMacToHost(&oemConfig);
	memset(inf, 0, sizeof(inf));

	char region[2];
	char sesion[2];
	char target[2];
	RFID_MAC_REGION* pRegion;

	status = RFID_18K6CGetQueryTagGroup(handle, &pGroup);

	//RFID_18K6C_INVENTORY_SESSION session = pGroup.session;
	INT32U ses = pGroup.session;
	sprintf(sesion, "%d", ses);
	printf("sesion %s\n", sesion);
	
	//RFID_18K6C_INVENTORY_SESSION_TARGET target = pGroup.target;
	INT32U tar = pGroup.target;
	sprintf(target, "%d", pGroup.target);
	printf("target %s\n", target);

	pRegion = (RFID_MAC_REGION *)malloc(sizeof(RFID_MAC_REGION));
	pRegionConfig = NULL;
	status = RFID_MacGetRegion(handle, pRegion, pRegionConfig);
	INT32U reg = pRegion[0];
	sprintf(region, "%d", reg);
	printf("region %s\n", region);

	/* Moved to static decl's to avoid warnings on inititializer for algorithmSettings */
	 RFID_18K6C_SINGULATION_FIXEDQ_PARMS             fixedQParms;
	 RFID_18K6C_SINGULATION_DYNAMICQ_PARMS           dynamicQParms;
	 RFID_18K6C_SINGULATION_ALGORITHM                algorithm;


	

	 /* First get the original fixedq singulation parameters and the current   */
/* singulation algorithm selection, so they can be restored when we are   */
/* done                                                                   */
	 fixedQParms.length = sizeof(RFID_18K6C_SINGULATION_FIXEDQ_PARMS);
	 status = RFID_18K6CGetSingulationAlgorithmParameters(
		 handle,
		 RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ,
		 &fixedQParms);
	 if (RFID_STATUS_OK != status)
	 {
		 fprintf(
			 stderr,
			 "ERROR: RFID_18K6CGetSingulationAlgorithmParameters returned 0x%.8x\n",
			 status);
	 }
	 /* Get the current singulation algorithm        */
	 status = RFID_18K6CGetCurrentSingulationAlgorithm(
		 handle,
		 &algorithm);

	 printf("Current Singulation Algorithm: %s (%u)\n\n",
		 (RFID_18K6C_SINGULATION_ALGORITHM_DYNAMICQ == algorithm) ?
		 "Dynamic Q" : "Fixed Q", algorithm);


	 
	/*if (0 == oemConfig.regulatory_region)
	{
		regionString = "FCC";
	}
	else if (1 == oemConfig.regulatory_region)
	{
		regionString = "ETSI";
	}
	else if (2 == oemConfig.regulatory_region)
	{
		regionString = "ETSI(LBT)";
	}
	else
	{
		regionString = "???";
	}
	printf("Regulatory Region: %s\n", regionString);
*/
	sprintf(inf, "%s\n%s\n%s\n", sesion, target, region);
	//printf("INFO %s\n", inf);
}

void setAdvancedOptions(RFID_RADIO_HANDLE handle, char msg[20], char inf[15]) {
	INT32U	numToRead = ((INT32U)&oemConfig.hw_options4 - (INT32U)&oemConfig) / sizeof(INT32U);
	if (strcmp(msg, "SET_REGION") == 0) {
		printf("Option %s\n", "SET_REGION");
		if (strncmp(inf, "FCC", 3) == 0) {
			oemConfig.regulatory_region = 0;
		}
		else if (strncmp(inf, "ETSI", 4) == 0)
		{
			oemConfig.regulatory_region = 1;
		}
		else if (strncmp(inf, "ETSI(LBT)", 8) == 0)
		{
			oemConfig.regulatory_region = 2;
		}
		RFID_MacWriteOemData(handle, 0, &numToRead, (INT32U *)&oemConfig);

	} 
	else if (strcmp(msg, "SET_TARI") == 0) {
		printf("Option %s\n", "SET_TARI");

	}
	else if (strcmp(msg, "SET_BLF") == 0) {
		printf("Option %s\n", "SET_BLF");

	}
	else if (strcmp(msg, "SET_M") == 0) {
		printf("Option %s\n", "SET_M");

	}
	else if (strcmp(msg, "SET_Q") == 0) {
		printf("Option %s\n", "SET_Q");

	}
	else if (strcmp(msg, "SET_SESSION") == 0) {
		printf("Option %s\n", "SET_SESSION");
		status = RFID_18K6CGetQueryTagGroup(handle, &pGroup);

		if (strncmp(inf, "0", 1) == 0) {
			pGroup.session = 0;
		}
		else
		{
			pGroup.session = 1;
		}
		status = RFID_18K6CSetQueryTagGroup(handle, &pGroup);

	}
	else if (strcmp(msg, "SET_TARGET") == 0) {
		printf("Option %s\n", "SET_TARGET");
		status = RFID_18K6CGetQueryTagGroup(handle, &pGroup);
		if (strncmp(inf, "A", 1) == 0) {
			pGroup.target = 0;
		}
		else
		{
			pGroup.target = 1;
		}
		status = RFID_18K6CSetQueryTagGroup(handle, &pGroup);

	}
}
