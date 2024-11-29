#pragma once
#include <string>
#include "WiFi.h"


class WifiManager
{
public:
  static int init();
  static void on_update();

  static wl_status_t get_status() { return Wifi.status(); }  
  static std::string getTime();

  static std::string ssid;
  static std::string password;
  
  static std::string ntpServer;

  static WiFiClass Wifi;
};
