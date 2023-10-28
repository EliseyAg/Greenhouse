#include "WiFi.h"

// подключение библиотек
#include <stdlib.h>
#include <SPI.h>
//#include <Wire.h>
#include <Adafruit_GFX.h>
#include <EncButton.h>
#include <SSD1306Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PCF8591.h>

#define DT 18
#define CLK 5
#define SW 19
#define SDA 21
#define SCL 22

#define relay4_pin 33
#define relay3_pin 25
#define relay2_pin 14
#define relay1_pin 27
#define relay0_pin 15

PCF8591 pcf8591(0x48, SDA, SCL);

#define PIN_PHOTO_SENSOR 13
#define PIN_SOIL_SENSOR 32     // what pin we're connected to

// создание объекта OLED
// адрес I2C - 0x3C
SSD1306Wire display(0x3c, SDA, SCL);

EncButton enc(2, 3, 4);

const char* ssid     = "Waflya";
const char* password = "!ontario@@";
#define NTP_OFFSET  10800 // In seconds (в секундах)
#define NTP_INTERVAL 60 * 1000    // In miliseconds (в миллисекундах)
#define NTP_ADDRESS  "ru.pool.ntp.org"
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

int n = 0;

float h, t;
float _h, _t;
int _tim, _l;
bool timlum = 0;
int _s;
bool timsoi = 0;

int timers1[2][3] = {{1, 0, 0}, {0, 0, 0}};
int timers2[2][3] = {{0, 0, 0}, {0, 0, 0}};
String timers[2][3] = {{"00", "00", "00"}, {"00", "00", "00"}};
String timers_str[2][3] = {{"00", "00", "00"}, {"00", "00", "00"}};
 
const int potPin = 2;

// показания
int potValue = 0;

int lum = 0;
int somo = 0;

//display

void displayDrawString(const char* str, int x, int y) {

  display.drawString(x, y, str);
}

void displayDrawString(String str, int x, int y) {

  display.drawString(x, y, str);
}

void displayDrawStringInt(const char* str, int a, int x, int y) {

  display.drawString(x, y, str);
  char buffer1[3];
  itoa(a, buffer1, 10);
  display.drawString(x + 70, y, buffer1);
}


// Wifi

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {

  displayDrawString("connected", 0, 0);

  // что-то делаем, например, снова вычисляем mac-адрес
  for (int i = 0; i < 6; i++) {
    //Serial.printf("%02X", info.sta_disconnected.mac[i]);
    if (i < 5)Serial.print(":");
  }

  Serial.println("\n------------");
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {

  displayDrawString("disconnected", 0, 0);

  // что-то делаем, например, снова вычисляем mac-адрес
  for (int i = 0; i < 6; i++) {
    //Serial.printf("%02X", info.sta_disconnected.mac[i]);
    if (i < 5)Serial.print(":");
  }

  Serial.println("\n------------");
}


//Encoder

int  GetEncoderTurns()   { enc.tick(); if (enc.turn()) return enc.dir(); }
int  GetEncoderSide()    { enc.tick(); if (enc.turn()) { if (enc.left()) return 1; else if(enc.right()) return 2; } else return 0; }
bool GetEncoderPress()   { enc.tick(); return enc.press();   }
bool GetEncoderRelease() { enc.tick(); return enc.release(); }
bool GetEncoderClick()   { enc.tick(); return enc.click();   }


//Data

void GetData() {

  //PCF8591::AnalogInput ai = pcf8591.analogReadAll();
  int ana = pcf8591.analogRead(AIN0);

  lum = analogRead(PIN_PHOTO_SENSOR);
  lum = map(lum, 0, 4096, 0, 255);
  
  somo = ana;
  somo = map(somo, 100, 255, 100, 0);
}


//Menu

void WriteData() {

  // ошибочные данные с датчика!
  if (isnan(h) || isnan(t)) { 
    displayDrawString("Failed to read from DHT", 0, 14);
  } 
  else {
    displayDrawStringInt("Humidity: ", h, 0, 14);
    displayDrawStringInt("Temperature: ", t, 0, 24);
  }

  displayDrawStringInt("Lighting: ", lum, 0, 34);
  displayDrawStringInt("Moisture: ", somo, 0, 44);
}

void WriteSettings() {

  WriteMenu();

  displayDrawStringInt("Humidity: ", _h, 0, 14);
  displayDrawStringInt("Temperature: ", _t, 0, 24);
  displayDrawString("Lum/time", 0, 34);
  if (timlum)  displayDrawString("lum", 50, 34);
  else         displayDrawString("time", 50, 34);
  if (!timlum) displayDrawString("Lighting: timer", 0, 44);
  else         displayDrawStringInt("Lighting: ", _l, 0, 44);
  displayDrawString("Soil/time", 0, 54);
  if (timsoi)  displayDrawString("soil", 50, 54);
  else         displayDrawString("time", 50, 54);

  bool a = 0;
  bool c = 0;
  int b = 0;

  enc.tick();
  if (enc.isClick())
  {
    a = !a;
  }

  while (a)
  {
    enc.tick();
    if (enc.isDouble())
    {
      a = !a;
    }
    if (enc.isTurn())
    {
      if (enc.isRight()) b++;
      else if (enc.isLeft()) b--;
      if (b > 5) b = 0;
      if (b < 0) b = 5;
    }
    if (enc.isClick())
    {
      c = !c;
    }

    display.clear();

    WriteMenu();
    if (b == 0) displayDrawStringInt("Humidity: ", _h, 0, 14);
    if (b == 1) displayDrawStringInt("Temperature: ", _t, 0, 14);
    if (b == 2) {
      displayDrawString("Value/time:", 0, 14);
      if (timlum) displayDrawString("value", 60, 14);
      else        displayDrawString("time", 60, 14);
    }
    if (b == 3 && !timlum) displayDrawString("Lighting: timer", 0, 14);
    if (b == 3 &&  timlum) displayDrawStringInt("Lighting: ", _l, 0, 14);
    if (b == 4) {
      displayDrawString("Value/time:", 0, 14);
      if (timsoi) displayDrawString("value", 60, 14);
      else        displayDrawString("time", 60, 14);
    }
    if (b == 5 && !timsoi) displayDrawString("Moisture: timer", 0, 14);
    if (b == 5 &&  timsoi) displayDrawStringInt("Moisture: ", _s, 0, 14);

    display.display();

    while (c)
    {
      enc.tick();
      if (enc.isClick())
      {
        c = !c;
      }
      if (enc.isTurn())
      {
        if (enc.isRight()) {
          if (b == 0) _h++;
          if (b == 1) _t++;
          if (b == 2) timlum = !timlum;
          if (b == 3) _l++;
          if (b == 2) timsoi = !timsoi;
          if (b == 3) _s++;
        }
        else if (enc.isLeft()) {
          if (b == 0) _h--;
          if (b == 1) _t--;
          if (b == 2) timlum = !timlum;
          if (b == 3) _l--;
          if (b == 4) timsoi = !timsoi;
          if (b == 5) _s--;
        }
      }
      display.clear();

      WriteMenu();
      if (b == 0) displayDrawStringInt("Humidity: ", _h, 0, 14);
      if (b == 1) displayDrawStringInt("Temperature: ", _t, 0, 14);
      if (b == 2) {
        displayDrawString("Value/time:", 0, 14);
        if (timlum) displayDrawString("value", 60, 14);
        else        displayDrawString("time", 60, 14);
      }
      if (b == 3 && !timlum) displayDrawString("Lighting: timer", 0, 14);
      if (b == 3 &&  timlum) displayDrawStringInt("Lighting: ", _l, 0, 14);
      if (b == 4) {
        displayDrawString("Value/time:", 0, 14);
        if (timsoi) displayDrawString("value", 60, 14);
        else        displayDrawString("time", 60, 14);
      }
      if (b == 5 && !timsoi) displayDrawString("Moisture: timer", 0, 14);
      if (b == 5 &&  timsoi) displayDrawStringInt("Moisture: ", _s, 0, 14);
      
      display.display();
    }
  }

}

void WriteTimerSettings(int i)
{
  bool a = 0;
  while (a) {
    
  }
}

void WriteMenu()
{
  if (n == 0) displayDrawString("Menu", 1, 0);
  else if (n == 1) displayDrawString("Settings", 1, 0);
  else if (n == 2) displayDrawString("Humidity", 1, 0);
  else if (n == 3) displayDrawString("Temperature", 1, 0);
  else if (n == 4) displayDrawString("Lighting", 1, 0);
  else if (n == 5) displayDrawString("Moisture", 1, 0);
}

void WriteHum()
{
  bool a = 0;
  bool c = 0;
  int b = 0;
  
  displayDrawString("Timer:", 0, 14);
  if (timlum) {
    displayDrawString("on", 70, 14);
    displayDrawString("Value: ", 0, 24);
    displayDrawString("off", 70, 24);
  }
  else {
    displayDrawString("off", 70, 14);
    displayDrawStringInt("Value:", _h, 0, 24);
  }
  enc.tick();
  if (enc.isClick()) a = !a;
  while (a) {
    display.clear();
    
    if (b == 0) display.drawRect(69, 16, 15, 10);
    if (b == 1) display.drawRect(69, 26, 15, 10);
    displayDrawString("Timer:", 0, 14);
    displayDrawString("Timer:", 0, 14);
    if (timlum) {
      displayDrawString("on", 70, 14);
      displayDrawString("Value: ", 0, 24);
      displayDrawString("off", 70, 24);
    }
    else {
      displayDrawString("off", 70, 14);
      displayDrawStringInt("Value:", _h, 0, 24);
    }
    WriteMenu();

    display.display();

    enc.tick();
    if (enc.isDouble()) a = !a;
    if (enc.isClick()) c = !c;
    if (enc.isTurn())
      {
        if (enc.isRight()) b++;
        else if (enc.isLeft()) b--;
        if (b > 1) b = 0;
        if (b < 0) b = 1;
      }
    while (c) {
      display.clear();

      WriteMenu();
      if (b == 0) display.drawRect(69, 16, 15, 10);
      if (b == 1) display.drawRect(69, 26, 15, 10);
      displayDrawString("Timer:", 0, 14);
      if (timlum) {
        displayDrawString("on", 70, 14);
        displayDrawString("Value: ", 0, 24);
        displayDrawString("off", 70, 24);
      }
      else {
        displayDrawString("off", 70, 14);
        displayDrawStringInt("Value:", _h, 0, 24);
      }
      enc.tick();
      if (enc.isClick()) c = !c;
      if (b == 0) {
        if (enc.isTurn())
        {
          if (enc.isRight()) timlum = !timlum;
          else if (enc.isLeft()) timlum = !timlum;
        }
      }
      else {
        if (enc.isTurn())
        {
          if (enc.isRight()) _h++;
          else if (enc.isLeft()) _h--;
        }
      }
      display.display();
    }
  }
}

void WriteMainMenu()
{
  WriteData();
  //timeClient.update();
  //String formattedTime = timeClient.getFormattedTime();
  displayDrawString("Time:", 0, 54);
  //displayDrawString(formattedTime, 30, 54);
}


//Timers

void setTimer1() {

  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      char buffer[3];
      timers_str[i][j] = itoa(timers1[i][j], buffer, 10);
      if (timers_str[i][j] == "0") timers_str[i][j] = "00";
      if (timers_str[i][j] == "1") timers_str[i][j] = "01";
      if (timers_str[i][j] == "2") timers_str[i][j] = "02";
      if (timers_str[i][j] == "3") timers_str[i][j] = "03";
      if (timers_str[i][j] == "4") timers_str[i][j] = "04";
      if (timers_str[i][j] == "5") timers_str[i][j] = "05";
      if (timers_str[i][j] == "6") timers_str[i][j] = "06";
      if (timers_str[i][j] == "7") timers_str[i][j] = "07";
      if (timers_str[i][j] == "8") timers_str[i][j] = "08";
      if (timers_str[i][j] == "9") timers_str[i][j] = "09";
    }
  }
}

void setTimer2() {

  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      char buffer[3];
      timers_str[i][j] = itoa(timers2[i][j], buffer, 10);
      if (timers_str[i][j] == "0") timers_str[i][j] = "00";
      if (timers_str[i][j] == "1") timers_str[i][j] = "01";
      if (timers_str[i][j] == "2") timers_str[i][j] = "02";
      if (timers_str[i][j] == "3") timers_str[i][j] = "03";
      if (timers_str[i][j] == "4") timers_str[i][j] = "04";
      if (timers_str[i][j] == "5") timers_str[i][j] = "05";
      if (timers_str[i][j] == "6") timers_str[i][j] = "06";
      if (timers_str[i][j] == "7") timers_str[i][j] = "07";
      if (timers_str[i][j] == "8") timers_str[i][j] = "08";
      if (timers_str[i][j] == "9") timers_str[i][j] = "09";
    }
  }
}

void getTimer() {

  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      char buffer[3];
      timers[i][j] = itoa(timers1[i][j] - timers2[i][j], buffer, 10);
      if (timers[i][j] == "0") timers[i][j] = "00";
      if (timers[i][j] == "1") timers[i][j] = "01";
      if (timers[i][j] == "2") timers[i][j] = "02";
      if (timers[i][j] == "3") timers[i][j] = "03";
      if (timers[i][j] == "4") timers[i][j] = "04";
      if (timers[i][j] == "5") timers[i][j] = "05";
      if (timers[i][j] == "6") timers[i][j] = "06";
      if (timers[i][j] == "7") timers[i][j] = "07";
      if (timers[i][j] == "8") timers[i][j] = "08";
      if (timers[i][j] == "9") timers[i][j] = "09";
    }
  }
}

 
void setup() {
  _h = 40;
  _t = 30;
  _l = 50;
  _tim = 1;

  enc.setBtnLevel(LOW);
  enc.setClickTimeout(500);
  enc.setDencTimeout(50);
  enc.setHoldTimeout(600);
  enc.setStepTimeout(200);

  enc.setEncReverse(0);
  enc.setEncType(enc_STEP4_LOW);
  enc.setFastTimeout(30);

  // сбросить счётчик энкодера
  enc.counter = 0;
 
  Serial.begin(9600);

  pcf8591.begin();

  pinMode(PIN_PHOTO_SENSOR, INPUT);

  // инициализация OLED
  display.init();
  // установить ориентацию экрана
  display.flipScreenVertically();
  // установка шрифта и размера
  display.setFont(ArialMT_Plain_10);
  // расположение текста
  display.setTextAlignment(TEXT_ALIGN_LEFT);
#if 0 
  WiFi.begin(ssid, password);
  
  Serial.print("ESP Board MAC Address:  ");
 
  Serial.println(WiFi.macAddress());

  while (WiFi.status() != WL_CONNECTED)
  {
    display.clear();
    delay(500);
    displayDrawString("Please connect to internet.", 0, 0);
    display.display();
  }

  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  display.clear();

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  timeClient.begin();

  WiFi.softAP(ssid, password);
  WiFi.softAP(ssid); // без пароля
 
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  //WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_AP_STACONNECTED);
  //WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_AP_STADISCONNECTED);
#endif

  enc.setType(TYPE2);
  n = 0;

  pinMode(relay1_pin, OUTPUT);
  pinMode(relay2_pin, OUTPUT);
  pinMode(relay3_pin, OUTPUT);
  pinMode(relay4_pin, OUTPUT);
  pinMode(relay0_pin, OUTPUT);
  digitalWrite(relay1_pin, LOW);
  digitalWrite(relay2_pin, LOW);
  digitalWrite(relay3_pin, LOW);
  digitalWrite(relay4_pin, LOW);
  digitalWrite(relay0_pin, HIGH);
}
 
void loop() {
    
  // очистить дисплей
  display.clear();  

  WriteMenu();

  GetData();
  getTimer();

  enc.tick();
  if (enc.isTurn())
  {
    if (enc.isRight()) n++;
    else if (enc.isLeft()) n--;
  }
  if (n > 5) n = 0;
  else if (n < 0) n = 5;
  //Serial.println(n);

  if (n == 0) WriteMainMenu();
  else if (n == 1) WriteSettings();
  else if (n == 2) WriteHum();
  else if (n == 3) WriteHum();
  else if (n == 4) WriteHum();
  
  // вывести изображение из буфера на экран
  display.display();

  if (h < _h) { digitalWrite(relay2_pin, HIGH); }
  else { digitalWrite(relay2_pin, LOW); }

  if (t < _t) { digitalWrite(relay3_pin, HIGH); }
  else { digitalWrite(relay3_pin, LOW); }

  if (lum < _l) { digitalWrite(relay1_pin, HIGH); }
  else { digitalWrite(relay1_pin, LOW); }

}