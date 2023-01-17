/**
 * ESP NTRIP Client
 *
 * Copyright 2023 by KLS <https://github.com/klstronics/ESPNTRIPClient>
 *
 * Licensed under GNU General Public License 3.0 or later. 
 * Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0 <http://www.gnu.org/licenses/>
 */

#include <FS.h>
#include <ArduinoJson.h>
#include "NTRIPClient.h"
#include <WiFiManager.h>
#ifdef ESP32
  #include <SPIFFS.h>
#else
  #include <ESP8266WiFi.h>
#endif


#ifdef ESP32
  #define RX 16
  #define TX 4
  #define CONFIGSWITCH 17
  #define LED 14
  #define LED_ON digitalWrite(LED, HIGH);
  #define LED_OFF digitalWrite(LED, LOW);
#else
  #define CONFIGSWITCH 5
  #define LED 2 
  #define LED_ON digitalWrite(LED, LOW);
  #define LED_OFF digitalWrite(LED, HIGH);
#endif


String ntriphost = "";
uint16_t ntripport = 2101;
String ntripmountpoint = "";
String ntripuser = "";
String ntrippassword = "";

NTRIPClient *ntrip;
WiFiManager wm;
bool saved = false;
String gga = "";
String configfile = "/ntrip.json";


void setup() {
  #ifdef ESP32
    Serial.begin(115200, SERIAL_8N1, RX, TX);
  #else
    Serial.begin(115200, SERIAL_8N1);
  #endif

  pinMode(CONFIGSWITCH, INPUT_PULLUP);
  #ifdef LED
    pinMode(LED, OUTPUT);
    LED_OFF
  #endif

  if (SPIFFS.begin()) {
    if (SPIFFS.exists(configfile)) {
      if (File file = SPIFFS.open(configfile, "r")) {
        StaticJsonDocument<512> doc;
        if (!deserializeJson(doc, file)) {
          ntriphost = String((const char*) doc["host"]);
          ntripport = String((const char*) doc["port"]).toInt();
          ntripmountpoint = String((const char*) doc["mountpoint"]);
          ntripuser = String((const char*) doc["user"]);
          ntrippassword = String((const char*) doc["password"]);
        }
        file.close();
      }
    }
  }
  else {
    SPIFFS.format();
    ESP.restart();
  }

  bool configured = wm.getWiFiIsSaved() && ntriphost.length() > 0 && ntripmountpoint.length() > 0;

  wm.setDebugOutput(false);
  wm.setConfigPortalTimeout(configured ? 25 : 0);
  wm.setCaptivePortalEnable(true);
  wm.setAPClientCheck(true);
  wm.setBreakAfterConfig(false);
  wm.setConfigPortalBlocking(true);
  wm.setDebugOutput(false);
  wm.setSaveParamsCallback(saveParamCallback);
    
  WiFiManagerParameter host("host", "NTRIP Host", ntriphost.c_str(), 32);
  WiFiManagerParameter port("port", "NTRIP Port", String(ntripport).c_str(), 6);
  WiFiManagerParameter mountpoint("mountpoint", "Mountpoint", ntripmountpoint.c_str(), 32);
  WiFiManagerParameter user("user", "User", ntripuser.c_str(), 32);
  WiFiManagerParameter password("password", "Password", ntrippassword.c_str(), 32);
  wm.addParameter(&host);
  wm.addParameter(&port);
  wm.addParameter(&mountpoint);
  wm.addParameter(&user);
  wm.addParameter(&password);
  
  if (!configured || digitalRead(CONFIGSWITCH) == HIGH) {
    wm.startConfigPortal("NTRIPConfig");
    wm.stopConfigPortal();
    wm.disconnect();
    if (saved) {
      ESP.restart();
    }
  }

  ntrip = new NTRIPClient(ntriphost, ntripport);
}


void loop() {
  connectWiFi();

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '$') {
      gga = c;
    }
    else {
      gga += c;
    }

    if (c == '\n' && gga.length() > 40) {
      if (!ntrip->connected()) {
        ntrip->startStream(ntripmountpoint, gga, ntripuser, ntrippassword);
      }
      else {
        ntrip->sendGGA(gga);
      }
      gga = "";    
    }
  }
  
  while (ntrip->available()) {
    Serial.write(ntrip->read());
  }
}


void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    LED_ON
    return;
  }
  LED_OFF
  
  WiFi.setHostname("NTRIPClient");
  WiFi.mode(WIFI_STA);
  WiFi.begin(wm.getWiFiSSID(true).c_str(), wm.getWiFiPass(true).c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  LED_ON
}


void saveParamCallback(){
  StaticJsonDocument<512> doc;

  doc["host"] = String(wm.server->arg("host"));
  doc["port"] = String(wm.server->arg("port"));
  doc["mountpoint"] = String(wm.server->arg("mountpoint"));
  doc["user"] = String(wm.server->arg("user"));
  doc["password"] = String(wm.server->arg("password"));

  File file = SPIFFS.open(configfile, "w");
  if (file) {
    serializeJson(doc, file);
    file.close();
  }
  
  saved = true;
  //serializeJson(doc, Serial);
}
