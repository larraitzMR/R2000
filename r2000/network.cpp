
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
#define DEFAULT_PROTO SOCK_STREAM
//Servidor socket
WSADATA WSAData;
SOCKET ReceivingSocket, client;
SOCKADDR_IN ReceiverAddr, clientAddr;





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
	if ((ReceivingSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		return -1;
	}
	else {
		printf("Server: socket() is OK!\n");
	}

	memset(&ReceiverAddr, 0, sizeof(ReceiverAddr));
	memset(&clientAddr, 0, sizeof(clientAddr));

	ReceiverAddr.sin_addr.s_addr = INADDR_ANY;
	ReceiverAddr.sin_family = AF_INET;
	ReceiverAddr.sin_port = htons(port);


	// Bind the socket with the server address 
	if (bind(ReceivingSocket, (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	// Some info on the receiver side...

	getsockname(ReceivingSocket, (SOCKADDR *)&ReceiverAddr, (int *)sizeof(ReceiverAddr));

	printf("Server: I'm listening and waiting connection on port %d\n", port);

	//printf("Server: Receiving IP(s) used : %s\n", inet_pton(AF_INET, "192.168.1.52", &ReceiverAddr.sin_addr));
	printf("Server: Receiving port used : %d\n", htons(ReceiverAddr.sin_port));
	printf("Server: I\'m ready to receive a datagram...\n");

	////Make socket NON-BLOCK: recvfrom() returns with EWOULDBLOCK
	//flags = fcntl(descriptor, F_GETFL);
	//flags |= O_NONBLOCK;
	//fcntl(descriptor, F_SETFL, flags);

	return ReceivingSocket;
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


/* Read (and parse) messages from UDP socket */
int read_udp_message(int sock_descriptor, char *message, char len) {
	struct sockaddr_in SenderAddr; //Keeps connection information on client side
	int SenderAddrSize = sizeof(SenderAddr);

	int ByteReceived; //Number of chars received
	//int len=0;

	//memset(message, 0, len);
	ByteReceived = recvfrom(sock_descriptor, message, 64, 0, (struct sockaddr *)&SenderAddr, &SenderAddrSize);

	//Process packet
	if (ByteReceived > 0) {
		printf("Server: Total Bytes received : %d\n", ByteReceived);
		printf("Server: The data is \%s\\n", message);

	}
	else if (ByteReceived <= 0) {
		printf("Server: Connection closed with error code : %ld\n",	WSAGetLastError());
	}
	else {

		printf("Server: recvfrom() failed with error code : %d\n",WSAGetLastError());
	}


	// Some info on the sender side

	getpeername(ReceivingSocket, (SOCKADDR *)&SenderAddr, &SenderAddrSize);

	printf("Server: Sending IP used : %s\n", inet_pton(AF_INET, IP_ADDRESS, &SenderAddr.sin_addr));
	printf("Server: Sending port used : %d\n", htons(SenderAddr.sin_port));

	return ByteReceived;
} // READ_MESSAGE END


/* Configure a local TCP listen socket in a given port.
 * Return socket descriptor or -1 in case of error
 */
int configure_tcp_socket(int port) {


	printf("Listening for incoming connections...\n");

	WSAStartup(MAKEWORD(2, 0), &WSAData);
	if ((ReceivingSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		return -1;
	}
	else {
		printf("Server: socket() is OK!\n");
	}

	memset(&ReceiverAddr, 0, sizeof(ReceiverAddr));
	memset(&clientAddr, 0, sizeof(clientAddr));

	ReceiverAddr.sin_addr.s_addr = INADDR_ANY;
	ReceiverAddr.sin_family = AF_INET;
	ReceiverAddr.sin_port = htons(port);


	// Bind the socket with the server address 
	if (bind(ReceivingSocket, (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	// Some info on the receiver side...

	getsockname(ReceivingSocket, (SOCKADDR *)&ReceiverAddr, (int *)sizeof(ReceiverAddr));

	printf("Server: I'm listening and waiting connection on port %d\n", port);

	//printf("Server: Receiving IP(s) used : %s\n", inet_pton(AF_INET, "192.168.1.52", &ReceiverAddr.sin_addr));
	printf("Server: Receiving port used : %d\n", htons(ReceiverAddr.sin_port));
	printf("Server: I\'m ready to receive a datagram...\n");

	listen(ReceivingSocket, 0);
	printf("Listening for incoming connections...\n");

	return ReceivingSocket;
} // CONFIGURE_TCP_SOCKET END