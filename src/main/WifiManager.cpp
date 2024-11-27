#include "WifiManager.hpp"
#include "Memory.hpp"

std::string WifiManager::ssid = "";
std::string WifiManager::password = "";

WiFiClass WifiManager::Wifi;

void WifiManager::init()
{
  ssid = Memory::getString(16);
  password = Memory::getString(64);

  WiFi.begin(ssid.c_str(), password.c_str());
  uint32_t sec = millis() / 1000ul;

  while (WiFi.status() != WL_CONNECTED && sec <= 5)
  {
    sec = millis() / 1000ul;
  }
}
