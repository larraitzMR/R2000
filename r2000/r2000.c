/*
 *****************************************************************************
 * Author: Larraitz Orio
 * Date: 2019
 *****************************************************************************
 */

#define _CRT_SECURE_NO_WARNINGS

 //#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include "rfid_library.h"
#include "rfid_packets.h"
#include "byte_swap.h"
#include "print_packet.h"

#include "network.h"
#include "r2000.h"
#include "reader_params.h"
#include "sample_utility.h"


#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "rfid.lib")


INT8U      maxOriginalAntennas = 0;
INT8U      maxAccessAPIRetries = 6;
INT8U      accessAPIRetryCount = 0;

int startReading = 0;
SOCKET clientRead;


RFID_RADIO_HANDLE           handle;

RFID_18K6C_INVENTORY_PARMS              inventoryParms;
RFID_PACKET_18K6C_INVENTORY* inv;
INT32U                                  inventoryFlags = 0;
int                                     indent_level = 0;



enum
{
	RWE_READ,
	RWE_WRITE,
	RWE_ERASE
};
typedef INT32U  RWE_OPTIONS;



#define  DEF_WORD_LENGTH     6
#define  DEF_START_OFFSET    0
#define  DEF_MEM_BANK        RFID_18K6C_MEMORY_BANK_TID
#define  DEF_RWE             RWE_WRITE 

#define RWE_READ_STR         "READ"
#define RWE_WRITE_STR        "WRITE"
#define RWE_ERASE_STR        "ERASE"

#define MAX_WORD_LENGTH      1024



// default init for original behavior (read 95-bits of EPC at offset 2
static INT16U g_WordLength = DEF_WORD_LENGTH;
static INT16U g_StartOffset = DEF_START_OFFSET;
static INT32U g_MemBank = DEF_MEM_BANK;

static RWE_OPTIONS g_ReadWriteErase = DEF_RWE;

RFID_STATUS                             status;
INT8U                                   readData[MAX_WORD_LENGTH * 2];
INT16U                                  writeData[MAX_WORD_LENGTH];
RFID_18K6C_READ_PARMS                   readParms;
RFID_18K6C_WRITE_PARMS                  writeParms;
CONTEXT_PARMS                           context;
INT32U									antena;
char dataHex[4];

static void saveByteArray(const INT8U* bytes, int length, char* buf)
{
	int index;
	char* b;

	b = malloc(sizeof(char) * (1 + 1));

	for (index = 0; index < length; ++index)
	{
		sprintf(b, "%.2x", bytes[index]);
		//printf("%s", b);
		strcat(buf, b);
		memset(b, 0, sizeof(b));
	}
}

INT32S PacketCallbackFunction(RFID_RADIO_HANDLE handle, INT32U bufferLength, const INT8U* pBuffer, void* context)
{
	int* indent = (int*)context;
	RFID_UNREFERENCED_LOCAL(handle);
	char mensaje[50];
	int index;
	char buf[25];
	char PC[5]; 
	char b[1];
	char rsi[4];
	char rssi[3];
	//const INT8U* packet;


	RFID_PACKET_COMMON* common = (RFID_PACKET_COMMON*)pBuffer; 
	INT16U packetType = MacToHost16(common->pkt_type);

	memset(buf, 0, sizeof(buf));
	memset(mensaje, 0, sizeof(mensaje));
	memset(rssi, 0, sizeof(rssi));


	if (packetType == RFID_PACKET_TYPE_ANTENNA_BEGIN) {
		RFID_PACKET_ANTENNA_BEGIN* antennabegin = (RFID_PACKET_ANTENNA_BEGIN*)pBuffer;

		antena = MacToHost32(antennabegin->antenna);
		/*INT8 a = MacToHost32(antennabegin->antenna);
		INT8U ant = MacToHost32(antennabegin->antenna);*/
	}
	else if (packetType == RFID_PACKET_TYPE_18K6C_INVENTORY) {

		RFID_PACKET_18K6C_INVENTORY* inv = (RFID_PACKET_18K6C_INVENTORY*)pBuffer;
		int length = ((MacToHost16(common->pkt_len) - 3) * 4) - (common->flags >> 6);


		INT8U* byteData = (INT8U*)& inv->inv_data[0];
		//INT16U ri = (INT8U*)& inv->rssi;
		//INT16U rsi = (INT16U*)& inv->rssi;
		//INT8 rssi = (INT8*)& inv->rssi;
		//printf("RSSI: %d", rssi);
		//INT8U rs = (INT8U*)& inv->rssi;

		/*INT32 nb_rssi32 = (INT32*)& inv->nb_rssi;
		
		INT8U nb_rssiu = (INT8U*)& inv->nb_rssi;
		

		INT8U pr = &inv->nb_rssi;*/
		//	printf(" %u, %u, %u\n", &inv->rssi, ri, rs);

		//INT8 nb_rssi = (INT8*)&inv->nb_rssi;
		//printf("NB_RSSI: %d\n", nb_rssi);

		saveByteArray((INT8*)&inv->nb_rssi, 1, rssi);
		//printf("RSSI: %s\n", rssi);

		int epcLength = 0;
		int tidLength = 0;
		if (((common->flags >> 2) & 0x03) == 0x01)  /* M4 TID (12 bytes) is included in data */
		{
			tidLength = 12;
		}
		epcLength = length - tidLength - 4;  /* -4 for 16-bit PC and CRC */

		memset(buf, 0, sizeof(buf));
	
		int* selAnt[4]; 
		//getEnabledAntena(handle, selAnt);


		saveByteArray(&byteData[2], epcLength, buf);
		//printf("EPC: %s", buf);

		if (strlen(buf) != 0) {
			/*printf(" PC: ");
			saveByteArray(&byteData[0], 2, PC);

			printf(" CRC: "); 
			saveByteArray(&byteData[2 + epcLength], 2, CRC);*/

			/*printf(" RSSI: "); s
			printf("%d\n", rssi);*/
			/*sprintf(rsi, "%u", rssi);
			printf("%s\n", rsi);*/

			//sprintf(mensaje, "%s,%s,%s,%s", PC, buf, CRC, rsi);
			sprintf(mensaje, "$%s,%s,%u#", buf, rssi, antena);
			printf(mensaje);
			send(clientRead, mensaje, strlen(mensaje), 0);

			memset(buf, 0, sizeof(buf));
			memset(mensaje, 0, sizeof(mensaje));
			memset(rssi, 0, sizeof(rssi));

		}
	}
	send(clientRead, "$#", 2, 0);
	
	return 0;
}


DWORD WINAPI startRead(void* data) {
	int status = 0;
	char msg[20];
	printf("HILO\n");
	/* Set up the inventory parameters */
	inventoryParms.length = sizeof(RFID_18K6C_INVENTORY_PARMS);
	inventoryParms.common.tagStopCount = 0;
	inventoryParms.common.pCallback = PacketCallbackFunction;
	inventoryParms.common.pCallbackCode = NULL;
	inventoryParms.common.context = &indent_level;

	int index;

	while (startReading == 1) {
		//printf("START READING\n");
		/* Attempt to perform an inventory on the radio */
		if (RFID_STATUS_OK !=
			(status = RFID_18K6CTagInventory(handle, &inventoryParms, inventoryFlags)))
		{
			//printf("RFID_18K6CTagInventory failed: RC = %d\n", status);
		}
	}
	return 0;
}

DWORD WINAPI stopRead(void* data) {
	RFID_RadioCancelOperation(handle, 0);
	startReading = 0;
}

// Function to convert hexadecimal to decimal
int* hexadecimalToDecimal(char hexVal[], INT16U* data)
{
	int len = 24;

	// Initializing base value to 1, i.e 16^0 
	int base = 1;
	int cont = 0;
	int write = 5;

	int dec_val = 0;
	INT16U writeData[6];

	// Extracting characters as digits from last character 
	for (int i = len - 1; i >= 0; i--)
	{
		// if character lies in '0'-'9', converting  
		// it to integral 0-9 by subtracting 48 from 
		// ASCII value. 
		if (hexVal[i] >= '0' && hexVal[i] <= '9')
		{
			dec_val += (hexVal[i] - 48) * base;

			// incrementing base by power 
			base = base * 16;
		}

		// if character lies in 'A'-'F' , converting  
		// it to integral 10 - 15 by subtracting 55  
		// from ASCII value 
		else if (hexVal[i] >= 'A' && hexVal[i] <= 'F')
		{
			dec_val += (hexVal[i] - 55) * base;

			// incrementing base by power 
			base = base * 16;
		}
		cont++;
		if (cont == 4) {
			writeData[write] = dec_val;
			data[write] = dec_val;
			write--;
			cont = 0;
			dec_val = 0;
			base = 1;
		}
	}
	//data = writeData;

	return data;
}

#define BYTES_PER_LEN_UNIT  4
RFID_18K6C_SINGULATION_FIXEDQ_PARMS     singulationParms;


DWORD WINAPI readTagData(void* data) {

	INT32U  index;
	INT16U  word;
	INT8U* packet;
	char selAnt[4];
	char EPC[33];
	char TID[25];
	char toSend[60];
	getConnectedAntennaPorts(handle, selAnt);

	memset(EPC, 0, sizeof(EPC));
	memset(TID, 0, sizeof(TID));

	/* The MAC uses the fixed-Q singulation algorithm for tag read and write, */
		/* so we'll configure the algorithm so that it works well for a single    */
		/* tag and even though it is not necessary, set the singulation algorithm */
		/* to fixed Q.                                                            */
	singulationParms.length =
		sizeof(RFID_18K6C_SINGULATION_FIXEDQ_PARMS);
	singulationParms.qValue = 0;
	singulationParms.retryCount = 0;
	singulationParms.toggleTarget = 0;
	singulationParms.repeatUntilNoTags = 0;
	status = RFID_18K6CSetSingulationAlgorithmParameters(
		handle,
		RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ,
		&singulationParms);
	if (RFID_STATUS_OK != status)
	{
		fprintf(
			stderr,
			"ERROR: RFID_18K6CSetSingulationAlgorithmParameters returned 0x%.8x\n",
			status);
	}
	status = RFID_18K6CSetCurrentSingulationAlgorithm(
		handle,
		RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ);
	if (RFID_STATUS_OK != status)
	{
		fprintf(
			stderr,
			"ERROR: RFID_18K6CSetCurrentSingulationAlgorithm returned 0x%.8x\n",
			status);
	}

	/* Configure the MAC for the data response mode (packets) we want to see */
	status = RFID_RadioSetResponseDataMode(
		handle,
		(RFID_RESPONSE_TYPE)RFID_RESPONSE_TYPE_DATA,
		RFID_RESPONSE_MODE_COMPACT);

	if (RFID_STATUS_OK != status)
	{
		fprintf(
			stderr,
			"ERROR: RFID_RadioSetResponseDataMode returned 0x%.8x\n",
			status);
	}

	context.succesfulAccessPackets = 0;
	context.pReadData = readData;
	readParms.length = sizeof(readParms);
	readParms.readCmdParms.length = sizeof(readParms.readCmdParms);
	readParms.readCmdParms.bank = RFID_18K6C_MEMORY_BANK_EPC;
	readParms.readCmdParms.count = 8;
	readParms.readCmdParms.offset = 0;
	readParms.accessPassword = 0;
	readParms.common.pCallback = RfidTagAccessCallback;
	readParms.common.pCallbackCode = NULL;
	readParms.common.tagStopCount = 0;
	readParms.common.context = &context;

	/* Keep attempting to read   from the tag's memory until it     */
	/* succeeds or until the tag-read function fails for some reason.         */
	accessAPIRetryCount = 0;
	while ((RFID_STATUS_OK == status) &&
		!context.succesfulAccessPackets &&
		(accessAPIRetryCount < maxAccessAPIRetries))
	{
		printf("Attempting to read \n\n");

		status = RFID_18K6CTagRead(handle, &readParms, 0);
		if (RFID_STATUS_OK != status)
		{
			fprintf(
				stderr,
				"ERROR: RFID_18K6CTagRead returned 0x%.8x\n",
				status);
		}
		RFID_MacClearError(handle);
		accessAPIRetryCount++;
	}
	if (!context.succesfulAccessPackets)
	{
		printf("Tag access read failed\n");
	}

	printf("Read Data=");

	/* Flip all of the  bits so we have something to write back that is    */
	/* different from the tag's current bits.                              */
	for (index = 0; index < 8; ++index)
	{
		word = (((INT16U)readData[index * 2]) << 8) | readData[(index * 2) + 1];
		printf("%04X ", word);
		writeData[index] = ~word;
		sprintf(dataHex, "%04X", word);
		strcat(EPC, dataHex);

	}
	printf("\n\n");

	//sprintf(toSend, "%04X", writeData);
	printf("EPC: %s\n", EPC);

	context.succesfulAccessPackets = 0;
	context.pReadData = readData;
	readParms.length = sizeof(readParms);
	readParms.readCmdParms.length = sizeof(readParms.readCmdParms);
	readParms.readCmdParms.bank = RFID_18K6C_MEMORY_BANK_TID;
	readParms.readCmdParms.count = g_WordLength;
	readParms.readCmdParms.offset = g_StartOffset;
	readParms.accessPassword = 0;
	readParms.common.pCallback = RfidTagAccessCallback;
	readParms.common.pCallbackCode = NULL;
	readParms.common.tagStopCount = 0;
	readParms.common.context = &context;

	/* Keep attempting to read   from the tag's memory until it     */
	/* succeeds or until the tag-read function fails for some reason.         */
	accessAPIRetryCount = 0;
	while ((RFID_STATUS_OK == status) &&
		!context.succesfulAccessPackets &&
		(accessAPIRetryCount < maxAccessAPIRetries))
	{
		printf("Attempting to read \n\n");

		status = RFID_18K6CTagRead(handle, &readParms, 0);
		if (RFID_STATUS_OK != status)
		{
			fprintf(
				stderr,
				"ERROR: RFID_18K6CTagRead returned 0x%.8x\n",
				status);
		}
		RFID_MacClearError(handle);
		accessAPIRetryCount++;
	}
	if (!context.succesfulAccessPackets)
	{
		printf("Tag access read failed\n");
	}

	printf("Read Data=");

	/* Flip all of the  bits so we have something to write back that is    */
	/* different from the tag's current bits.                              */
	for (index = 0; index < g_WordLength; ++index)
	{
		word = (((INT16U)readData[index * 2]) << 8) | readData[(index * 2) + 1];
		printf("%04X ", word);
		writeData[index] = ~word;
		sprintf(dataHex, "%04X", word);
		strcat(TID, dataHex);

	}
	printf("\n\n");

	//sprintf(toSend, "%04X", writeData);
	printf("EPC: %s\n", EPC);
	printf("TID: %s\n", TID);
	sprintf(toSend, "$%s,%s#", EPC, TID);

	send(clientRead, toSend, sizeof(toSend), 0);
	memset(toSend, 0, sizeof(toSend));
	memset(dataHex, 0, sizeof(dataHex));


	return 0;
}

//DWORD WINAPI writeTagData(void* data) {
//
//	INT32U  index;
//	INT16U  word;
//
//	context.succesfulAccessPackets = 0;
//	writeParms.length = sizeof(writeParms);
//	writeParms.writeType = RFID_18K6C_WRITE_TYPE_SEQUENTIAL;
//	writeParms.writeCmdParms.sequential.length = sizeof(RFID_18K6C_WRITE_SEQUENTIAL_CMD_PARMS);
//	writeParms.writeCmdParms.sequential.bank = RFID_18K6C_MEMORY_BANK_EPC;
//	writeParms.writeCmdParms.sequential.count = 6;
//	writeParms.writeCmdParms.sequential.offset = 2;
//	//EN VEZ DE WRITE DATA, PASARLE EL BUFFER DE LO LEIDO.
//	writeParms.writeCmdParms.sequential.pData = (char)data;
//	writeParms.accessPassword = 0;
//	writeParms.common.pCallback = RfidTagAccessCallback;
//	writeParms.common.pCallbackCode = NULL;
//	writeParms.common.tagStopCount = 0;
//	writeParms.common.context = &context;
//	/* Keep attempting to write  to the tag's  memory until it      */
//		   /* succeeds or until the tag-write function fails for some reason.        */
//	accessAPIRetryCount = 0;
//	while ((RFID_STATUS_OK == status) &&
//		!context.succesfulAccessPackets &&
//		(accessAPIRetryCount < maxAccessAPIRetries))
//	{
//		printf("Attempting to write \n\n");
//		status = RFID_18K6CTagWrite(handle, &writeParms, 0);
//		if (RFID_STATUS_OK != status)
//		{
//			fprintf(
//				stderr,
//				"ERROR: RFID_18K6CTagWrite returned 0x%.8x\n",
//				status);
//		}
//		RFID_MacClearError(handle);
//		accessAPIRetryCount++;
//	}
//	if (!context.succesfulAccessPackets)
//	{
//		printf("Tag access write failed\n");
//	}
//
//	return 0;
//}

void writeTagData(INT16U data[6]) {
	INT32U  index;
	INT16U  word;

	context.succesfulAccessPackets = 0;
	writeParms.length = sizeof(writeParms);
	writeParms.writeType = RFID_18K6C_WRITE_TYPE_SEQUENTIAL;
	writeParms.writeCmdParms.sequential.length = sizeof(RFID_18K6C_WRITE_SEQUENTIAL_CMD_PARMS);
	writeParms.writeCmdParms.sequential.bank = RFID_18K6C_MEMORY_BANK_EPC;
	writeParms.writeCmdParms.sequential.count = 6;
	writeParms.writeCmdParms.sequential.offset = 2;
	//EN VEZ DE WRITE DATA, PASARLE EL BUFFER DE LO LEIDO.
	writeParms.writeCmdParms.sequential.pData = data;
	writeParms.accessPassword = 0;
	writeParms.common.pCallback = RfidTagAccessCallback;
	writeParms.common.pCallbackCode = NULL;
	writeParms.common.tagStopCount = 0;
	writeParms.common.context = &context;
	/* Keep attempting to write  to the tag's  memory until it      */
		   /* succeeds or until the tag-write function fails for some reason.        */
	accessAPIRetryCount = 0;
	while ((RFID_STATUS_OK == status) &&
		!context.succesfulAccessPackets &&
		(accessAPIRetryCount < maxAccessAPIRetries))
	{
		printf("Attempting to write \n\n");
		status = RFID_18K6CTagWrite(handle, &writeParms, 0);
		if (RFID_STATUS_OK != status)
		{
			fprintf(
				stderr,
				"ERROR: RFID_18K6CTagWrite returned 0x%.8x\n",
				status);
		}
		RFID_MacClearError(handle);
		accessAPIRetryCount++;
	}
	if (!context.succesfulAccessPackets)
	{
		printf("Tag access write failed\n");
	}

	return 0;
}



#define BUFFER_SIZE 70

int main(
	int     argc,
	char** argv
)
{
	RFID_STATUS                 status;
	RFID_RADIO_ENUM* pEnum;

	RFID_RADIO_POWER_STATE		pstate;
	RFID_RADIO_INFO* pInfo;
	INT32U                      index;
	INT32U                      antenna;
	RFID_VERSION				version;
	RFID_ANTENNA_PORT_STATUS    antennaStatus;
	RFID_ANTENNA_PORT_CONFIG    antennaConfig;
	INT32						antennaPort;
	RFID_18K6C_TAG_GROUP		pGroup;
	RFID_IMPINJ_EXTENSIONS      pExtensions;

	RFID_RADIO_OPERATION_MODE	pmode;
	RFID_RADIO_LINK_PROFILE		linkProfile;

	RFID_RADIO_POWER_STATE      pradio;

	RFID_MAC_REGION* pRegion;
	void* pRegionConfig;




	RFID_UNREFERENCED_LOCAL(argc);
	RFID_UNREFERENCED_LOCAL(argv);

	//Servidor socket
	WSADATA WSAData;
	SOCKET server, client;
	//SOCKADDR_IN  clientAddr;
	struct sockaddr_in clientAddr, clientAddrRead;
	char buffer[10];
	int msgsize = 40;
	char msg[BUFFER_SIZE + 1];
	int conectado = 0;
	int clientAddrSize = sizeof(clientAddr);
	int clientAddrSizeRead = sizeof(clientAddrRead);
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
	pEnum = (RFID_RADIO_ENUM*)malloc(sizeof(RFID_RADIO_ENUM));
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
			(RFID_RADIO_ENUM*)realloc(pEnum, pEnum->totalLength);
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
		/* Do something useful with the radio */
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
	pExtensions.fastId = RFID_FAST_ID_DISABLED;
	RFID_STATUS RFID_RadioSetImpinjExtensions(handle, pExtensions);


	/* COMUNICACIÓN SOCKET CON EL SOFTWARE MYRUNS */
	server = configure_tcp_socket(5557);

	if ((client = accept(server, (struct sockaddr*) & clientAddr, &clientAddrSize)) != INVALID_SOCKET)
	{
		conectado = 1;
		printf("CONECTADO AL READER\n");
	}

	server = configure_tcp_socket(5556);
	if ((clientRead = accept(server, (struct sockaddr*) & clientAddrRead, &clientAddrSizeRead)) != INVALID_SOCKET)
	{
		printf("Conectado para enviar tags!\n");
	}

	while (conectado == 1) {
		//printf("while\n");
		/*retval = recvfrom(client, msg, sizeof(msg), 0, (struct sockaddr*)&clientAddr, &clientAddrSize);*/
		memset(msg, '\0', BUFFER_SIZE + 1);
		retval = recv(client, msg, (BUFFER_SIZE), MSG_WAITALL);
		//buscar #
		//sprintf(msg, "%s", msg);
		char* mens = strtok(msg, "#");
		sprintf(msg, "%s", mens);

		//printf("msg WHILE: %s\n", msg);
		if (strncmp(msg, "DISCONNECT", 10) == 0) {
			printf("msg: %s\n", msg);
			conectado = 0;
			send(client, "OK#", 3, 0);
		}
		if (strncmp(msg, "POWER_MINMAX", 12) == 0)
		{
			printf("msg: %s\n", msg);
		}
		else if (strncmp(msg, "GET_POWER", 9) == 0) {
			printf("msg: %s\n", msg);
			char pow[6] = "";
			power = getAntennaPower(handle);
			printf("ENVIADO POWER: %.1f\n", power);
			sprintf(pow, "%.1f#", power);
			send(client, pow, sizeof(pow), 0);
		}
		else if (strncmp(msg, "SET_POWER", 9) == 0) {
			printf("msg: %s\n", msg);
			char* mens = strtok(msg, " ");
			char* pow = strtok(NULL, " ");
			double value = atof(pow);
			printf("RECIBIDO POWER: %.1f\n", value);
			setAntennaPower(handle, value);
			send(client, "OK#", 3, 0);
		}
		else if (strcmp(msg, "ANT_PORTS") == 0) {
			//printf("msg: %s\n", msg);
		}
		else if (strncmp(msg, "CON_ANT_PORTS", 13) == 0) {
			//que antenas estan enabled
			printf("msg: %s\n", msg);
			char selAnt[4];
			char selAntSend[5];
			getConnectedAntennaPorts(handle, selAnt);
			//sprintf(selAntSend, "%s#", selAnt);
			send(client, selAntSend, 4, 0);
		}
		else if (strncmp(msg, "GET_SEL_ANT", 11) == 0) {
			printf("msg: %s\n", msg);
			//int* selAnt[4];
			char selAnt[5] = {0};
			char selAntSend[5];
			char p[2] = {'1','#'};
			char a[2] = "1#";
			getConnectedAntennaPorts(handle, selAnt);
			strcat(selAnt, "#");
			send(client, selAnt, strlen(selAnt), 0);	
		}
		else if (strncmp(msg, "SET_SEL_ANT", 11) == 0) {
			printf("msg: %s\n", msg);
			char* mens = strtok(msg, " ");
			char* ant = strtok(NULL, "");
			printf("CONECTADAS: %s\n", ant);
			setSelectedAntena(handle, ant);
			send(client, "OK#", 3, 0);
		}
		else if (strncmp(msg, "GET_INFO", 8) == 0) {
			printf("msg: %s\n", msg);
			char info[9];
			char infoSend[9];
			//getReaderInfo(handle, info);
			sprintf(infoSend, "%s#", info);
			send(client, "2.4.240#", sizeof(infoSend), 0);
			memset(info, 0, sizeof(info));
			memset(infoSend, 0, sizeof(infoSend));
		}
		else if (strncmp(msg, "GET_ADV_OPT", 11) == 0) {
			printf("msg: %s\n", msg);
			char option[7];
			char optionSend[8];
			getAdvancedOptions(handle, option);
			sprintf(optionSend, "%s#", option);
			send(client, optionSend, sizeof(optionSend), 0);
			memset(option, 0, sizeof(option));
			memset(optionSend, 0, sizeof(optionSend));
		}
		else if (strncmp(msg, "SET_REGION", 10) == 0) {
			printf("msg: %s\n", msg);
			char* mens = strtok(msg, " ");
			char* reg = strtok(NULL, "");
			printf("REGION: %s\n", reg);
			fflush(stdout);

			setAdvancedOptions(handle, "SET_REGION", nuevo);
			send(client, "OK#", 3, 0);
		}
		else if (strncmp(msg, "SET_TARI", 8) == 0) {
			printf("msg: %s\n", msg);
			char* mens = strtok(msg, " ");
			char* tari = strtok(NULL, "");
			printf("SET TARI: %s\n", tari);
			fflush(stdout);
			setAdvancedOptions(handle, "SET_TARI", tari);
			send(client, "OK#", 3, 0);

		}
		else if (strncmp(msg, "SET_BLF", 7) == 0) {
			printf("msg: %s\n", msg);
			char* mens = strtok(msg, " ");
			char* blf = strtok(NULL, "");
			printf("SET BLF: %s\n", blf);
			fflush(stdout);
			setAdvancedOptions(handle, "SET_BLF", blf);
			send(client, "OK#", 3, 0);
		}
		else if (strncmp(msg, "SET_M", 5) == 0) {
			printf("msg: %s\n", msg);
			char* mens = strtok(msg, " ");
			char* m = strtok(NULL, "");
			printf("SET M: %s\n", m);
			fflush(stdout);
			setAdvancedOptions(handle, "SET_M", m);
			send(client, "OK#", 3, 0);
		}
		else if (strncmp(msg, "SET_Q", 5) == 0) {
			printf("msg: %s\n", msg);
			//setAdvancedOptions(handle, "SET_Q", nuevo);
			send(client, "OK#", 3, 0);

		}
		else if (strncmp(msg, "SET_SESSION", 11) == 0) {
			printf("msg: %s\n", msg);
			char* mens = strtok(msg, " ");
			char* session = strtok(NULL, "");
			printf("SESION: %s\n", session);
			fflush(stdout);
			setAdvancedOptions(handle, "SET_SESSION", session);
			send(client, "OK#", 3, 0);
		}
		else if (strncmp(msg, "SET_TARGET", 10) == 0) {
			printf("msg: %s\n", msg);
			char* mens = strtok(msg, " ");
			char* target = strtok(NULL, "");
			printf("SET TARGET: %s\n", target);
			fflush(stdout);
			setAdvancedOptions(handle, "SET_TARGET", target);
			send(client, "OK#", 3, 0);
		}
		else if (strncmp(msg, "START_READING", 13) == 0) {
			antena = 0;
			printf("msg: %s\n", msg);
			HANDLE thread = CreateThread(NULL, 0, startRead, clientRead, 0, NULL);
			startReading = 1;
			send(client, "OK#", 3, 0);
		}
		else if (strncmp(msg, "STOP_READING", 13) == 0) {
			printf("msg: %s\n", msg);
			HANDLE thread = CreateThread(NULL, 0, stopRead, clientRead, 0, NULL);
			send(client, "OK#", 3, 0);
			//startReading = 0;
			//CloseHandle(hilo);
		}
		else if (strncmp(msg, "READ_INFO", 9) == 0) {
			printf("msg: %s\n", msg);
			HANDLE thread = CreateThread(NULL, 0, readTagData, clientRead, 0, NULL);
			startReading = 1;
			send(client, "OK#", 3, 0);
		}
		else if (strncmp(msg, "WRITE_EPC", 9) == 0) {
			printf("msg: %s\n", msg);
			char EPCBuf[24];
			INT16U writeData[6];

			char* mens = strtok(msg, " ");
			char* ant = strtok(NULL, " ");
			char* t = strtok(NULL, " ");
			char* TID = strtok(NULL, " ");
			char* EPC = strtok(NULL, " ");
			sprintf(EPCBuf, "%s", EPC);

			startReading = 0;
			int* ptr = hexadecimalToDecimal(EPC, writeData);
			//hexToDec(EPC);
			writeTagData(writeData);
			//HANDLE thread = CreateThread(NULL, 0, writeTagData, EPCBuf, 0, NULL);
			send(client, "OK#", 3, 0);
		}

	}

		closesocket(client);
		//closesocket(client2);
		printf("Client disconnected!\n");
		WSACleanup();

} /* main */