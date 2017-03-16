#ifndef CONTEXT_CPP_
#define CONTEXT_CPP_

#include "context.hpp"

using namespace std;

namespace CoreIR {

Context::Context() : maxErrors(3) {
  global = newNamespace("_G");
  cache = new TypeCache(this);
}

// Order of this matters
Context::~Context() {
  
  for (auto it : genargsList) delete it;
  for (auto it : genargList) delete it;
  for (auto it : recordParamsList) delete it;
  for (auto it : genparamsList) delete it;
  for (auto it : libs) delete it.second;
  for (auto it : instanceArrays) free(it);
  // for (auto it : connectionArrays) free(it);
  for (auto it : wireableArrays) free(it);
 
  delete cache;
}



Namespace* Context::newNamespace(string name) { 
  Namespace* n = new Namespace(this,name);
  libs.emplace(name,n);
  return n;
}

Namespace* Context::getNamespace(string name) {
  auto it = libs.find(name);
  if (it == libs.end()) {
    Error e;
    e.message("Could Not Find Namespace");
    e.message("  Namespace : " + name);
    e.fatal();
    error(e);
    return nullptr;
  }
  return it->second;
}

// This tries to link all the definitions of def namespace to declarations of decl namespace
// This will clobber declns
bool Context::linkLib(Namespace* defns, Namespace* declns) {
  if (haserror()) {
    return true;
  }
  for (auto it : defns->getGenerators()) {
    Generator* gdef = (it.second);
    string gdefname = gdef->getName();
    assert(it.first == gdefname);
    genFun gdeffun = gdef->getDef();
    Generator* gdecl = declns->getGenerator(gdefname);
    
    //If def is not found in decl,
    //  make e.message?
    if (haserror() ) return true;
    
    genFun gdeclfun = gdecl->getDef();

    //case def is found in decl, but def is a decl
    //  Do nothing? Warning? Add it?
    if (!gdeffun) continue;
    
    //case def is found in decl, but decl already has a def
    //  Error, two definitiosn for linking
    if (gdeffun && gdeclfun && (gdeffun != gdeclfun) ) {
      Error e;
      e.message("Cannot link a def if there is already a def! (duplicate symbol)");
      e.message("  Cannot link : " + defns->getName() + "." + gdefname);
      e.message("  To : " + declns->getName() + "." + gdefname);
      error(e);
      return true;
    }

    assert(gdeffun && !gdeclfun); // Internal check
    //case def is found in decl, decl has no def
    //  Perfect, Add def to decl
    gdecl->addDef(gdeffun);
  }

  //TODO do modules as well
  return false;
}

Type* Context::Any() { return cache->newAny(); }
Type* Context::BitIn() { return cache->newBitIn(); }
Type* Context::BitOut() { return cache->newBitOut(); }
Type* Context::Array(uint n, Type* t) { return cache->newArray(n,t);}
Type* Context::Record(RecordParams rp) { return cache->newRecord(rp); }
Type* Context::TypeGenInst(TypeGen* tgd, Args* args) { return cache->newTypeGenInst(tgd,args); }
Type* Context::Flip(Type* t) { return t->getFlipped();}

RecordParams* Context::newRecordParams() {
  RecordParams* record_param = new RecordParams();
  recordParamsList.push_back(record_param);
  return record_param;
}
Params* Context::newParams() {
  Params* gp = new Params();
  genparamsList.push_back(gp);
  return gp;
}

Instance** Context::newInstanceArray(int size) {
  Instance** arr = (Instance**) malloc(sizeof(Instance*) * size);
  instanceArrays.push_back(arr);
  return arr;
}

// Connection* Context::newConnectionArray(int size) {
//   Connection* arr = (Connection*) malloc(sizeof(Connection) * size);
//   connectionArrays.push_back(arr);
//   return arr;
// }

Wireable** Context::newWireableArray(int size) {
  Wireable** arr = (Wireable**) malloc(sizeof(Wireable*) * size);
  wireableArrays.push_back(arr);
  return arr;
}
    Arg* int2Arg(int i);
    Arg* string2Arg(string s);
    Arg* type2Arg(Type* t);
    
    int arg2Int(Arg* g);
    string arg2String(Arg* g);
    Type* arg2Type(Arg* g);
    
    Args* args(unordered_map<string,Arg*> args=unordered_map<string,Arg*>());   
    Args* configArgs(unordered_map<string,Arg*> args=unordered_map<string,Arg*>());   

Arg* Context::int2Arg(int i) { 
  Arg* ga = new GenInt(i); 
  genargList.push_back(ga);
  return ga;
}
Arg* Context::string2Arg(string s) { 
  Arg* ga = new GenString(s); 
  genargList.push_back(ga);
  return ga;
}
Arg* Context::type2Arg(Type* t) { 
  Arg* ga = new GenType(t); 
  genargList.push_back(ga);
  return ga;
}
int Context::arg2Int(Arg* g) { return ((GenInt*) g)->i; }
string Context::arg2String(Arg* g) { return ((GenString*) g)->str; }
Type* Context::arg2Type(Arg* g) { return ((GenType*) g)->t; }

Args* Context::args(unordered_map<string,Arg*> args) {
  Args* ga = new Args(this,args);
  genargsList.push_back(ga);
  return ga;
}

Context* newContext() {
  Context* m = new Context();
  return m;
}

void deleteContext(Context* m) { 
  delete m;
}


} //CoreIR namespace

#endif //CONTEXT_CPP_
