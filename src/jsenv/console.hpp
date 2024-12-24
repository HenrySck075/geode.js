#ifndef __puppeteer_console__
#define __puppeteer_console__

#include <Geode/loader/Event.hpp>

class ConsoleEvent : public geode::Event {
public:
  std::string type;

  ConsoleEvent(std::string type) : type(type) {};
};

#endif
