
#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <iostream>
#include "rfid_library.h"
#include "network.h"
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "Ws2_32.lib")

// =======================================================================
//
//  GLOBAL VARIABLES
//
// =======================================================================
struct sockaddr_in local_socket; //Configuration for local socket

//Servidor socket
WSADATA WSAData;
SOCKET server, client;
SOCKADDR_IN serverAddr, clientAddr;



// =======================================================================
//
//  FUNCTIONS
//
// =======================================================================

/* Configure a local UDP listen socket in a given port.
 * Return socket descriptor or -1 in case of error
 */
int configure_udp_socket(int port) {


	WSAStartup(MAKEWORD(2, 0), &WSAData);
	if ((server = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		return -1;
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	memset(&clientAddr, 0, sizeof(clientAddr));

	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);

	// Bind the socket with the server address 
	if (bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	////Make socket NON-BLOCK: recvfrom() returns with EWOULDBLOCK
	//flags = fcntl(descriptor, F_GETFL);
	//flags |= O_NONBLOCK;
	//fcntl(descriptor, F_SETFL, flags);

	return server;
} // CONFIGURE_UDP_SOCKET END


/* Send UDP message to remote receiver */
int send_udp_msg(int socket_fd, char *data, int port) {

	struct sockaddr_in remote_socket; //Keeps connection information on remote side
	char message[64];
	short i;

	memset(&remote_socket, 0, sizeof(remote_socket));

	//Configure message receiver address
	remote_socket.sin_family = AF_INET;
	remote_socket.sin_port = htons(port);
	if (inet_pton(AF_INET, IP_ADDRESS, &remote_socket.sin_addr) == 0) {
		return -1;
	}

	// MESSAGE FORMAT:
	// * | MSG_LEN(1b) | CO(1b) | DATA(Nb) | CHECKSUM(1b)

	//Empty message
	memset(message, 0, sizeof(message));

	//Data
	for (i = 0; i < strlen(data); i++) {
		message[i] = data[i];
	}

	//Checksum
	//message[len] = network_checksum(message,len);


	//DEBUG
#ifdef NETWORK_DEBUG
//	for (i=0; i<len+1; i++){
//		printf("%02x:",(unsigned char)message[i]);
//	}
//	printf("\n");
//	fflush(stdout);
#endif

//Send message
	if (sendto(socket_fd, message, strlen(data) + 1, 0, (struct sockaddr *)&remote_socket, sizeof(remote_socket)) < 0) {
		return -1;
	}

	return 0;
} // SEND_UDP_MSG END

/* Send UDP message to remote receiver */
int send_udp_msg_checksum(int socket_fd, char *data, int port) {

	struct sockaddr_in remote_socket; //Keeps connection information on remote side
	char message[64];
	short i;

	memset(&remote_socket, 0, sizeof(remote_socket));

	//Configure message receiver address
	remote_socket.sin_family = AF_INET;
	remote_socket.sin_port = htons(port);
	if (inet_pton(AF_INET, IP_ADDRESS, &remote_socket.sin_addr) == 0) {
		return -1;
	}

	// MESSAGE FORMAT:
	// * | MSG_LEN(1b) | CO(1b) | DATA(Nb) | CHECKSUM(1b)

	//Empty message
	memset(message, 0, sizeof(message));

	//Data
	for (i = 0; i < strlen(data); i++) {
		message[i] = data[i];
	}


	//Checksum
	message[strlen(data)] = network_checksum(message, strlen(data));


	//DEBUG
#ifdef NETWORK_DEBUG
//	for (i=0; i<len+1; i++){
//		printf("%02x:",(unsigned char)message[i]);
//	}
//	printf("\n");
//	fflush(stdout);
#endif

//Send message
	if (sendto(socket_fd, message, strlen(data) + 1, 0, (struct sockaddr *)&remote_socket, sizeof(remote_socket)) < 0) {
		return -1;
	}

	return 0;
} // SEND_UDP_MSG END


/* Read (and parse) messages from UDP socket */
int read_udp_message(int sock_descriptor, char *message, char len) {
	struct sockaddr_in remote_socket; //Keeps connection information on client side
	int remote_server_len = sizeof(remote_socket);

	int rc; //Number of chars received
	//int len=0;

	memset(message, 0, len);
	rc = recvfrom(sock_descriptor, message, 64, 0, (struct sockaddr *)&remote_socket, &remote_server_len);

	//Check if there was a real error, if EWOULDBLOCK is retourned, is because socket is NON-BLOCK
	if (rc < 0) {
		if (errno != EWOULDBLOCK) {
#ifdef NETWORK_DEBUG
			//			printf("network: REAL ERROR WHEN READING AN UDP SOCKET\n");
			fflush(stdout);
#endif
			return -1;
		}
	}

	//Process packet
	if (rc > 0) {
		// Test checksum
		if (message[rc - 1] != network_checksum(message, rc - 1)) {
#ifdef NETWORK_DEBUG
			//			printf("network: WRONG CHECKSUM IN RECEIVED UDP MESSAGE\n");
			fflush(stdout);
#endif
			return -1;
		}

		// Test fist character
		if (message[0] != '*') {
#ifdef NETWORK_DEBUG
			printf("network: WRONG FIRST CHARACTER IN UDP MESSAGE\n");
			fflush(stdout);
#endif
			return -1;
		}

	}

	return rc;
} // READ_MESSAGE END




char network_checksum(char *data, short len) {
	char result = 0;
	short i = 0;

	for (i = 0; i < len; i++) {
		result += data[i];
	}

	//Two - complement
	result = ~result;
	result++;

	return result;
} // NETWORK_CHECKSUM END

