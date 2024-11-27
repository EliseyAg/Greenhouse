#pragma once
#include <string>
#include "WiFi.h"

class WifiManager
{
public:
  static void init();

  static wl_status_t get_status() { return Wifi.status(); }
  


  static std::string ssid;
  static std::string password;

  static WiFiClass Wifi;
};
