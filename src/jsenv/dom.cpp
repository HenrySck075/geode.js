#include "Geode/DefaultInclude.hpp"
extern "C" void registerDOMNodeObject();
extern "C" void registerDOMSceneObject();

$execute {
  registerDOMNodeObject();
  registerDOMSceneObject();
}
