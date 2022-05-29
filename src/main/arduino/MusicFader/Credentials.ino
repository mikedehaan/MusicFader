
/** Load WLAN credentials from EEPROM */
void loadCredentials() {
    EEPROM.begin(512);

    EEPROM.get(0, eepromSavedData);
    char ok[2 + 1];
    EEPROM.get(0 + sizeof(eepromSavedData), ok);
    EEPROM.end();
    if (String(ok) != String("OK")) {
        // Set reasonable defaults
        eepromSavedData.fader1Bus     = 0;
        eepromSavedData.fader1Channel = 1;
        eepromSavedData.fader2Bus     = 0;
        eepromSavedData.fader2Channel = 2;
        eepromSavedData.ssid[0]       = 0;
        eepromSavedData.password[0]   = 0;
    } else {
        Serial.println("EEPROM was ok. Reading credentials.");
    }
    
    Serial.println("Recovered credentials:");
    Serial.println(eepromSavedData.ssid);
    Serial.println(strlen(eepromSavedData.password) > 0 ? "********" : "<no password>");
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {
    EEPROM.begin(512);
    EEPROM.put(0, eepromSavedData);
    char ok[2 + 1] = "OK";
    EEPROM.put(0 + sizeof(eepromSavedData), ok);
    EEPROM.commit();
    EEPROM.end();
    Serial.println("Credential saved.");
}
