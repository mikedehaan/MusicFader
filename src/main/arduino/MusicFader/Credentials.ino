
/** Load WLAN credentials from EEPROM */
void loadCredentials() {
    EEPROM.begin(512);

    EEPROM.get(0, eepromSavedData);
    //EEPROM.get(0 + sizeof(ssid), password);
    char ok[2 + 1];
    EEPROM.get(0 + sizeof(eepromSavedData), ok);
    EEPROM.end();
    if (String(ok) != String("OK")) {
        eepromSavedData.ssid[0] = 0;
        eepromSavedData.password[0] = 0;
    } else {
        Serial.println("EEPROM was ok. Reading credentials.");
        //memmove(ssid, eepromSavedData.ssid, sizeof(eepromSavedData.ssid));
        //memmove(password, eepromSavedData.password, sizeof(eepromSavedData.password));
    }
    
    Serial.println("Recovered credentials:");
    Serial.println(eepromSavedData.ssid);
    Serial.println(strlen(eepromSavedData.password) > 0 ? "********" : "<no password>");
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {
    EEPROM.begin(512);
    //memmove(eepromSavedData.ssid,     ssid,     sizeof(ssid));
    //memmove(eepromSavedData.password, password, sizeof(password));
    EEPROM.put(0, eepromSavedData);
    char ok[2 + 1] = "OK";
    EEPROM.put(0 + sizeof(eepromSavedData), ok);
    EEPROM.commit();
    EEPROM.end();
    Serial.println("Credential saved.");
}
