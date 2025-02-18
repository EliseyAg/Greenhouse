#include "WifiManager.hpp"
#include "Memory.hpp"

#include <Arduino.h>

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <esp_wifi.h>

bool server_started;

#define MAX_CLIENTS 4
#define WIFI_CHANNEL 6

const IPAddress localIP(4, 3, 2, 1);		   // the IP address the web server
const IPAddress gatewayIP(4, 3, 2, 1);
const IPAddress subnetMask(255, 255, 255, 0);

const String localIPURL = "http://4.3.2.1";
const char *cap_ssid = "GreenHouse";
const char *cap_pass = NULL;

DNSServer dnsServer;
AsyncWebServer server(80);

std::string WifiManager::ssid;
std::string WifiManager::pass;

std::string WifiManager::ntpServer = "ru.pool.ntp.org";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, WifiManager::ntpServer.c_str());

WiFiClass WifiManager::My_WiFi;

void setUpDNSServer(DNSServer &dnsServer, const IPAddress &localIP)
{
// Define the DNS interval in milliseconds between processing DNS requests
#define DNS_INTERVAL 30

	// Set the TTL for DNS response and start the DNS server
	dnsServer.setTTL(3600);
	dnsServer.start(53, "*", localIP);
}

void startSoftAccessPoint(const char *ssid, const char *password, const IPAddress &localIP, const IPAddress &gatewayIP)
{
// Define the maximum number of clients that can connect to the server
#define MAX_CLIENTS 4
// Define the WiFi channel to be used (channel 6 in this case)
#define WIFI_CHANNEL 6

	// Set the WiFi mode to access point and station
	WiFi.mode(WIFI_MODE_AP);

	// Define the subnet mask for the WiFi network
	const IPAddress subnetMask(255, 255, 255, 0);

	// Configure the soft access point with a specific IP and subnet mask
	WiFi.softAPConfig(localIP, gatewayIP, subnetMask);

	// Start the soft access point with the given ssid, password, channel, max number of clients
	WiFi.softAP(ssid, password, WIFI_CHANNEL, 0, MAX_CLIENTS);

	// Disable AMPDU RX on the ESP32 WiFi to fix a bug on Android
	esp_wifi_stop();
	esp_wifi_deinit();
	wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
	my_config.ampdu_rx_enable = false;
	esp_wifi_init(&my_config);
	esp_wifi_start();
	vTaskDelay(100 / portTICK_PERIOD_MS);  // Add a small delay
}

void setUpWebserver(AsyncWebServer &server, const IPAddress &localIP)
{
	// Required
	server.on("/connecttest.txt", [](AsyncWebServerRequest *request) { request->redirect("http://logout.net"); });
	server.on("/wpad.dat", [](AsyncWebServerRequest *request) { request->send(404); });								// Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)

	// Background responses: Probably not all are Required, but some are. Others might speed things up?
	// A Tier (commonly used by modern systems)
	server.on("/generate_204", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });		   // android captive portal redirect
	server.on("/redirect", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });			   // microsoft redirect
	server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });  // apple call home
	server.on("/canonical.html", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });	   // firefox captive portal call home
	server.on("/success.txt", [](AsyncWebServerRequest *request) { request->send(200); });					   // firefox captive portal call home
	server.on("/ncsi.txt", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });			   // windows call home

	// B Tier (uncommon)
	//  server.on("/chrome-variations/seed",[](AsyncWebServerRequest *request){request->send(200);}); //chrome captive portal call home
	//  server.on("/service/update2/json",[](AsyncWebServerRequest *request){request->send(200);}); //firefox?
	//  server.on("/chat",[](AsyncWebServerRequest *request){request->send(404);}); //No stop asking Whatsapp, there is no internet connection
	//  server.on("/startpage",[](AsyncWebServerRequest *request){request->redirect(localIPURL);});

	// return 404 to webpage icon
	server.on("/favicon.ico", [](AsyncWebServerRequest *request) { request->send(404); });	// webpage icon

	// Serve Basic HTML Page
	server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
		AsyncWebServerResponse *response = request->beginResponse(200, "text/html", index_html);
		response->addHeader("Cache-Control", "public,max-age=31536000");  // save this file to cache for 1 year (unless you refresh)
		request->send(response);
		Serial.println("Served Basic HTML Page");
	});

  server.addHandler(new CaptivePortalHandler()).setFilter(ON_AP_FILTER);

	// the catch all
	server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
		request->redirect(localIPURL);
		Serial.print("onnotfound ");
		Serial.print(request->host());	// This gives some insight into whatever was being requested on the serial monitor
		Serial.print(" ");
		Serial.print(request->url());
		Serial.print(" sent redirect to " + localIPURL + "\n");
	});
}

void WifiManager::addWiFi()
{
  startSoftAccessPoint(cap_ssid, cap_pass, localIP, gatewayIP);

	setUpDNSServer(dnsServer, localIP);

	setUpWebserver(server, localIP);
	server.begin();
  server_started = true;
}

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

  WiFi.begin(ssid.c_str(), pass.c_str());
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
  if (server_started)
  {
    dnsServer.processNextRequest();
  }
}


std::string WifiManager::getTime()
{
  return std::string(timeClient.getFormattedTime().c_str());
}
