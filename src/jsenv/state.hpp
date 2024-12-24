#ifndef __puppeteer_js__
#define __puppeteer_js__

#include "../../external/tinyjs/TinyJS.hpp"

CTinyJS* getState();

void nukeState();
#endif


#define $jsMethod(name) static void name(CFunctionsScopePtr const& v, void* userdata)

void nothing(CFunctionsScopePtr const &v, void *userdata);

