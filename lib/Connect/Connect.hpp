
#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <M5Atom.h>
//#include <WiFi.h>
//#include <WebServer.h>
#include <ESPmDNS.h>
#include <AutoConnect.h>
using WiFiWebServer = WebServer;
#include <esp32-hal-log.h>

class Connect : public Task {
public:
  Connect() : _portal(_server),
              _hostName("DefaultHost"),
              _apName("DefaultAP"),
              _httpPort(80) {
    _content = String(R"(
            <!DOCTYPE html>
            <html>
            <head>
              <meta charset="UTF-8" name="viewport" content="width=device-width, initial-scale=1">
            </head>
            <body>
            Place the root page with the sketch application.&ensp;
            __AC_LINK__
            </body>
            </html>)");
  }

  void setHostName(String name) {
    _hostName = name;
  }

  void setAPName(String name) {
    _apName = name;
  }

  void begin(void) {
    begin("", "");
  }

  void begin(const char* SSID, const char* PASSWORD) {
    // Responder of root page and apply page handled directly from WebServer class.
    _server.on("/", [&]() {
      _content.replace("__AC_LINK__", String(AUTOCONNECT_LINK(COG_16)));
      _server.send(200, "text/html", _content);
    });

    _config.autoReconnect = true;
    _config.apid          = _apName;
    _portal.config(_config);

    bool result = false;

    if (String(SSID).isEmpty() || String(PASSWORD).isEmpty()) {
      result = _portal.begin();
    } else {
      result = _portal.begin(SSID, PASSWORD);
    }

    if (result) {
      log_i("WiFi connected: %s", WiFi.localIP().toString().c_str());

      if (MDNS.begin(_hostName.c_str())) {
        MDNS.addService("http", "tcp", _httpPort);
        log_i("HTTP Server ready! Open http://%s.local/ in your browser\n", _hostName.c_str());
      } else
        log_e("Error setting up MDNS responder");
    } else {
      log_e("ESP32 can't connect to AP.");
      ESP.restart();
    }
  }

  void run(void* data) {
    while (1) {
      _portal.handleClient();
      delay(1);
    }
  }

private:
  WiFiWebServer     _server;
  AutoConnectConfig _config;
  AutoConnect       _portal;

  String   _content;
  String   _hostName;
  String   _apName;
  uint16_t _httpPort;
};
