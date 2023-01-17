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

#include "NTRIPClient.h"
#include "base64.h"


NTRIPClient::NTRIPClient(String server, int port) {
  this->server = server;
  this->port = port;
}


String NTRIPClient::getMountPoints() {
  if (connected()) {
    return "already connected";
  }
  
  if (!connect(server.c_str(), port)) {
    return "connection failed";
  }  
  
  print("GET / HTTP/1.0\r\nUser-Agent: " + useragent + "\r\nAccept: */* \r\nConnection: close\r\n\r\n");

  String response;
  while (connected()) { 
    while (available()) {
      response += (char)read();
    }
  }
  
  stop();
  return response;
}


bool NTRIPClient::startStream(String mountpoint, String gga, String username, String password) {
  mountpoint.trim();
  username.trim();
  password.trim();
  
  if (connected()) {
    stop();
  }

  if (!connect(server.c_str(), port)) {
    return false;
  }

  print("GET /" + mountpoint + " HTTP/1.0\r\nUser-Agent: " + useragent + "\r\nAuthorization: Basic " + base64::encode(username + ":" + password) + "\r\n\r\n");
  sendGGA(gga);

  uint32_t timeout = millis() + 15000;
  while (!available()) {
    if (timeout < millis()) {
      stop();
      return false;
    }
    delay(10);
  }
  while (available()) {
    read();
  }

  return true;
}


void NTRIPClient::sendGGA(String gga) {
  gga.trim();
  if (gga.length() > 40) {
    print(gga + "\r\n");
  }
}
