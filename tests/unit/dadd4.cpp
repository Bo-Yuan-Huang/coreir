#include "coreir.h"
#include "coreir-lib/stdlib.h"
#include "coreir-pass/passes.hpp"

using namespace CoreIR;

int main() {
  uint n = 32;
  
  // New context
  Context* c = newContext();
  
  Namespace* g = c->getGlobal();
  
  Namespace* stdlib = CoreIRLoadLibrary_stdlib(c);
  
  //Declare add2 Generator
  Generator* add2 = stdlib->getGenerator("add2");
  assert(add2);
  
  // Define Add4 Module
  Type* add4Type = c->Record({
      {"in",c->Array(4,c->Array(n,c->BitIn()))},
      {"out",c->Array(n,c->Bit())}
  });

  Module* add4_n = g->newModuleDecl("Add4",add4Type);
  ModuleDef* def = add4_n->newModuleDef();
    Wireable* self = def->sel("self");
    Wireable* add_00 = def->addInstance("add00",add2,{{"width",c->argInt(n)}});
    Wireable* add_01 = def->addInstance("add01",add2,{{"width",c->argInt(n)}});
    Wireable* add_1 = def->addInstance("add1",add2,{{"width",c->argInt(n)}});
    
    def->wire(self->sel("in")->sel(0),add_00->sel("in0"));
    def->wire(self->sel("in")->sel(1),add_00->sel("in1"));
    def->wire(self->sel("in")->sel(2),add_01->sel("in0"));
    def->wire(self->sel("in")->sel(3),add_01->sel("in1"));

    def->wire(add_00->sel("out"),add_1->sel("in0"));
    def->wire(add_01->sel("out"),add_1->sel("in1"));

    def->wire(add_1->sel("out"),self->sel("out"));
  add4_n->setDef(def);
  cout << "Checkign Errors 1" << endl;
  c->checkerrors();
  add4_n->print();
  
  bool err = false;
  cout << "Typechecking!" << endl;
  typecheck(c,add4_n,&err);
  if (err) c->die();

  auto selprint = [](SelectPath path) {
    string s = "";
    for (auto i : path) s = s + i + ".";
    return s;
  };

  DirectedModule dm(add4_n);
  cout << "Directed connectins" << endl;
  for (auto c : dm.getConnections()) {
    cout << selprint(c.getSrc()) << "->" << selprint(c.getSnk()) << endl;
  }
 
  deleteContext(c);
  
  return 0;
}
