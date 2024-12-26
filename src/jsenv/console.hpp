#ifndef __puppeteer_console__
#define __puppeteer_console__

#include <Geode/loader/Event.hpp>
#include "../../exports.h"

class javascript_DLL ConsoleEvent : public geode::Event {
public:
  std::string type;

  ConsoleEvent(std::string type) : type(type) {};
};

#endif
