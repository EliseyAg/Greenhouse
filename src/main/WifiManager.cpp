#include "WifiManager.hpp"
#include "Memory.hpp"

#include <NTPClient.h>
#include <WiFiUdp.h>


std::string WifiManager::ssid;
std::string WifiManager::pass;

std::string WifiManager::ntpServer = "ru.pool.ntp.org";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, WifiManager::ntpServer.c_str());

WiFiClass WifiManager::My_WiFi;

int WifiManager::init()
{
  int ssid_len = 10;
  int pass_len = 10;
  EEPROM.get(0, ssid_len);
  EEPROM.get(8, pass_len);
  const char c_ssid[32] = "";
  const char c_pass[32] = "";
  EEPROM.get(16, c_ssid);
  EEPROM.get(64, c_pass);  

  ssid = std::string(c_ssid);
  pass = std::string(c_pass);

  ssid = ssid.substr(0, ssid_len);
  pass = pass.substr(0, pass_len);

  My_WiFi.begin(ssid.c_str(), pass.c_str());
  uint32_t sec = millis() / 1000ul;

  while (My_WiFi.status() != WL_CONNECTED && sec <= 5)
  {
    sec = millis() / 1000ul;
  }

  if (My_WiFi.status() != WL_CONNECTED)
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
