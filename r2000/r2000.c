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
			
			//printf("msg: %s\n", msg);
			if (strncmp(msg, "DISCONNECT", 10) == 0) {
				printf("msg: %s\n", msg);
				conectado = 0;
			}
			if (strncmp(msg, "POWER_MINMAX", 12) == 0)
			{ 
				printf("msg: %s\n", msg);
			} else if (strncmp(msg, "GET_POWER", 9) == 0) {
				printf("msg: %s\n", msg);
				char pow[6] = ""; 
				power = getAntennaPower(handle);
				printf("ENVIADO POWER: %.1f\n", power);
				//itoa(power, pow, 10);
				sprintf(pow, "%.1f", power);
				//string str = string(intStr);
				//printf("POW: %s\n", pow);
				send(client, pow, sizeof(pow), 0);
			} 
			else if (strncmp(msg, "SET_POWER", 9) == 0) {
				printf("msg: %s\n", msg);
				int longitud = strlen(msg) - 9;
				char *nuevo = (char*)malloc(sizeof(char) * (longitud + 1));
				nuevo[longitud] = '\0';
				strncpy(nuevo, msg + 9, longitud);
				//printf("NUEVO: %s\n", nuevo);
				double value = atof(nuevo);
				printf("RECIBIDO POWER: %.1f\n", value);
				setAntennaPower(handle, value);
			} 
			else if (strcmp(msg, "ANT_PORTS") == 0) {
				//printf("msg: %s\n", msg);
			}
			else if (strncmp(msg, "CON_ANT_PORTS", 13) == 0) { 
				//que antenas estan enabled
				printf("msg: %s\n", msg);
				char selAnt[4];
				getConnectedAntennaPorts(handle, selAnt);
				send(client, selAnt, 4, 0);
			}
			else if (strncmp(msg, "GET_SEL_ANT", 11) == 0) {
				printf("msg: %s\n", msg);
				int *selAnt[4];
				getConnectedAntennaPorts(handle, selAnt);
				//printf("%s", selAnt);
				send(client, selAnt, 4, 0);
			}
			else if (strncmp(msg, "SET_SEL_ANT", 11) == 0) {
				printf("msg: %s\n", msg);
				char *nuevoDato;
				int longitud = strlen(msg) - 11;
				nuevoDato = (char*)malloc(sizeof(char) * (longitud + 1));
				nuevoDato[longitud] = '\0';
				strncpy(nuevoDato, msg + 11, longitud);
				printf("CONECTADAS: %s\n", nuevoDato);
				setSelectedAntena(handle, nuevoDato);
			}
			else if (strncmp(msg, "GET_INFO", 8) == 0) {
				printf("msg: %s\n", msg);
				char info[9];
				getReaderInfo(handle, info);
				send(client, info, sizeof(info), 0);
				memset(info, 0, sizeof(info));
			}
			else if (strncmp(msg, "GET_ADV_OPT", 11) == 0) {
				printf("msg: %s\n", msg);
				char option[7];

				getAdvancedOptions(handle, option);
				send(client, option, sizeof(option), 0);
				memset(option, 0, sizeof(option));

			} 
			else if (strncmp(msg, "SET_REGION", 10) == 0) {
				printf("msg: %s\n", msg);
				int longitud = strlen(msg) - 11;
				char *nuevo = (char*)malloc(sizeof(char) * (longitud + 1));
				nuevo[longitud] = '\0';
				strncpy(nuevo, msg + 11, longitud);
				printf("REGION: %s\n", nuevo);
				fflush(stdout);

				setAdvancedOptions(handle, "SET_REGION", nuevo);

			}
			else if (strncmp(msg, "SET_TARI", 8) == 0) {
				printf("msg: %s\n", msg);
				int longitud = strlen(msg) - 9;
				char *nuevo = (char*)malloc(sizeof(char) * (longitud + 1));
				nuevo[longitud] = '\0';
				strncpy(nuevo, msg + 9, longitud);
				printf("SET TARI: %s\n", nuevo);
				fflush(stdout);
				setAdvancedOptions(handle, "SET_TARI", nuevo);

			}
			else if (strncmp(msg, "SET_BLF", 7) == 0) {
				printf("msg: %s\n", msg);
				int longitud = strlen(msg) - 8;
				char *nuevo = (char*)malloc(sizeof(char) * (longitud + 1));
				nuevo[longitud] = '\0';
				strncpy(nuevo, msg + 8, longitud);
				printf("SET BLF: %s\n", nuevo);
				fflush(stdout);
				setAdvancedOptions(handle, "SET_BLF", nuevo);

			}
			else if (strncmp(msg, "SET_M", 5) == 0) {
				printf("msg: %s\n", msg);
				int longitud = strlen(msg) - 6;
				char *nuevo = (char*)malloc(sizeof(char) * (longitud + 1));
				nuevo[longitud] = '\0';
				strncpy(nuevo, msg + 6, longitud);
				printf("SET M: %s\n", nuevo);
				fflush(stdout);
				setAdvancedOptions(handle, "SET_M", nuevo);

			}
			else if (strncmp(msg, "SET_Q", 5) == 0) {
				printf("msg: %s\n", msg);
				setAdvancedOptions(handle, "SET_Q", nuevo);

			}
			else if (strncmp(msg, "SET_SESSION", 11) == 0) {
				printf("msg: %s\n", msg);
				int longitud = strlen(msg) - 12;
				char *nuevo = (char*)malloc(sizeof(char) * (longitud + 1));
				nuevo[longitud] = '\0';
				strncpy(nuevo, msg + 12, longitud);
				printf("SESION: %s\n", nuevo);
				fflush(stdout);
				setAdvancedOptions(handle, "SET_SESSION", nuevo);

			}
			else if (strncmp(msg, "SET_TARGET", 10) == 0) {
				printf("msg: %s\n", msg);
				int longitud = strlen(msg) - 11;
				char *nuevo = (char*)malloc(sizeof(char) * (longitud + 1));
				nuevo[longitud] = '\0';
				strncpy(nuevo, msg + 11, longitud);
				printf("SET TARGET: %s\n", nuevo);
				fflush(stdout);
				setAdvancedOptions(handle, "SET_TARGET", nuevo);

			}
 			memset(msg, 0, sizeof(msg));
		}

		closesocket(client);
		//closesocket(client2);
		printf("Client disconnected!\n");
		WSACleanup();
	}

} /* main */

