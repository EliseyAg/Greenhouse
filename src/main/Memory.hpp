#pragma once
#include <string>
#include <Arduino.h>
#include <EEPROM.h>


class Memory
{
public:
  static int init();

  static void commit();

  template<typename T>
  static const T &put(int b, T &t);
  template<typename T>
  static T &get(int b, T &t) { EEPROM.get(b, t); }
};
