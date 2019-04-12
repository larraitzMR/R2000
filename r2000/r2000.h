

#define PARAMS_PORT 		5555 //UDP port for parameters
#define CONTROL_PORT 		5556 //UDP port for control module
#define COMMUNICATIONS_PORT 5557 //UDP port for communications module
#define RFID_PORT			5558 //UDP port for rfid module



void Int32ArrayMacToHost(
	INT32U*         pBegin,
	INT32U          numberInt32
);

void Int16ArrayMacToHost(
	INT16U*         pBegin,
	INT32U          numberInt16
);