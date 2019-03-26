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
	SOCKADDR_IN  clientAddr;
	char buffer[50];
	int conectado = 0;
	int clientAddrSize = sizeof(clientAddr);

	server = configure_tcp_socket(5557);

	if ((client = accept(server, (SOCKADDR *)&clientAddr, &clientAddrSize)) != INVALID_SOCKET)
	{
		printf("Client connected!\n");
		recv(client, buffer, sizeof(buffer), 0);
		printf("Client says: %s\n", buffer);
		if (strncmp(buffer, "CONNECT", 7) == 0)
		{
			conectado = 1;
			printf("CONECTADO AL READER\n");
			memset(buffer, 0, sizeof(buffer));
			send(client, "READY", 5, 0);
		}
	}
	if (conectado == 1)
	{
		while (strncmp(buffer, "DISCONNECT", 10) != 0) {
			recv(client, buffer, sizeof(buffer), 0);
			printf("BUFFER: %s\n", buffer);
			//printf("%s\n", buffer);
			//memset(buffer, 0, sizeof(buffer));
			//printf("DENTRO DEL WHILE\n");

		}
		closesocket(client);
		printf("Client disconnected!\n");
	}

	return 0;
} /* main */
