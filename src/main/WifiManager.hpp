#pragma once
#include <string>
#include "WiFi.h"


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
