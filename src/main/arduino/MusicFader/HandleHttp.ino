#include "Global.h"

String createDropDown(String title, String name, int min, int max, int selectedValue) {
    String result;

    result += title;
    result += F("<select name='");
    result += name + F("'>");
    for (int i = min; i <= max; i++) {
        if (selectedValue == i) {
            result += F("<option selected value='");
        } else {
            result += F("<option value='");
        }

        result += i;
        result += F("'>");
        result += i;
        result += F("</option>");
    }
    result += F("</select>");

    return result;
}

/** Handle root or redirect to captive portal */
void handleRoot() {
    if (captivePortal()) {  // If captive portal redirect instead of displaying the page.
        return;
    }
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");

    String Page;
    Page += F("<!DOCTYPE html><html lang='en'><head>"
            "<meta name='viewport' content='width=device-width'>"
            "<title>CaptivePortal</title></head><body>"
            "<h1>HELLO WORLD!!</h1>");
    if (server.client().localIP() == apIP) {
        Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
    } else {
        Page += String(F("<p>You are connected through the wifi network: ")) + eepromSavedData.ssid + F("</p>");
    }
    Page += F("<p>You may want to <a href='/wifi'>config the wifi connection</a>.</p>"
            "</body></html>");

    server.send(200, "text/html", Page);
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
    if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
        Serial.println("Request redirected to captive portal");
        server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
        server.send(302, "text/plain", "");  // Empty content inhibits Content-length header so we have to close the socket ourselves.
        server.client().stop();              // Stop is needed because we sent no content length
        return true;
    }
    return false;
}

/** Wifi config page handler */
void handleWifi() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");

    String Page;
    Page += F("<!DOCTYPE html><html lang='en'><head>"
            "<meta name='viewport' content='width=device-width'>"
            "<title>CaptivePortal</title></head><body>"
            "<h1>Wifi config</h1>");
    Page += F("<style>body { font-size: 2em; }select  { font-size: 1em; }</style>");
    if (server.client().localIP() == apIP) {
        Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
    } else {
        Page += String(F("<p>You are connected through the wifi network: ")) + eepromSavedData.ssid + F("</p>");
    }
    Page += String(F("\r\n<br />"
                   "<table><tr><th align='left'>SoftAP config</th></tr>"
                   "<tr><td>SSID "))
          + String(softAP_ssid) + F("</td></tr>"
                                    "<tr><td>IP ")
          + toStringIp(WiFi.softAPIP()) + F("</td></tr>"
                                            "</table>"
                                            "\r\n<br />"
                                            "<table><tr><th align='left'>WLAN config</th></tr>"
                                            "<tr><td>SSID ")
          + String(eepromSavedData.ssid) + F("</td></tr>"
                             "<tr><td>IP ")
          + toStringIp(WiFi.localIP()) + F("</td></tr>"
                                           "</table>"
                                           "\r\n<br />"
                                           "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>");
    Serial.println("scan start");
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n > 0) {
        for (int i = 0; i < n; i++) { Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</td></tr>"); }
    } else {
        Page += F("<tr><td>No WLAN found</td></tr>");
    }

    String htmlSsid = "";
    htmlSsid += eepromSavedData.ssid;

    String htmlPassword = "";
    htmlPassword += eepromSavedData.password;

    Page += F("</table>"
            "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>");
    Page += F("<input type='text' placeholder='network' name='n' value='");
    Page += htmlSsid;
    Page += F("'/>");

    Page += F("<br />");

    Page += F("<input type='password' placeholder='password' name='p' value='");
    Page += htmlPassword;
    Page += F("'/>");
    Page += F("<br />");

    // Fader 1
    Page += F("<h2>Fader1</h2><br />");
    Page += createDropDown("Bus:",     "b1", 0, 6,  eepromSavedData.fader1Bus);
    Page += F("<br />");
    Page += createDropDown("Channel:", "c1", 0, 16, eepromSavedData.fader1Channel);
    Page += F("<br />");

    #ifdef USE_MUX
    Page += F("<h2>Fader2</h2><br />");
    Page += createDropDown("Bus:",     "b2", 0, 6,  eepromSavedData.fader2Bus);
    Page += F("<br />");
    Page += createDropDown("Channel:", "c2", 0, 16, eepromSavedData.fader2Channel);
    #endif

    Page += F("<br /><input type='submit' value='Save'/>");
    Page += F("</form></body></html>");
            
    server.send(200, "text/html", Page);
    server.client().stop();  // Stop is needed because we sent no content length
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave() {
    Serial.println("wifi save");
    server.arg("n").toCharArray(eepromSavedData.ssid, sizeof(eepromSavedData.ssid) - 1);
    server.arg("p").toCharArray(eepromSavedData.password, sizeof(eepromSavedData.password) - 1);

    eepromSavedData.fader1Bus = server.arg("b1").toInt();
    eepromSavedData.fader1Channel = server.arg("c1").toInt();
    eepromSavedData.fader2Bus = server.arg("b2").toInt();
    eepromSavedData.fader2Channel = server.arg("c2").toInt();

    server.sendHeader("Location", "wifi", true);
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send(302, "text/plain", "");  // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop();              // Stop is needed because we sent no content length
    saveCredentials();
    connect = strlen(eepromSavedData.ssid) > 0;  // Request WLAN connect with new credentials if there is a SSID
}

void handleNotFound() {
    if (captivePortal()) {  // If caprive portal redirect instead of displaying the error page.
    return;
    }
    String message = F("File Not Found\n\n");
    message += F("URI: ");
    message += server.uri();
    message += F("\nMethod: ");
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += F("\nArguments: ");
    message += server.args();
    message += F("\n");

    for (uint8_t i = 0; i < server.args(); i++) { message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n"); }
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send(404, "text/plain", message);
}
