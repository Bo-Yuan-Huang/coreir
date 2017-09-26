#include "coreir.h"

using namespace std;
using namespace CoreIR;


void checker(Module* m) {
  ModuleDef* def = m->getDef();
  Wireable* w = def->sel("i0");
  cout << w->toString() << endl;
  Instance* i0 = cast<Instance>(def->sel("i0"));
  cout << i0->toString() << endl;
  assert(i0->getGenArgs().count("ga") && i0->getGenArgs().at("ga")->get<int>()==6);
  assert(i0->getGenArgs().count("gb") && i0->getGenArgs().at("gb")->get<int>()==7);
  assert(i0->getConfigArgs().count("ca") && i0->getConfigArgs().at("ca")->get<int>()==11);
  assert(i0->getConfigArgs().count("cb") && i0->getConfigArgs().at("cb")->get<int>()==12);
  
  Instance* i1 = cast<Instance>(def->sel("i1"));
  assert(i1->getGenArgs().count("ga") && i1->getGenArgs().at("ga")->get<int>()==5);
  assert(i1->getGenArgs().count("gb") && i1->getGenArgs().at("gb")->get<int>()==7);
  assert(i1->getConfigArgs().count("ca") && i1->getConfigArgs().at("ca")->get<int>()==10);
  assert(i1->getConfigArgs().count("cb") && i1->getConfigArgs().at("cb")->get<int>()==12);
}

int main() {
  
  // New context
  Context* c = newContext();
  
  //create generator with some defaults
  Namespace* g = c->getGlobal();
  
  //Declare a TypeGenerator (in global) for addN
  g->newTypeGen(
    "default_type", //name for the typegen
    {{"ga",AINT},{"gb",AINT}}, //generater parameters
    [](Context* c, Args args) { //Function to compute type
      return c->Record();
    }
  );


  Generator* d = g->newGeneratorDecl("defaults",g->getTypeGen("default_type"),{{"ga",AINT},{"gb",AINT}},{{"ca",AINT},{"cb",AINT}});
  //Set defaults for ga and ca
  d->addDefaultGenArgs({{"ga",Const(5)}});
  d->addDefaultConfigArgs({{"ca",Const(10)}});

  Module* tester = g->newModuleDecl("Tester",c->Record());
  ModuleDef* def = tester->newModuleDef();
    def->addInstance("i0",d,{{"ga",Const(6)},{"gb",Const(7)}},{{"ca",Const(11)},{"cb",Const(12)}});
    def->addInstance("i1",d,{{"gb",Const(7)}},{{"cb",Const(12)}});
  tester->setDef(def);
  tester->print();
 
  //run assertion checks
  checker(tester);
  
  deleteContext(c);
}


