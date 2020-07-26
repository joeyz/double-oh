#include <iostream>
#include <chrono>
#include <ctime> 

#ifndef ROUTERMETHODS_HPP
#define ROUTERMETHODS_HPP
void handleRoot() {
  String page = "";
  File file = SPIFFS.open("/index.html", FILE_READ);
  server.streamFile(file, "text/html");
  file.close();
}

void handleAdmin() {
  String page = "";
  scriptFile(eeprom);
  File file = SPIFFS.open("/verysecureadminpanel.html", FILE_READ);
  server.streamFile(file, "text/html");
  file.close();
}

void handleGuestbook() {
  String guestbookJS = "const guestbook = [";
  File file = SPIFFS.open("/guestbook.txt", FILE_READ);
  while(file.available()) {
    guestbookJS += file.readString();
  }
  file.close();
  guestbookJS += "];";
  server.send(200, "text/javascript", guestbookJS);
}

void handleGuestbookPost() {
  // Get arguments and values, as well as current time
  char guestbookName[30];
  server.arg("guestbookName").toCharArray(guestbookName, 30);
  char guestbookComment[60];
  server.arg("guestbookComment").toCharArray(guestbookComment, 60);

  // Sanitize by removing any backticks and dollar signs
  auto filter1 = std::remove(std::begin(guestbookName), std::end(guestbookName), '`');
  auto filter2 = std::remove(std::begin(guestbookName), std::end(guestbookName), '{');
  auto filter3 = std::remove(std::begin(guestbookComment), std::end(guestbookComment), '`');
  auto filter4 = std::remove(std::begin(guestbookComment), std::end(guestbookComment), '{');
  
  Serial.println(guestbookName);
  Serial.println(guestbookComment);
  
  // Append to the js file that generates the guestbook
  File file = SPIFFS.open("/guestbook.txt", FILE_APPEND);
  file.print("[`");
  file.print(guestbookName);
  file.print("`,`");
  file.print(guestbookComment);
  file.println("`],");
  file.close();
  
//  file = SPIFFS.open("/guestbook.txt", FILE_READ);
//  while(file.available()) {
//    Serial.write(file.read());
//  }
//  file.close();

  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleSettingsPost() {
  // Get arguments and values
  // Check args that changed value
  byte newNetworkType = (byte)server.arg("networkType").toInt();
  byte newIpAddr1 = (byte)server.arg("ipAddr1").toInt();
  byte newIpAddr2 = (byte)server.arg("ipAddr2").toInt();
  char newSsid[31];
  server.arg("networkSSID").toCharArray(newSsid, 31);
  char newPsk[31];
  server.arg("networkPSK").toCharArray(newPsk, 31);
  char newDeviceName[16];
  server.arg("deviceName").toCharArray(newDeviceName, 16);
  bool changed = false;

  // TODO: check that all values are valid; disregard invalid ones or change them to closest valid ones

  // If any of the non-password values changed, update EEPROM and eeprom** variables
  if ((newNetworkType != eeprom.wifiType) ||
  (newIpAddr1 != eeprom.ip1) || 
  (newIpAddr2 != eeprom.ip2) || 
  (newDeviceName != eeprom.deviceName)) {
    changed = true;

    prefs.begin("doubleoh", false);

    eeprom.wifiType = newNetworkType;
    prefs.putChar("wifiType", newNetworkType);
    eeprom.ip1 = newIpAddr1;
    prefs.putChar("IP1", newIpAddr1);
    eeprom.ip2 = newIpAddr2;
    prefs.putChar("IP2", newIpAddr2);
    // eepromDeviceName = newDeviceName;
    strncpy(eeprom.deviceName, newDeviceName, 16);
    prefs.putBytes("deviceName", newDeviceName, 16);

    prefs.end();
  }

  // If either SSID or PSK changed, update both PSK and SSID
  if ((newSsid != eeprom.ssid) ||
  (newPsk != eeprom.psk && newPsk[0] != 0x00 && newPsk != NULL)) {
    changed = true;

    prefs.begin("doubleoh", false);

    // eepromSsid = newSsid;
    strncpy(eeprom.ssid, newSsid, 31);
    prefs.putBytes("SSID", newSsid, 31);
    // eepromPsk = newPsk;
    strncpy(eeprom.psk, newPsk, 31);
    prefs.putBytes("PSK", newPsk, 31);

    prefs.end();
  }

  if (changed) {
    printStatus(eeprom);
  }
  
  // Great but now which page to render
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleNotFound(){
  String message = "Hello! File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleResetPost() {
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
  ESP.restart();
}

void handleJS(String filename) {
  File file = SPIFFS.open(filename, "r");
  server.streamFile(file, "text/javascript");
  file.close();
}

void handleFavicon() {
  File file = SPIFFS.open("/favicon.ico", "r");
  server.streamFile(file, "image/x-icon");
  file.close();
}
#endif