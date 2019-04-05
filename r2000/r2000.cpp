/*
 *****************************************************************************
 * Author: Larraitz Orio
 * Date: 2019
 *****************************************************************************
 */


#include "pch.h"

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <iostream>
#include "rfid_library.h"
#include "network.h"
#include "r2000.h"
#include "reader_params.h"


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
	RFID_RADIO_POWER_STATE		pstate;
	INT32U                      index;
	INT32U                      antenna;
	RFID_ANTENNA_PORT_STATUS    antennaStatus;
	RFID_ANTENNA_PORT_CONFIG    antennaConfig;

	RFID_RADIO_OPERATION_MODE	pmode;
	RFID_RADIO_LINK_PROFILE		linkProfile;
	


	RFID_UNREFERENCED_LOCAL(argc);
	RFID_UNREFERENCED_LOCAL(argv);

	//Servidor socket
	WSADATA WSAData;
	SOCKET server, client, client2;
	//SOCKADDR_IN  clientAddr;
	struct sockaddr_in clientAddr, clientAddr2;
	char buffer[15];
	char msg[15];
	int conectado = 0;
	int preparado = 0;
	int clientAddrSize = sizeof(clientAddr);
	int clientAddrSize2 = sizeof(clientAddr2);
	int retval;
	char version[15];

	double power = 0.0;



	/* Initialialize the RFID library                                         */
	status = RFID_Startup(NULL, 0);
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



	 

	status = RFID_RadioGetOperationMode(handle, &pmode);
	//printf("\tRadioHandle used: %u", handle);
	printf("PowerStat found: %u\n ", pmode);

	UINT32 currentLinkProfile = 0;


	status = RFID_RadioGetLinkProfile(handle, currentLinkProfile, &linkProfile);
	printf("Length: %u\n ", linkProfile.length);
	printf("profileId: %u\n ", linkProfile.profileId);
	printf("profileVersion: %u \n", linkProfile.profileVersion);
	printf(" iso length: %u\n ", linkProfile.profileConfig.iso18K6C.tari);
	//printf("Length: %u ", linkProfile.length);
	//printf("Length: %u ", linkProfile.length);
	//printf("Length: %u ", linkProfile.length);
	//printf("Length: %u ", linkProfile.length);




	///* COMUNICACIÓN SOCKET CON EL SOFTWARE MYRUNS */
	//server = configure_tcp_socket(5557);

	//if ((client = accept(server, (struct sockaddr*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET)
	//{
	//	printf("Client connected!\n");
	//	recvfrom(client, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &clientAddrSize);
	//	printf("Client says: %s\n", buffer);
	//	if (strncmp(buffer, "CONNECT", 7) == 0)
	//	{
	//		conectado = 1;
	//		printf("CONECTADO AL READER\n");
	//		memset(buffer, 0, sizeof(buffer));
	//		send(client, "READY", 5, 0);
	//	}
	//}
	//server = configure_tcp_socket(5558);
	//if ((client = accept(server, (struct sockaddr*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET)
	//{
	//	printf("Client connected!\n");
	//	preparado = 1;
	//}
	//if (conectado == 1 && preparado == 1)
	//{
	//	while (conectado == 1) {
	//		status = RFID_RadioGetPowerState(handle, &pstate);


	//		retval = recvfrom(client, msg, sizeof(msg), 0, (struct sockaddr*)&clientAddr, &clientAddrSize);
	//		printf("%s\n", msg);
			if (strncmp(msg, "DISCONNECT", 10) == 0) {
				conectado = 0;
			}
			if (strncmp(msg, "POWER_MINMAX", 12) == 0)
			{
				printf("%s\n", msg);
			} else if (strcmp(msg, "GET_POWER") == 0) {
				power = getAntennaPower(handle);
				//printf("%s\n", msg);
			}
			else if (strcmp(msg, "SET_POWER") == 0) {
				setAntennaPower(handle, 1, 100);
			} 
			else if (strcmp(msg, "GET_INFO") == 0) {
				char info[22];

				getReaderInfo(handle, info);
				printf("Version: %s\n", info);

			}
			memset(msg, 0, sizeof(msg));
	//	}

	//	closesocket(client);
	//	//closesocket(client2);
	//	printf("Client disconnected!\n");
	//}

} /* main */
