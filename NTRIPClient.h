/**
 * NTRIP Client
 *
 * Copyright 2023 by KLS <https://github.com/klstronics/ESPNTRIPClient>
 *
 * Licensed under GNU General Public License 3.0 or later. 
 * Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0 <http://www.gnu.org/licenses/>
 */

#ifndef _NTRIPCLIENT_H_
#define _NTRIPCLIENT_H_

#ifdef ESP8266
  #include "ESP8266WiFi.h"
#else
  #include "WiFi.h"
#endif


class NTRIPClient : protected WiFiClient {
private:
  String server;
  int port;
  const String useragent = "ESP32 NTRIP Client";
  String gga;

  NTRIPClient();

public:
  NTRIPClient(String server, int port);
  String getMountPoints();
  bool startStream(String mountpoint, String gga, String username, String password);
  void stopStream() { stop(); }
  void sendGGA(String gga);
  int available() { return WiFiClient::available(); }
  uint8_t connected() { return WiFiClient::connected(); }
  int read() { return WiFiClient::read(); }
};

#endif
