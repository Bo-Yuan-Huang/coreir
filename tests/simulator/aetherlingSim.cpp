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
            Module* mulBy2 = c->getGlobal()->newModuleDecl("mulBy2", oneInOneOutGenType);
            ModuleDef* mulBy2Def = test->newModuleDef();

            uint width = 16;
            string constModule = Aetherling_addCoreIRConstantModule(c, def, width, Const::make(c, width, 4));
            mulBy2Def->addInstance("mul", "coreir.mul");
            mulBy2Def->connect("self.in", "mul.in0");
            mulBy2Def->connect(constModule + ".out", "mul.in1");
            mulBy2Def->connect("mul.out", "self.out")

            Values mapnModArgs = {
                {"width", Const::make(c, width)},
                {"parallelOperators", Const::make(c, parallelOperators)},
                {"operator", Const::make(c, mulBy2)}
            };
            
            string mapNName = "map" + to_string(parallelOperators);
            def->addInstance(mapNName, "aetherlinglib.mapN", mapnModArgs);
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
            c->runPasses({"rungenerators", "flatten", "flattentypes"});
            mainModule->print();
            
            SimulatorState state(mainModule);
            state.execute();

            for (int i = 0; i < parallelOperators; i++) {
                REQUIRE(state.getBitVec("self.out_" + to_string(i)) == BitVector(width, i*constInput));
            }
                    
        }

        deleteContext(c);
    }
}
