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
#include "r2000.h"


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

	//Servidor socket
	WSADATA WSAData;
	SOCKET server, client;
	SOCKADDR_IN serverAddr, clientAddr;

	void ** dato;
	char msg[25];

	char powerMinMax[10];
	char power[2];
	char Ready[6] = "READY";
	int reg[20];
	int puertos[4];
	int puertosC[4];
	int selAnt[4];
	char puertosConect[4];
	char puertosAnt[4];
	char antenaCheck[1];
	char selecAntenna[4];
	char regiones[20];

	WSAStartup(MAKEWORD(2, 0), &WSAData);
	server = socket(AF_INET, SOCK_STREAM, 0);

	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5557);

	bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
	listen(server, 0);
	printf("Listening for incoming connections...\n");

	char buffer[1024];
	int conectado = 0;
	int clientAddrSize = sizeof(clientAddr);
	if ((client = accept(server, (SOCKADDR *)&clientAddr, &clientAddrSize)) != INVALID_SOCKET)
	{
		printf("Client connected!\n");
		recv(client, buffer, sizeof(buffer), 0);
		printf("Client says: %s\n", buffer);
		memset(buffer, 0, sizeof(buffer));
		if (strncmp(buffer, "CONNECT", 7) != 0)
		{
			conectado = 1;
			printf("CONECTADO AL READER\n");
			memset(buffer, 0, sizeof(buffer));
			//send(server, "OK", 2, 0);
		}
	}
	if (conectado == 1)
	{
		while (strncmp(buffer, "DISCONNECT", 10) != 0) {
			recv(client, buffer, sizeof(buffer), 0);
			printf("BUFFER: %s\n", buffer);
			//printf("%s\n", buffer);
			memset(buffer, 0, sizeof(buffer));
			//printf("DENTRO DEL WHILE\n");

		}
		closesocket(client);
		printf("Client disconnected!\n");
	}

	return 0;
} /* main */
