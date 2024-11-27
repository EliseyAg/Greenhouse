#pragma once
#include <string>

class Memory
{
public:
  static void init();

  static void commit();

  static void putString(int byte, std::string str);
  static std::string getString(int byte);
};
