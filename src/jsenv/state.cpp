#include "state.hpp"
#include <cstddef>
#include <iostream>

CTinyJS* mumustate = nullptr;
CTinyJS* getState() {
  if (mumustate == nullptr) {
    mumustate = new CTinyJS();
  }
  return mumustate;
}

void nukeState() {
  delete mumustate;
};  

class bob {
public:
  /// @brief This is only ever be destructed on game close
  ~bob() {
    // shut up geode
    std::cout << "Closing MuJS state.";
    nukeState();
  }
};

static bob not_bob;

void nothing(CFunctionsScopePtr const &v, void *userdata) {
  v->setReturnVar(newScriptVarUndefined(mumustate));
}
