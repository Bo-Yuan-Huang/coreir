#include "coreir.h"
#include "coreir/passes/transform/cullgraph.h"

using namespace std;
using namespace CoreIR;

namespace {
void recurse(Module* m, set<Module*>& mused, set<Generator*>& gused) {
  if (m->isGenerated()) {
    gused.insert(m->getGenerator());
    mused.insert(m);
  }
  if (m->hasDef()) {
    mused.insert(m);
  }
  else {
    return;
  }
  for (auto ipair : m->getDef()->getInstances()) {
    recurse(ipair.second->getModuleRef(),mused,gused);
  }
}
}

string Passes::CullGraph::ID = "cullgraph";
bool Passes::CullGraph::runOnContext(Context* c) {
  //if (!c->hasTop()) return false;
  set<Module*> mused;
  set<Generator*> gused;
 
  for (auto npair: c->getNamespaces()) {
    for (auto mpair: npair.second->getModules()) {
      recurse(mpair.second,mused,gused);
    }
  }
  
  //Find a list of all used Modules and Generators
  set<GlobalValue*> toErase;
  for (auto npair : c->getNamespaces()) {
    for (auto gpair : npair.second->getGenerators()) {
      if (gused.count(gpair.second)==0) {
        toErase.insert(gpair.second);
      }
    }
    for (auto mpair : npair.second->getModules()) {
      if (mused.count(mpair.second)==0 && !mpair.second->isGenerated()) {
        toErase.insert(mpair.second);
      }
    }
  }
  set<GlobalValue*> genToErase;
  for (auto i : toErase) {
    if (auto m = dyn_cast<Module>(i)) {
      m->getNamespace()->eraseModule(m->getName());
    }
    else {
      genToErase.insert(i);
    }
  }
  for (auto i : genToErase) {
    auto g = cast<Generator>(i);
    g->getNamespace()->eraseGenerator(g->getName());
  }
  return toErase.size()>0;
  
}
