#include "rfid_library.h"

void initializeRFID(RFID_RADIO_HANDLE handle, RFID_RADIO_ENUM* pEnum);
int getAntennaPower(RFID_RADIO_HANDLE handle);
int setAntennaPower(RFID_RADIO_HANDLE handle, double power);
void getReaderInfo(RFID_RADIO_HANDLE handle, char *inf);
void getConnectedAntennaPorts(RFID_RADIO_HANDLE handle, char ant[4]);