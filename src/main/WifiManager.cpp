#include "WifiManager.hpp"
#include "Memory.hpp"

#include <NTPClient.h>
#include <WiFiUdp.h>

std::string WifiManager::ssid = "";
std::string WifiManager::password = "";

std::string WifiManager::ntpServer = "ru.pool.ntp.org";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, WifiManager::ntpServer.c_str());

WiFiClass WifiManager::Wifi;

int WifiManager::init()
{
  //ssid = Memory::getString(0);
  //password = Memory::getString(32);

  ssid = "your ssid";
  password = "your password";

  WiFi.begin(ssid.c_str(), password.c_str());
  uint32_t sec = millis() / 1000ul;

  while (WiFi.status() != WL_CONNECTED && sec <= 5)
  {
    sec = millis() / 1000ul;
  }

  if (WiFi.status() != WL_CONNECTED)
    return 1;

  timeClient.begin();
  timeClient.setTimeOffset(10800);

  return 0;
}


void WifiManager::on_update()
{
  timeClient.update();
}


std::string WifiManager::getTime()
{
  return std::string(timeClient.getFormattedTime().c_str());
}
