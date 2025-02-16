#include "Memory.hpp"

#include <EEPROM.h>


int Memory::init()
{
  return EEPROM.begin(128);
}

void Memory::commit()
{
  EEPROM.commit();
}

template<typename T>
const T &Memory::put(int b, T &t)
{
  EEPROM.put(b, t);
}
