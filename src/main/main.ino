#include "WifiManager.hpp"
#include "Memory.hpp"

// подключение библиотек
#include <string>
#include <Wire.h>
#include <stdlib.h>
#include <SPI.h>
#include <EncButton.h>
#include <GyverOLED.h>
#include <GyverBME280.h>
#include "driver/timer.h"

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

#define PIN_PHOTO_SENSOR 33

#define PCF8591 (0x90 >> 1) // I2C bus address
#define ADC0 0x00 // control bytes for reading individual ADCs
#define ADC1 0x01
#define ADC2 0x02
#define ADC3 0x03

#define BOT_TOKEN "6981923946:AAHQzU_c3QTfDY12RtkjhX2feldfavjdBDs"

#include <FastBot.h>

FastBot bot(BOT_TOKEN);

// создание объекта OLED, адрес I2C - 0x3C
GyverOLED<SSD1306_128x64, OLED_BUFFER> display;

// Создание обьекта датчика bme
GyverBME280 bme;

// Создание объекта энкодера 
EncButton enc(CLK, DT, SW);

//const char* ssid     = "ElisPOCO";
//const char* password = "00000000";

int8_t n = 0;

int8_t _t, temp; //значения температуры
uint8_t _h, hum; //значения влажности
uint8_t _l, lum; //освещенность
uint8_t _s, somo; //влажность почвы

bool timhum = 0;
bool timlum = 0;
bool timsoi = 0;

bool is_l = false; bool is_lr = false;
bool is_s = false; bool is_sr = false;
bool is_t = false; bool is_tr = false;
bool is_h = false; bool is_hr = false;

bool is_WiFi = false;

TaskHandle_t Task0;
TaskHandle_t Task1;
StaticQueue_t _dataSendQueueBuffer;

struct Data
{
  int8_t t; //значения температуры
  uint8_t h; //значения влажности
  uint8_t l; //освещенность
  uint8_t s; //влажность почвы
} setformem;

struct Timers
{
  uint64_t t; //значения температуры
  uint64_t h; //значения влажности
  uint64_t l; //освещенность
  uint64_t s; //влажность почвы
} timformem;

uint32_t sech0 = 0;
uint32_t sect0 = 0;
uint32_t secl0 = 0;
uint32_t secs0 = 0;

uint32_t sech1;
uint32_t sect1;
uint32_t secl1;
uint32_t secs1;

uint32_t secdis = 0;

uint8_t _dataSendQueueStorage[10 * sizeof(Data)];
QueueHandle_t _dataSendQueue = xQueueCreateStatic(10, sizeof(Data), &(_dataSendQueueStorage[0]), &_dataSendQueueBuffer);

String ID = "9044470632";
String myhosains[10] = {};
int lenhos = 0;

bool uder = 0;

void core0(void *p)
{
  byte value0, value1, value2, value3;
  Wire.begin();
  for(;;)
  {
    // Скорость шины  I2C повыше
    Wire.setClock(150000L);

    Wire.beginTransmission(PCF8591); // wake up PCF8591
    Wire.write(ADC0); // control byte - read ADC0
    Wire.endTransmission(); // end tranmission
    Wire.requestFrom(PCF8591, 2);
    value0=Wire.read();
    value0=Wire.read();
    Wire.beginTransmission(PCF8591); // wake up PCF8591
    Wire.write(ADC1); // control byte - read ADC1
    Wire.endTransmission(); // end tranmission
    Wire.requestFrom(PCF8591, 2);
    value1=Wire.read();
    value1=Wire.read();
    Wire.beginTransmission(PCF8591); // wake up PCF8591
    Wire.write(ADC2); // control byte - read ADC2
    Wire.endTransmission(); // end tranmission
    Wire.requestFrom(PCF8591, 2);
    value2=Wire.read();
    value2=Wire.read();
    Wire.beginTransmission(PCF8591); // wake up PCF8591
    Wire.write(ADC3); // control byte - read ADC3
    Wire.endTransmission(); // end tranmission
    Wire.requestFrom(PCF8591, 2);
    value3=Wire.read();
    value3=Wire.read();

    // Скорость шины  I2C повыше
    Wire.setClock(400000L);

    //Serial.print(value0); Serial.print(" ");
    //Serial.print(value1); Serial.print(" ");
    //Serial.print(value2); Serial.print(" ");
    //Serial.print(value3); Serial.print(" ");
    
    Data a;
    int light = 0;

    light += analogRead(PIN_PHOTO_SENSOR);
    vTaskDelay(pdMS_TO_TICKS(100));
    light += analogRead(PIN_PHOTO_SENSOR);
    vTaskDelay(pdMS_TO_TICKS(100));
    light += analogRead(PIN_PHOTO_SENSOR);
    vTaskDelay(pdMS_TO_TICKS(100));
    light += analogRead(PIN_PHOTO_SENSOR);
    vTaskDelay(pdMS_TO_TICKS(100));
    light += analogRead(PIN_PHOTO_SENSOR);
    vTaskDelay(pdMS_TO_TICKS(100));

    light = light / 5;
    a.l = map(light, 0, 4096, 0, 100);
    a.t = bme.readTemperature();
    a.h = bme.readHumidity();

    a.s = map(value3, 225, 0, 0, 100);
    xQueueSend(_dataSendQueue, &a, pdMS_TO_TICKS(1000));
    vTaskDelay(pdMS_TO_TICKS(4500));
  }
}

void tasklight(void *p)
{
  for (;;)
  {
    if (lum <= _l)  {
      vTaskDelay(120000); 
      if (lum <= _l)  {
        vTaskDelay(120000);
        if (lum <= _l)  {
          vTaskDelay(120000);
          if (lum <= _l)  {
            vTaskDelay(120000);
            if (lum <= _l)  {
              vTaskDelay(120000);
              if (lum <= _l)  {
                is_lr = true;
                vTaskDelay(secl1 * 1000);
                is_lr = false;
                vTaskDelay((secl1 * 3 - 600) * 1000);
              }
            }
          }
        }
      }
    }
    vTaskDelay(60000); 
  }
}

void tasksomo(void *p)
{
  for (;;)
  {
    if (lum <= _l)  {
      vTaskDelay(120000); 
      if (lum <= _l)  {
        vTaskDelay(120000);
        if (lum <= _l)  {
          vTaskDelay(120000);
          if (lum <= _l)  {
            vTaskDelay(120000);
            if (lum <= _l)  {
              vTaskDelay(120000);
              if (lum <= _l)  {
                is_lr = true;
                vTaskDelay(secl1 * 1000);
                is_lr = false;
                vTaskDelay((secl1 * 3 - 600) * 1000);
              }
            }
          }
        }
      }
    }
  }
}

void tasktemp(void *p)
{
  for (;;)
  {
    if (lum <= _l)  {
      vTaskDelay(120000); 
      if (lum <= _l)  {
        vTaskDelay(120000);
        if (lum <= _l)  {
          vTaskDelay(120000);
          if (lum <= _l)  {
            vTaskDelay(120000);
            if (lum <= _l)  {
              vTaskDelay(120000);
              if (lum <= _l)  {
                is_lr = true;
                vTaskDelay(secl1 * 1000);
                is_lr = false;
                vTaskDelay((secl1 * 3 - 600) * 1000);
              }
            }
          }
        }
      }
    }
  }
}

void taskhum(void *p)
{
  for (;;)
  {
    if (lum <= _l)  {
      vTaskDelay(120000); 
      if (lum <= _l)  {
        vTaskDelay(120000);
        if (lum <= _l)  {
          vTaskDelay(120000);
          if (lum <= _l)  {
            vTaskDelay(120000);
            if (lum <= _l)  {
              vTaskDelay(120000);
              if (lum <= _l)  {
                is_lr = true;
                vTaskDelay(secl1 * 1000);
                is_lr = false;
                vTaskDelay((secl1 * 3 - 600) * 1000);
              }
            }
          }
        }
      }
    }
  }
}

//display

void displayDrawString(const char* str, int x, int y) {
  display.setCursorXY(x, y);
  display.print(str);
}

void displayDrawString(String str, int x, int y) {
  display.setCursorXY(x, y);
  display.print(str);
}

void displayDrawStringInt(const char* str, int a, int x, int y) {
  display.setCursorXY(x, y);
  display.print(str);
  char buffer1[3];
  itoa(a, buffer1, 10);
  display.setCursorXY(x + 90, y);
  display.print(buffer1);
}

void displayDrawInt(int a, int x, int y) {
  display.setCursorXY(x, y);
  char buffer1[3];
  itoa(a, buffer1, 10);
  display.setCursorXY(x, y);
  display.print(buffer1);
}

//#if 0
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
//#endif

//Menu

void WriteData()
{

  displayDrawStringInt("Вл. воздухав: ", hum, 0, 16);
  displayDrawStringInt("Темп. воздуха: ", temp, 0, 28);

  displayDrawStringInt("Освещенность: ", lum, 0, 40);
  displayDrawStringInt("Вл. почвы: ", somo, 0, 52);
}

void WriteMenu()
{
  if (n == 0) displayDrawString("Меню", 1, 0);
  else if (n == 1) displayDrawString("Влажность воздуха", 1, 0);
  else if (n == 2) displayDrawString("Температура воздуха", 1, 0);
  else if (n == 3) displayDrawString("Освещенность", 1, 0);
  else if (n == 4) displayDrawString("Влажность почвы", 1, 0);
  else if (n == 5) displayDrawString("Wi-fi", 1, 0);
}

void DrawSettings(int a, int i)
{
  display.clear();
  WriteMenu();

  displayDrawString("Значение:", 0, 16);
  displayDrawInt(a, 90, 16);

  bool b = false;
  if (i == 0) { b = is_h; }
  if (i == 1) { b = is_h; }
  if (i == 2) { b = is_l; }
  if (i == 3) { b = is_s; }

  displayDrawString("Принудит. вкл:", 0, 28);
  if (b)
  {
    displayDrawString("вкл", 90, 28);
  }
  else
  {
    displayDrawString("выкл", 90, 28);
  }

  displayDrawString("Время работы:", 0, 40);
  if (i == 0) { displayDrawInt(sech1, 90, 40); }
  if (i == 1) { displayDrawInt(sect1, 90, 40); }
  if (i == 2) { displayDrawInt(secl1, 90, 40); }
  if (i == 3) { displayDrawInt(secs1, 90, 40); }
}

int WriteSettings(int a, int i)
{
  bool ir = 0;      // флаг выбора строки в меню настроек
  bool ir1 = 0;     // флаг настройки конкретного параметра
  int vibor = 0;    // номер строки для редактирования

  DrawSettings(a, i);

  if (enc.click()) { ir = 1; }

  while (ir)
  {
    enc.tick();

    DrawSettings(a, i);
    displayDrawString("Выход", 0, 52);

    switch (enc.action())
    {
      case EB_PRESS:
      break;
      case EB_CLICK:
      if (vibor == 0)
      {
        ir1 = 1;
        while (ir1 == 1)
        {
          enc.tick();
          switch (enc.action())
          {
            case EB_PRESS:
            break;
            case EB_CLICKS:
            if (enc.getClicks() == 2) { ir1 = 0; }
            break;
            case EB_HOLD:
            break;
            case EB_TURN:
            a -= enc.dir() * 5;
            break;
          }
          
          DrawSettings(a, i);
          display.update();
        }
      }
      if (vibor == 1)
      {
        if (i == 0) { is_h = !is_h; }
        if (i == 1) { is_t = !is_t; }
        if (i == 2) { is_l = !is_l; }
        if (i == 3) { is_s = !is_s; }
      }
      if (vibor == 2)
      {
        ir1 = 1;
        while (ir1 == 1)
        {
          enc.tick();
          switch (enc.action())
          {
            case EB_PRESS:
            break;
            case EB_CLICKS:
            if (enc.getClicks() == 2) { ir1 = 0; }
            break;
            case EB_HOLD:
            break;
            case EB_TURN:
            switch (i)
            {
              case 0:
              sech1 -= enc.dir() * 300;
              break;
              
              case 1:
              sect1 -= enc.dir() * 300;
              break;

              case 2:
              secl1 -= enc.dir() * 300;
              break;

              case 3:
              secs1 -= enc.dir() * 300;
              break;
            }
            break;
          }
          DrawSettings(a, i);
          display.update();
        }
      }
      else if (vibor == 3)
      {
        ir = 0;
      }
      break;
      case EB_HOLD:
      break;
      case EB_TURN:
      vibor -= enc.dir();
      break;
    }

    if (vibor > 4) vibor = 0;
    else if (vibor < 0) vibor = 4;

    if (vibor == 0)
    {
      display.line(88, 26, 128, 26);
    }
    else if (vibor == 1)
    {
      display.line(88, 38, 128, 38);
    }
    else if (vibor == 2)
    {
      display.line(88, 50, 128, 50);
    }
    else if (vibor == 3)
    {
      display.line(0,  62, 128, 62);
    }
    display.update();
    CheckRelay();
  }

  switch (i)
  {
    case 0:
    setformem.h = a;
    break;
    
    case 1:
    setformem.t = a;
    break;

    case 2:
    setformem.l = a;
    break;

    case 3:
    setformem.s = a;
    break;
  }

  switch (i)
  {
    case 0:
    timformem.h = sech1;
    break;
    
    case 1:
    timformem.t = sect1;
    break;

    case 2:
    timformem.l = secl1;
    break;

    case 3:
    timformem.s = secs1;
    break;
  }

  return a;
}

void WriteWifi()
{
  WriteMenu();

  displayDrawString("Подключено: ", 0, 16);
  displayDrawString(WifiManager::ssid.c_str(), 70, 16);
  displayDrawString("ID: ", 0, 28);
  displayDrawString(ID, 40, 28);
  displayDrawString("Время: ", 0, 40);
  displayDrawString(WifiManager::getTime().c_str(), 70, 40);
  displayDrawString("Нажмите чтобы изменить сеть", 0, 52);
  
  if (enc.click()) { WifiManager::addWiFi(); Serial.println("Pressed"); }
}

//relay

void CheckRelay()
{
  if (is_hr || is_h)  { digitalWrite(relay2_pin, HIGH); digitalWrite(relay0_pin, HIGH); }
  else if (!is_h)     { digitalWrite(relay2_pin, LOW); }

  if (is_tr || is_t)  { digitalWrite(relay3_pin, HIGH); digitalWrite(relay0_pin, HIGH); }
  else if (!is_t)     { digitalWrite(relay3_pin, LOW); }

  if (is_lr || is_l)  { digitalWrite(relay1_pin, HIGH); digitalWrite(relay0_pin, HIGH); }
  else if (!is_l)     { digitalWrite(relay1_pin, LOW); }
  
  if (is_sr || is_s)  { digitalWrite(relay4_pin, HIGH); digitalWrite(relay0_pin, HIGH); }
  else if (!is_s)     { digitalWrite(relay4_pin, LOW); }
  
  if (!is_hr && !is_tr && !is_lr && !is_sr && !is_h && !is_t && !is_l && !is_s) { digitalWrite(relay0_pin, LOW); }  //выключаем оптопару если в ней нет надобности
}

// обработчик сообщений

void newMsg(FB_msg& msg)
{
  // выводим ID чата, имя юзера и текст сообщения
  Serial.print(msg.chatID);     // ID чата 
  Serial.print(", ");
  Serial.print(msg.username);   // логин
  Serial.print(", ");
  Serial.println(msg.text);     // текст

  bool is_hosain = false;
  for (int i = 0; i < lenhos; i++)
  {
    if (msg.chatID == myhosains[i])
    {
      is_hosain = true; 
    }
  }

  if (is_hosain)
  {
    if (msg.text == "/data")
    {
      bot.replyMessage(String("Данные за ")     + WifiManager::getTime().c_str() +
                            "\nВлажность = "    + String(hum) + 
                            "\nТемпература = "  + String(temp) + 
                            "\nОсвещенность = " + String(lum) + 
                            "\nПочва = "        + String(somo), msg.messageID, msg.chatID);
    }
    if (msg.text == "/turnon_lamp")
    {
      is_l = true;
      bot.replyMessage("Лампа включилась.", msg.messageID, msg.chatID);
    }
    if (msg.text == "/turnoff_lamp")
    {
      is_l = false;
      bot.replyMessage("Лампа выключилась.", msg.messageID, msg.chatID);
    }
    uder = false;
  }
  else if (msg.text == "/start")
  {
    bot.replyMessage("Да стартуем уже!", msg.messageID, msg.chatID);
  }
  else if (msg.text == "/set_id")
  {
    uder = true;
    bot.replyMessage("Введите ID вашей теплицы.", msg.messageID, msg.chatID);
  }
  else if (uder)
  {
    if (msg.text == ID)
    {      
      if (!is_hosain) { myhosains[lenhos] = msg.chatID; lenhos++; bot.replyMessage("Вы добавлены в список хозяев.", msg.messageID, msg.chatID); }
    }
    uder = false;
  }
  else
  {
    bot.replyMessage("Иди отсюда, мамкин хакер, теплицей моей поуправлять решил? Да я тебя по IP вычислю!", msg.messageID, msg.chatID);
    uder = false;
  }
}

//base functions

void setup()
{
  Serial.begin(9600);
  delay(5000);

  EEPROM.begin(128);
  EEPROM.put(0, sizeof("Waflya"));
  EEPROM.put(8, sizeof("!ontario@@"));
  EEPROM.put(16, "Waflya");
  EEPROM.put(64, "!ontario@@");
  EEPROM.commit();

  // Пороговые значение для активации реле
  _h = setformem.h;
  _t = setformem.t;
  _l = setformem.l;
  _s = setformem.s;

  sech1 = timformem.h;
  sect1 = timformem.t;
  secl1 = timformem.l;
  secs1 = timformem.s;

  // Если доп. настройки не нужны  - инициализируем датчик
  bme.begin();

  //настройки энкодера
  enc.setBtnLevel(LOW);
  enc.setClickTimeout(500);
  //enc.setDencTimeout(50);
  enc.setHoldTimeout(600);
  enc.setStepTimeout(200);
  enc.setEncReverse(0);
  enc.setFastTimeout(30);

  // сбросить счётчик энкодера
  enc.counter = 0;

  pinMode(PIN_PHOTO_SENSOR, INPUT);

  // инициализация OLED
  display.init();
  // Скорость шины  I2C повыше
  Wire.setClock(400000L);
  // установка шрифта и размера
  display.setScale(1);

  xTaskCreatePinnedToCore(core0, "Task0", 10000, NULL, 1, &Task0, 0);
  xTaskCreatePinnedToCore(tasklight, "Task1", 1000, NULL, 1, &Task1, 0);

  Serial.println(WifiManager::init());

  Serial.println(WifiManager::get_status());
  Serial.println(WifiManager::ssid.c_str());
  Serial.println(WifiManager::pass.c_str());

  //if (WifiManager::get_status() == WL_CONNECTED)
  //{
    bot.attach(newMsg);
    is_WiFi = true;

    Serial.print("ESP Board MAC Address:  ");
    Serial.println(WiFi.macAddress());

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  //}

  display.clear();

  //WiFi.softAP(ssid, password);
  //WiFi.softAP(ssid); // без пароля
 
  //Serial.println();
  //Serial.print("IP address: ");
  //Serial.println(WiFi.softAPIP());

  //WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_AP_STACONNECTED);
  //WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_AP_STADISCONNECTED);

  // Настройка пинов для реле
  pinMode(relay1_pin, OUTPUT);
  pinMode(relay2_pin, OUTPUT);
  pinMode(relay3_pin, OUTPUT);
  pinMode(relay4_pin, OUTPUT);
  pinMode(relay0_pin, OUTPUT);
  digitalWrite(relay1_pin, LOW);
  digitalWrite(relay2_pin, LOW);
  digitalWrite(relay3_pin, LOW);
  digitalWrite(relay4_pin, LOW);
}

void loop()
{
  //if (WiFi.status() == WL_CONNECTED)
  //{
    // проверка на сообщения боту
    bot.tick();
  //}

  // очистить дисплей
  display.clear();  

  if (enc.tick())
  {
    switch (enc.action())
    {
      case EB_PRESS:
      break;
      case EB_HOLD:
      break;
      case EB_TURN:
      n -= enc.dir();
      break;
    }
    secdis = millis() / 1000ul;
    if (n > 5) n = 0;
    else if (n < 0) n = 5;
  }

  if (millis() / 1000ul - secdis <= 60)
  {
    WriteMenu();  
    if (n == 0) WriteData();
    else if (n == 1) _h = WriteSettings(_h, 0);
    else if (n == 2) _t = WriteSettings(_t, 1);
    else if (n == 3) _l = WriteSettings(_l, 2);
    else if (n == 4) _s = WriteSettings(_s, 3);
    else if (n == 5)      WriteWifi();
  }

  Data b;

  // Если в очереди есть данные, считываем
  if (xQueueReceive(_dataSendQueue, &b, 0) == pdTRUE)
  {
    lum = b.l;
    somo = b.s;
    temp = b.t;
    hum = b.h;
  }

  CheckRelay();
  
  // вывести изображение из буфера на экран
  display.update();

  WifiManager::on_update();
}
