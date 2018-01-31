//#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "coreir.h"
#include "coreir/passes/transform/rungenerators.h"
#include "coreir/simulator/interpreter.h"
#include "coreir/libs/commonlib.h"
#include "coreir/libs/aetherlinglib.h"

#include <iostream>

using namespace CoreIR;
using namespace CoreIR::Passes;
using namespace std;

namespace CoreIR {

    TEST_CASE("Simulate mapN from aetherlinglib") {
        // New context
        Context* c = newContext();
        Namespace* g = c->getGlobal();

        SECTION("aetherlinglib mapN with 4 mul, 3 as constant, 16 bit width") {
            uint width = 16;
            uint parallelOperators = 4;
            uint constInput = 3;

            CoreIRLoadLibrary_commonlib(c);
            CoreIRLoadLibrary_aetherlinglib(c);

            // create the main module to run the test on the adder
            Type* mainModuleType = c->Record({
                    {"out", c->Bit()->Arr(width)->Arr(parallelOperators)}
                });
            Module* mainModule = c->getGlobal()->newModuleDecl("mainMapNMulTest", mainModuleType);
            ModuleDef* def = mainModule->newModuleDef();

            /* creating the mulBy2 that the mapN will parallelize */
            //Type of module 
            Type* oneInOneOutGenType = c->Record({
                    {"in",c->Array(16,c->BitIn())},
                    {"out",c->Array(16,c->Bit())}
                });
            Module* mulBy2 = c->getGlobal()->newModuleDecl("mulBy2", oneInOneOutGenType);
            ModuleDef* mulBy2Def = mulBy2->newModuleDef();

            string constModule = Aetherling_addCoreIRConstantModule(c, mulBy2Def, width, Const::make(c, width, 4));
            mulBy2Def->addInstance("mul", "coreir.mul", {{"width", Const::make(c, width)}});
            mulBy2Def->connect("self.in", "mul.in0");
            mulBy2Def->connect(constModule + ".out", "mul.in1");
            mulBy2Def->connect("mul.out", "self.out");

            Values mapnModArgs = {
                {"width", Const::make(c, width)},
                {"parallelOperators", Const::make(c, parallelOperators)},
                {"operator", Const::make(c, mulBy2)}
            };
            
            string mapNName = "map" + to_string(parallelOperators);
            Instance* mapN_mul = def->addInstance(mapNName, "aetherlinglib.mapN", mapnModArgs);
            // create different input for each operator
            for (int i = 0 ; i < parallelOperators; i++) {
                string constName = "constInput" + to_string(i);
                def->addInstance(
                    constName,
                    "coreir.const",
                    {{"width", Const::make(c, width)}},
                    {{"value", Const::make(c, width, i)}});

                def->connect(constName + ".out", mapNName + ".in." + to_string(i));
                def->connect(mapNName + ".out", "self.out");
                // safe version of wiring out: def->connect(mapNName + ".out." + to_string(i), "self.out." + to_string(i))
            }

            mainModule->setDef(def);
            mapN_mul->getModuleRef()->print();
            c->runPasses({"rungenerators", "flatten", "flattentypes"});
            mainModule->print();
            cout << "hi" << endl;
            mapN_mul->getModuleRef()->print();
            cout << endl;
                        
            SimulatorState state(mainModule);
            state.execute();

            for (int i = 0; i < parallelOperators; i++) {
                REQUIRE(state.getBitVec("self.out_" + to_string(i)) == BitVector(width, i*constInput));
            }
                    
        }

        deleteContext(c);
    }
}
