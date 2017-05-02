#include "coreir.h"
#include "coreir-lib/stdlib.h"
#include "coreir-pass/passes.hpp"

using namespace CoreIR;

int main() {
  
  // New context
  Context* c = newContext();
  
  Namespace* g = c->getGlobal();
  
  Namespace* stdlib = CoreIRLoadLibrary_stdlib(c);
  
  Module* const16 = stdlib->getModule("const_16");
 
  // Define Module Type
  Type* mType = c->Record({
      {"out",c->Array(16,c->Bit())}
  });

  Module* mod = g->newModuleDecl("mod",mType);
  ModuleDef* def = mod->newModuleDef();
    Wireable* self = def->sel("self");
    Wireable* i0 = def->addInstance("c0",const16,{{"width",c->int2Arg(16)}});
    def->wire(i0->sel("out"),self->sel("out"));
    def->wire(i0->sel("out"),self->sel("out"));
    def->wire(self->sel("out"),i0->sel("out"));
    def->wire(self->sel("out")->sel(0),i0->sel("out")->sel(2));
  
  mod->setDef(def);
  cout << "Checkign Errors 1" << endl;
  c->checkerrors();
  
  //Verify that the number of connections is only 2. 
  assert(def->getConnections().size() == 2);

  deleteContext(c);
  
  return 0;
}
