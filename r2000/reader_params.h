
void initializeRFID(RFID_RADIO_HANDLE handle, RFID_RADIO_ENUM* pEnum);
int getAntennaPower(RFID_RADIO_HANDLE handle);
int setAntennaPower(RFID_RADIO_HANDLE handle, INT32U antenna, double power);
void getReaderInfo(RFID_RADIO_HANDLE handle, char *inf);