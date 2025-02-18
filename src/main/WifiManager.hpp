#pragma once
#include <string>
#include "WiFi.h"
#include <ESPAsyncWebServer.h>


const char index_html[] PROGMEM = R"=====(
  <!DOCTYPE html> <html>
    <head>
      <title>ESP32 Captive Portal</title>
      <style>
        
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
    </head>
    <body>
      <h1>Hello World!</h1>
      <form>
        <p><label>Имя сети: </label><input type="text" name="ssid"></p>
        <p><label>Пароль:   </label><input type="text" name="pass"></p>
        <p><input type="submit" value="Подтвердить"></p>
      </form>
    </body>
  </html>
)=====";

class CaptivePortalHandler : public AsyncWebHandler
{
public:
  CaptivePortalHandler() {}
  virtual ~CaptivePortalHandler() {}

  bool canHandle(AsyncWebServerRequest *request)
  {
    return request->url() == "/";
  }

  void handleRequest(AsyncWebServerRequest *request)
  {
    if (request->method() == HTTP_GET && request->url() == "/")
    {
      request->send(200, "text/html", index_html);
    }
    else
    {
      request->send(200, "text/html", index_html);
    }
  }
};


class WifiManager
{
public:
  static int init();
  static void on_update();

  static wl_status_t get_status() { return My_WiFi.status(); }
  static std::string getTime();

  static void addWiFi();

  static std::string ntpServer;
  static std::string ssid;
  static std::string pass;
  
private:
  static WiFiClass My_WiFi;
};
