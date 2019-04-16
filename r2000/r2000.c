/*
 *****************************************************************************
 * Author: Larraitz Orio
 * Date: 2019
 *****************************************************************************
 */



#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include "rfid_library.h"

#include "network.h"
#include "r2000.h"
#include "reader_params.h"

#include "byte_swap.h"
#include "oemcfg.h"

#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "rfid.lib")

#define OEMCFG_USBD_32BIT_LEN           40
#define OEMCFG_USBD_16BIT_LEN           OEMCFG_USBD_32BIT_LEN * 2
#define OEMCFGADDR_PBA_NUM_32BIT_LEN    4


typedef struct
{
	INT8U   byteLength;
	INT8U   usbDescriptorType;
	INT16U  name[OEMCFG_USBD_16BIT_LEN - 1];
} USB_STRING_DESCRIPTOR;



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

void OemConfigMacToHost(
	OEMCFG_AREA_MAP* pConfig
)
{

	USB_STRING_DESCRIPTOR*   pManufacturerName;
	USB_STRING_DESCRIPTOR*   pProductName;
	USB_STRING_DESCRIPTOR*   pSerialNumber;

	/* Convert the 32-bit integers up to the manufacturer name first and then */
	/* convert the 32-bit integers after the serial number through the end.   */
	Int32ArrayMacToHost(
		(INT32U *)pConfig,
		((INT32U *)&pConfig->mfg_name) - ((INT32U *)pConfig));
	Int32ArrayMacToHost(
		&pConfig->ant_avail,
		((INT32U *)(pConfig + 1)) - &pConfig->ant_avail);

	/* Convert the USB Unicode strings to host format.                        */
	pManufacturerName = (USB_STRING_DESCRIPTOR*)pConfig->mfg_name;
	pProductName = (USB_STRING_DESCRIPTOR*)pConfig->prod_name;
	pSerialNumber = (USB_STRING_DESCRIPTOR*)pConfig->serial_num;
	Int16ArrayMacToHost(
		pManufacturerName->name,
		(pManufacturerName->byteLength - 2) / 2);
	Int16ArrayMacToHost(
		pProductName->name,
		(pProductName->byteLength - 2) / 2);
	Int16ArrayMacToHost(
		pSerialNumber->name,
		(pSerialNumber->byteLength - 2) / 2);
} /* OemConfigMacToHost */

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



int main(
	int     argc,
	char**  argv
)
{
	RFID_STATUS                 status;
	RFID_RADIO_ENUM*            pEnum;
	RFID_RADIO_HANDLE           handle;
	RFID_RADIO_POWER_STATE		pstate;
	RFID_RADIO_INFO*			pInfo;
	INT32U                      index; 
	INT32U                      antenna;
	RFID_VERSION				version;
	RFID_ANTENNA_PORT_STATUS    antennaStatus;
	RFID_ANTENNA_PORT_CONFIG    antennaConfig;
	INT32						antennaPort;
	RFID_18K6C_TAG_GROUP		pGroup;

	RFID_RADIO_OPERATION_MODE	pmode;
	RFID_RADIO_LINK_PROFILE		linkProfile;
	
	RFID_MAC_REGION*			pRegion;
	void*						pRegionConfig;

	OEMCFG_AREA_MAP          oemConfig;
	char                     tempString[OEMCFG_USBD_32BIT_LEN * 4];
	INT32U                   numToRead = ((INT32U)&oemConfig.hw_options4 - (INT32U)&oemConfig) / sizeof(INT32U);
	char*                    regionString;
	USB_STRING_DESCRIPTOR*   pManufacturerName;
	USB_STRING_DESCRIPTOR*   pProductName;
	USB_STRING_DESCRIPTOR*   pSerialNumber;


	RFID_UNREFERENCED_LOCAL(argc);
	RFID_UNREFERENCED_LOCAL(argv);

	//Servidor socket
	WSADATA WSAData;
	SOCKET server, client, client2;
	//SOCKADDR_IN  clientAddr;
	struct sockaddr_in clientAddr, clientAddr2;
	char buffer[10];
	char msg[20];
	int conectado = 0;
	int preparado = 0;
	int clientAddrSize = sizeof(clientAddr);
	int clientAddrSize2 = sizeof(clientAddr2);
	int retval;
	//char version[15];

	double power = 0.0;
	char* nuevo[20];



	/* Initialialize the RFID library                                         */
	status = RFID_Startup(&version, 0);
	if (RFID_STATUS_OK != status)
	{
		fprintf(stderr, "ERROR: RFID_Startup returned 0x%.8x\n", status);
		return 0;;
	}

	/* Create an initial structure for enumerating the radios.  We'll adjust  */
	/* it once we know how big to make it.                                    */
	pEnum = (RFID_RADIO_ENUM *)malloc(sizeof(RFID_RADIO_ENUM));
	if (NULL == pEnum)
	{
		fprintf(stderr, "ERROR: Failed to allocate memory\n");
		RFID_Shutdown();
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
			free(pEnum);
		}
		pEnum = pNewEnum;
	}
	if (RFID_STATUS_OK != status)
	{
		fprintf(
			stderr,
			"ERROR: RFID_RetrieveAttachedRadiosList returned 0x%.8x\n",
			status);
		free(pEnum);
	}

	/* Now do something with radio list */ 
	for (INT32U index = 0; index < pEnum->countRadios; ++index) {
		pInfo = pEnum->ppRadioInfo[index];
		INT8U* a = pInfo->pUniqueId;
	/* Do something useful with the radio */ }

	/* Open up the first radio - if more than one radio is attached, so be it */
	if (!pEnum->countRadios)
	{
		fprintf(stderr, "ERROR: No radios attached to the system\n");
		free(pEnum);
	}
	status = RFID_RadioOpen(pEnum->ppRadioInfo[0]->cookie, &handle, 0);
	if (RFID_STATUS_OK != status)
	{
		fprintf(stderr, "ERROR: RFID_RadioOpen returned 0x%.8x\n", status);
		free(pEnum);
	}


	pRegion = (RFID_MAC_REGION *)malloc(sizeof(RFID_MAC_REGION));
	pRegionConfig = NULL;
	status = RFID_MacGetRegion(handle, pRegion, pRegionConfig);


	/* HABILITAR ANTENAS*/
	/*
	antennaPort = 1;
	status = RFID_AntennaPortGetStatus(handle, antennaPort, &antennaStatus);
	//antennaStatus.state = RFID_ANTENNA_PORT_STATE_ENABLED;
	status = RFID_AntennaPortSetState(handle, antennaPort, RFID_ANTENNA_PORT_STATE_ENABLED);*/


	/*
 	status = RFID_RadioGetOperationMode(handle, &pmode);
	//printf("\tRadioHandle used: %u", handle);
	printf("PowerStat found: %u\n ", pmode); */

	UINT32 currentLinkProfile = 0;


	//status = RFID_RadioGetLinkProfile(handle, currentLinkProfile, &linkProfile);
	//printf("Length: %u\n ", linkProfile.length);
	//printf("profileId: %u\n ", linkProfile.profileId);
	//printf("profileVersion: %u \n", linkProfile.profileVersion);
	//printf(" iso length: %u\n ", linkProfile.profileConfig.iso18K6C.tari);
	//printf("Length: %u ", linkProfile.length);
	//printf("Length: %u ", linkProfile.length);
	//printf("Length: %u ", linkProfile.length);
	//printf("Length: %u ", linkProfile.length);



	/* Clear out the oemConfig structure, since we only load/read a fraction of it */
	memset(&oemConfig, 0, sizeof(oemConfig));

	/* Read the OEM configuration area and then convert it to the host-native */
	/* format.  Note, if we were going to write this back to the MAC, then    */
	/* we would have to convert it to MAC-native format before writing it.    */
	status = RFID_MacReadOemData( handle, 0,	&numToRead, (INT32U *)&oemConfig);
	if (RFID_STATUS_OK != status)
	{
		fprintf(stderr, "ERROR: RFID_MacReadOemData returned 0x%.8x\n", status);
	}
	OemConfigMacToHost(&oemConfig);
	pManufacturerName = (USB_STRING_DESCRIPTOR*)oemConfig.mfg_name;
	ConvertUsbString( tempString, pManufacturerName->name, (pManufacturerName->byteLength - 2) / sizeof(INT16U));
	printf(	"Manufacturer:\t\t\t%.*s\n", (pManufacturerName->byteLength - 2) / sizeof(INT16U), tempString);

	pProductName = (USB_STRING_DESCRIPTOR*)oemConfig.prod_name;
	ConvertUsbString(tempString, pProductName->name, (pProductName->byteLength - 2) / sizeof(INT16U));
	printf("Product Name:\t\t\t%.*s\n", (pProductName->byteLength - 2) / sizeof(INT16U), tempString);

	pSerialNumber = (USB_STRING_DESCRIPTOR*)oemConfig.serial_num;
	ConvertUsbString(tempString, pSerialNumber->name, (pSerialNumber->byteLength - 2) / sizeof(INT16U));
	printf("Serial Number:\t\t\t%.*s\n", (pSerialNumber->byteLength - 2) / sizeof(INT16U), tempString);
	printf("RSSI Threshod:\t\t\t%d dBm\n", oemConfig.rssi_threshold);
	if (0 == oemConfig.regulatory_region)
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
	printf("Regulatory Region:\t\t%s\n", regionString);





	/* COMUNICACIÓN SOCKET CON EL SOFTWARE MYRUNS */
	server = configure_tcp_socket(5557);

	if ((client = accept(server, (struct sockaddr*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET)
	{
		printf("Client connected!\n");
		//recvfrom(client, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &clientAddrSize);
		retval = recv(client, buffer, sizeof(buffer), 0);
		printf("Client says: %s\n", buffer);
		if (strncmp(buffer, "CONNECT", 7) == 0)
		{
			conectado = 1;
			printf("CONECTADO AL READER\n");
			memset(buffer, 0, sizeof(buffer));
			send(client, "READY", 5, 0);
		}
		memset(buffer, 0, sizeof(buffer));
	}
	server = configure_tcp_socket(5558);
	if ((client = accept(server, (struct sockaddr*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET)
	{
		printf("Client connected!\n");
		preparado = 1;
	}
	if (conectado == 1 && preparado == 1)
	{
		while (conectado == 1) {
			/*retval = recvfrom(client, msg, sizeof(msg), 0, (struct sockaddr*)&clientAddr, &clientAddrSize);*/
			retval = recv(client, msg, sizeof(msg), 0);
			//nuevo[0] = '\0';
			//if (strstr(msg, "#") == NULL) {
			//	
			//	strcat(nuevo, msg);
			//	//strcat(append, str2);
			//	printf("append: %s\n", nuevo);
			//}
			//else {
			//	strcat(nuevo, msg);
			//	nuevo[strlen(nuevo) - 1] = 0;
			//	printf("msg: %s\n", nuevo);
			//}
			//printf("recv()'d %d bytes of data in buf\n", retval);
			//printf("msg: %s\n", msg);
			if (strncmp(msg, "DISCONNECT", 10) == 0) {
				conectado = 0;
			}
			if (strncmp(msg, "POWER_MINMAX", 12) == 0)
			{ 
				printf("%s\n", msg);
			} else if (strncmp(msg, "GET_POWER", 9) == 0) {
				power = getAntennaPower(handle);
				printf("DENTRO DE POWER\n");
				//send(client, "READY", 5, 0);
			} 
			else if (strcmp(msg, "SET_POWER") == 0) {
				setAntennaPower(handle, 1, 100);
			} 
			else if (strcmp(msg, "GET_INFO") == 0) {
				char info[15];

				sprintf(info, "%d.%d.%d.%d", version.major, version.minor, version.maintenance, version.release);
				printf("Version: %s\n", info);

				memset(&oemConfig, 0, sizeof(oemConfig));
				status = RFID_MacReadOemData(handle, 0, &numToRead, (INT32U *)&oemConfig);
				if (RFID_STATUS_OK != status)
				{
					fprintf(stderr, "ERROR: RFID_MacReadOemData returned 0x%.8x\n", status);
				}
				OemConfigMacToHost(&oemConfig);
				pManufacturerName = (USB_STRING_DESCRIPTOR*)oemConfig.mfg_name;
				ConvertUsbString(tempString, pManufacturerName->name, (pManufacturerName->byteLength - 2) / sizeof(INT16U));
				printf("Manufacturer:\t\t\t%.*s\n", (pManufacturerName->byteLength - 2) / sizeof(INT16U), tempString);

			}
			else if (strcmp(msg, "GET_ADV_OPT") == 0) {
				status = RFID_18K6CGetQueryTagGroup(handle, &pGroup);

				RFID_18K6C_INVENTORY_SESSION session = pGroup.session;
				RFID_18K6C_INVENTORY_SESSION_TARGET target = pGroup.target;

			}
			else if (strcmp(msg, "CON_ANT_PORTS") == 0) { 
				//mandar el numero de antena y guardarlo en antenna Port
				int numAntenas = 0;
				for (int antPort = 0; antPort < numAntenas; antPort++)
				{
					status = RFID_AntennaPortGetStatus(handle, antPort, &antennaStatus);
					status = RFID_AntennaPortSetState(handle, antPort, RFID_ANTENNA_PORT_STATE_ENABLED);
				}
			}
			
			memset(msg, 0, sizeof(msg));
		}

		closesocket(client);
		//closesocket(client2);
		printf("Client disconnected!\n");
		WSACleanup();
	}

} /* main */

