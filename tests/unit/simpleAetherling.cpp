#include "coreir.h"
#include "coreir/libs/aetherlinglib.h"

using namespace std;
using namespace CoreIR;

int main() {
    Context* c = newContext();
    CoreIRLoadLibrary_aetherlinglib(c);

    //Type of module 
    Type* oneInOneOutGenType = c->Record({
            {"in",c->Array(16,c->BitIn())},
            {"out",c->Array(16,c->Bit())}
        });
    Module* testModule = c->getGlobal()->newModuleDecl("testModule",oneInOneOutGenType);
    ModuleDef* testDef = test->newModuleDef();

    /* creating the mulBy2 that the mapN will parallelize */
    Module* mulBy2 = c->getGlobal()->newModuleDecl("mulBy2", oneInOneOutGenType);
    ModuleDef* mulBy2Def = test->newModuleDef();

    uint width = 16;
    string constModule = Aetherling_addCoreIRConstantModule(c, def, width, Const::make(c, width, 4));
    mulBy2Def->addInstance("mul", "coreir.mul");
    mulBy2Def->connect("self.in", "mul.in0");
    mulBy2Def->connect(constModule + ".out", "mul.in1");
    mulBy2Def->connect("mul.out", "self.out")

    Values mapNParams({
            {"width", Const::make(c, width)},
            {"parallelOperators", Const::make(c, 4)},
            {"operator", Const::make(c, mulBy2)}
        });
                      
    // ignoring last argumen to addIstance as no module parameters    
    def->addInstance("mapMul", "aetherlinglib.mapN", mapNParams);

    def->connect("self.in","mapMul.in");
    def->connect("mapMul.out","self.out");

    c->runPasses({"rungenerators", "verifyconnectivity"});
  
    testDef->print();
    cout << testDef->toString() << endl;
  
    deleteContext(c);
    return 0;
}
