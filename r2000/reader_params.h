#include "rfid_library.h"

void initializeRFID(RFID_RADIO_HANDLE handle, RFID_RADIO_ENUM* pEnum);
int getAntennaPower(RFID_RADIO_HANDLE handle);
int setAntennaPower(RFID_RADIO_HANDLE handle, double power);
void getConnectedAntennaPorts(RFID_RADIO_HANDLE handle, char ant[4]);
void getReaderInfo(RFID_RADIO_HANDLE handle, char inf[9]);
void getAdvancedOptions(RFID_RADIO_HANDLE handle, char inf[40]);
void setAdvancedOptions(RFID_RADIO_HANDLE handle, char msg[20], char inf[15]);