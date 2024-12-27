#pragma once

#include <Geode/cocos/include/ccMacros.h>

#ifdef GEODE_IS_WINDOWS
    #ifdef JavaScript_EXPORTS
        #define javascript_DLL __declspec(dllexport)
    #else
        #define javascript_DLL __declspec(dllimport)
    #endif
#else 
    #define javascript_DLL __attribute__((visibility("default")))
#endif

