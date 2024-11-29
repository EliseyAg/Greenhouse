#include "Memory.hpp"

#include <EEPROM.h>


void Memory::init()
{
  EEPROM.begin(64);
}

void Memory::commit()
{
  EEPROM.commit();
}

void Memory::putString(int byte, std::string str)
{
  EEPROM.put(byte, str.c_str());
}

std::string Memory::getString(int byte)
{
  const char* str = "";
  EEPROM.get(byte, str);

  std::string _str = std::string(str);
  return _str;
}
