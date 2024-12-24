#include "../../../../external/tinyjs/TinyJS.hpp"
#include "../state.hpp"
#include <Geode/DefaultInclude.hpp>
#include <Geode/Modify.hpp>
#include "../../DOM.hpp"
#include <queue>

struct idk : geode::Modify<idk, cocos2d::CCNode> {
  struct Fields {
    bool retainedByJS = false;
  };
  void retain() {
    if (!m_fields->retainedByJS) 
      cocos2d::CCNode::retain();
    m_fields->retainedByJS = false;
  }
};

static void finalize_Node(CScriptVar* v) {
  auto n = static_cast<idk*>(v->getUserData());
  if (n->m_fields->retainedByJS) {
    n->release();
  }
};

static CScriptVarLinkPtr createNodeObjFrom(cocos2d::CCNode* n) {
  auto nodeobj = getState()->evaluateComplex("new Node()");
  static_cast<cocos2d::CCNode*>(nodeobj->getVarPtr()->getUserData())->release();
  nodeobj->getVarPtr()->setUserData(n);
  return nodeobj;
}

static void returnNode(CFunctionsScopePtr const& v, cocos2d::CCNode* n) {
  v->setUserData(createNodeObjFrom(n)->getVarPtr().getVar());
}
//////
/// Properties
//////
#pragma region Properties

$jsMethod(Node_childList_g) {
  auto arr = newScriptVar(getState(), Array);
  int idx = 0;
  for (auto* c : geode::cocos::CCArrayExt<cocos2d::CCNode*>(
                  static_cast<cocos2d::CCNode*>(v->getArgument("this")->getUserData())
                  ->getChildren()
                )
  ) {
    arr->setArrayIndex(idx,createNodeObjFrom(c));
    idx++;
  }
  v->setReturnVar(arr);
}

$jsMethod(Node_isConnected_g) {
  v->setReturnVar(
    newScriptVarBool(
      getState(),
      static_cast<cocos2d::CCNode*>(
        v->getArgument("this")->getUserData()
      )->isRunning() // this is usually the way to determine if its connected to the scene
                     // but you know you can never trust a modder
    )
  );
}
$jsMethod(Node_isSameNode) {
  v->setReturnVar(
    newScriptVarBool(
      getState(),

      // check if these 2 variables points to the same address
      v->getArgument("this")->getUserData()
      ==
      v->getArgument("node")->getUserData()
    )
  );
}

$jsMethod(Node_firstChild_g) {
  auto node = static_cast<cocos2d::CCNode*>(v->getArgument("this")->getUserData());
  auto cl = node->getChildren();
  if (cl == nullptr || cl->count()==0) goto retnull; 
  returnNode(v, static_cast<cocos2d::CCNode*>(cl->firstObject())); return;
retnull:
  v->setReturnVar(newScriptVarNull(getState()));
  return;
}

$jsMethod(Node_lastChild_g) {
  auto node = static_cast<cocos2d::CCNode*>(v->getArgument("this")->getUserData());
  auto cl = node->getChildren();
  if (cl == nullptr || cl->count()==0) goto retnull; 
  returnNode(v, static_cast<cocos2d::CCNode*>(cl->lastObject())); return;
retnull:
  v->setReturnVar(newScriptVarNull(getState()));
  return;
}


$jsMethod(Node_nextSibling_g) {
  auto node = static_cast<cocos2d::CCNode*>(v->getArgument("this")->getUserData());
  if (auto p = node->getParent()) {
    auto cl = p->getChildren();
    auto i = cl->indexOfObject(node);
    if (i == cl->count()-1) goto retnull; 
    returnNode(v, static_cast<cocos2d::CCNode*>(cl->objectAtIndex(i+1))); return;
  }
retnull:
  v->setReturnVar(newScriptVarNull(getState()));
  return;
}

$jsMethod(Node_previousSibling_g) {
  auto node = static_cast<cocos2d::CCNode*>(v->getArgument("this")->getUserData());
  if (auto p = node->getParent()) {
    auto cl = p->getChildren();
    auto i = cl->indexOfObject(node);
    if (i == 0) goto retnull; 
    returnNode(v, static_cast<cocos2d::CCNode*>(cl->objectAtIndex(i-1))); return;
  }
retnull:
  v->setReturnVar(newScriptVarNull(getState()));
  return;
}


$jsMethod(Node_textContent_g) {
  auto node = static_cast<cocos2d::CCNode*>(v->getArgument("this")->getUserData());
  cocos2d::CCLabelProtocol* textNode = geode::cast::typeinfo_cast<cocos2d::CCLabelTTF*>(node);
  if (textNode == nullptr) 
    textNode = geode::cast::typeinfo_cast<cocos2d::CCLabelBMFont*>(node);
  if (textNode == nullptr) goto retnull;
  v->setReturnVar(newScriptVar(getState(), textNode->getString()));
retnull:
  v->setReturnVar(newScriptVarNull(getState()));
  return;
}
$jsMethod(Node_textContent_s) {
  if (!v->getArgument(1)->isString()) goto ret;
  {
    auto node = static_cast<cocos2d::CCNode*>(v->getArgument("this")->getUserData());
    cocos2d::CCLabelProtocol* textNode = geode::cast::typeinfo_cast<cocos2d::CCLabelTTF*>(node);
    if (textNode == nullptr) 
      textNode = geode::cast::typeinfo_cast<cocos2d::CCLabelBMFont*>(node);
    if (textNode == nullptr) goto ret;
    textNode->setString(v->getArgument(1)->toString().c_str());
  }
ret:
  v->setReturnVar(newScriptVarUndefined(getState()));
  return;
}
#pragma endregion

//////
/// Functions
//////
#pragma region Functions
$jsMethod(new_Node) {
  cocos2d::CCNode* n;
  // creates a new one
  n = cocos2d::CCNode::create();
  // keep it until this object gets removed
  n->retain();
  static_cast<idk*>(n)->m_fields->retainedByJS = true;
  auto obj = newScriptVar(getState(), Object);
  obj->setUserData(n);
};
#define $getNodeOfVariable2(n, var) \
  cocos2d::CCNode* n = nullptr; \
  { \
    auto vava = var; \
    n = vava->isObject() && vava->getUserData() != nullptr  \
      ? static_cast<cocos2d::CCNode*>(vava->getUserData())  \
      : nullptr; \
    if (!n) { \
      v->throwError(ReferenceError, "Not a valid Node object."); \
      return; \
    } \
  } 
#define $getNodeOfVariable(var) $getNodeOfVariable2(n, var)

$jsMethod(Node_appendChild) {
  $getNodeOfVariable(v->getArgument("node"));
  static_cast<idk*>(v->getArgument("this")->getUserData())->addChild(n);
  v->setReturnVar(newScriptVarUndefined(getState()));
}
$jsMethod(Node_removeChild) {
  $getNodeOfVariable(v->getArgument("node"));
  static_cast<idk*>(v->getArgument("this")->getUserData())->removeChild(n);
  v->setReturnVar(newScriptVarUndefined(getState()));
}
$jsMethod(Node_replaceChild) {
  $getNodeOfVariable2(oldNode, v->getArgument("oldChild"));
  $getNodeOfVariable2(newNode, v->getArgument("newChild"));
  auto thisNode = static_cast<cocos2d::CCNode*>(v->getArgument("this")->getUserData());
  auto children = thisNode->getChildren();
  int insertIndex = -1;
  if (children) {
    insertIndex = children->count()-1;
    if (children->containsObject(oldNode)) {
      insertIndex = children->indexOfObject(oldNode);
      children->removeObjectAtIndex(insertIndex);
    }
  }
  if (insertIndex == -1) {
    thisNode->addChild(newNode);
  } else {
    children->insertObject(newNode,insertIndex);
  }
  v->setReturnVar(newScriptVarUndefined(getState()));
}
$jsMethod(Node_insertBefore) {
  $getNodeOfVariable(v->getArgument("node"));
  auto rnv = v->getArgument("refNode");
  if (rnv->isNull())
    static_cast<idk*>(v->getArgument("this")->getUserData())->addChild(n);
  else if (rnv->isObject())
    static_cast<idk*>(
      v->getArgument("this")->getUserData()
    )->insertBefore(
      n, 
      static_cast<idk*>(rnv->getUserData())
    );

  v->setReturnVar(newScriptVarUndefined(getState()));
}

$jsMethod(Node_contains) {
  auto nodeVar = v->getArgument("node");
  if (nodeVar->isUndefined()) {
    v->throwError(ERROR_TYPES::Error, "node is required vro");
    return;
  }
  if (nodeVar->isNull()) {
    v->setReturnVar(newScriptVarBool(getState(), false));
    return;
  }
  auto thisNode = static_cast<idk*>(v->getArgument("this")->getUserData());
  auto targetNode = static_cast<idk*>(nodeVar->getUserData());
  if (thisNode == targetNode) {
    v->setReturnVar(newScriptVarBool(getState(), true));
    return;
  }

  std::queue<cocos2d::CCNode*> q;
  q.push(thisNode);
  while (!q.empty()) {
    auto cNode = q.front();

    for (auto c : geode::cocos::CCArrayExt<cocos2d::CCNode*>(cNode->getChildren())) {
      if (c == targetNode) {
        v->setReturnVar(newScriptVarBool(getState(), true));
        return;
      }
      q.push(c);
    }

    q.pop();
  }

  v->setReturnVar(newScriptVarBool(getState(), false));
}

$jsMethod(Node_getRootNode) {
  auto thisNode = static_cast<idk*>(v->getArgument("this")->getUserData());

  cocos2d::CCNode* lp = thisNode;
  cocos2d::CCNode* p = thisNode->getParent();
  while (p) {
    lp = p;
    p = p->getParent();
  }

  returnNode(v, lp); return;
}
$jsMethod(Node_getChildById) {
  auto thisNode = static_cast<idk*>(v->getArgument("this")->getUserData());
  auto nodeId = v->getArgument("id")->toString();

  auto node = thisNode->getChildByID(nodeId);
  if (node==nullptr) {
    v->throwError(Error, "No such child with ID \""+nodeId+"\"");
    return;
  }

  returnNode(v, node); return;
}

$jsMethod(Node_hasChildNodes) {
  v->setReturnVar(
    newScriptVarBool(
      getState(),
      static_cast<cocos2d::CCNode*>(
        v->getArgument("this")->getUserData()
      )->getChildrenCount() != 0
    )
  );
}
#pragma endregion

extern "C" void registerDOMNodeObject() {
  auto s = getState();
  auto node = s->addNative("function Node()", new_Node,0,SCRIPTVARLINK_CONSTANT);
  auto proto = node->findChild(TINYJS_PROTOTYPE_CLASS)->getVarPtr();
  proto->addChild(TINYJS_CONSTRUCTOR_VAR, node);
  {
    proto->addChild("childList", newScriptVarAccessor(s, Node_childList_g,0,&nothing,0));
    proto->addChild("firstChild", newScriptVarAccessor(s, Node_firstChild_g,0,&nothing,0));
    proto->addChild("isConnected", newScriptVarAccessor(s, Node_isConnected_g,0,&nothing,0));
    proto->addChild("lastChild", newScriptVarAccessor(s, Node_lastChild_g,0,&nothing,0));
    proto->addChild("nextSibling", newScriptVarAccessor(s, Node_nextSibling_g,0,&nothing,0));
    proto->addChild("previousSibling", newScriptVarAccessor(s, Node_previousSibling_g,0,&nothing,0));
    proto->addChild("textContent", newScriptVarAccessor(s, Node_textContent_g,0,Node_textContent_s,0));

    s->addNative("function Node.prototype.appendChild(node)", Node_appendChild);
    s->addNative("function Node.prototype.contains(node)", Node_contains);
    s->addNative("function Node.prototype.getChildById(id)", Node_getChildById);
    s->addNative("function Node.prototype.getRootNode(node)", Node_getRootNode);
    s->addNative("function Node.prototype.hasChildNodes(node)", Node_hasChildNodes);
    s->addNative("function Node.prototype.insertBefore(node, refNode)", Node_insertBefore);
    s->addNative("function Node.prototype.isSameNode(node)", Node_isSameNode);
    s->addNative("function Node.prototype.removeChild(node)", Node_removeChild);
    s->addNative("function Node.prototype.replaceChild(newChild, oldChild)", Node_replaceChild);
  }
  auto var = s->addNative("function Node.__constructor__()", new_Node, 0, SCRIPTVARLINK_CONSTANT);
  var->getFunctionData()->name = "Node";
}
