#pragma once

#ifdef GEODE_IS_WINDOWS
    #ifdef javascript_EXPORTS
        #define javascript_DLL __declspec(dllexport)
    #else
        #define javascript_DLL __declspec(dllimport)
    #endif
#else 
    #define javascript_DLL __attribute__((visibility("default")))
#endif
