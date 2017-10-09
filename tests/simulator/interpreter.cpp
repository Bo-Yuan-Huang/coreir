#include "catch.hpp"

#include "coreir.h"
#include "coreir-passes/analysis/pass_sim.h"
#include "coreir/passes/transform/rungenerators.h"
#include "coreir/simulator/interpreter.h"

#include "fuzzing.hpp"

#include "../src/simulator/output.hpp"
#include "../src/simulator/sim.hpp"
#include "../src/simulator/utils.hpp"

#include <iostream>

using namespace CoreIR;
using namespace CoreIR::Passes;
using namespace std;

namespace CoreIR {

  void addCounter(Context* c, Namespace* global) {

    Params counterParams({{"width",AINT}});

    TypeGen* counterTypeGen =
      global->newTypeGen(
			 "CounterTypeGen", //name of typegen
			 counterParams, //Params required for typegen
			 [](Context* c, Args args) { //lambda for generating the type
			   //Arg* widthArg = args.at("width"); //Checking for valid args is already done for you
			   uint width = args.at("width")->get<int>(); //widthArg->get<int>(); //get function to extract the arg value.
			   return c->Record({
			       {"en",c->BitIn()}, 
				 {"out",c->Array(width,c->Bit())}, //Note: Array is parameterized by width now
				   {"clk",c->Named("coreir.clkIn")},
				     });
			 } //end lambda
			 ); //end newTypeGen

    ASSERT(global->hasTypeGen("CounterTypeGen"),"Can check for typegens in namespaces");


    Generator* counter = global->newGeneratorDecl("counter",counterTypeGen,counterParams);

    counter->setGeneratorDefFromFun([](ModuleDef* def,Context* c, Type* t, Args args) {

	uint width = args.at("width")->get<int>();
      
	Args wArg({{"width", Const(width)}});
	def->addInstance("ai","coreir.add",wArg);
	def->addInstance("ci","coreir.const",wArg,{{"value", Const(1)}});

	def->addInstance("ri","coreir.reg",{{"width", Const(width)},{"en", Const(true)}});
    

	def->connect("self.clk","ri.clk");
	def->connect("self.en","ri.en");
	def->connect("ci.out","ai.in0");
	def->connect("ai.out","ri.in");
	def->connect("ri.out","ai.in1");
	def->connect("ri.out","self.out");
      }); //end lambda, end function
  
  }
  
  TEST_CASE("Interpret simulator graphs") {

    // New context
    Context* c = newContext();
  
    Namespace* g = c->getGlobal();

    SECTION("andr") {
      uint n = 11;

      Generator* andr = c->getGenerator("coreir.andr");
      Type* andrNType = c->Record({
	  {"in", c->Array(n, c->BitIn())},
	    {"out", c->Bit()}
	});

      Module* andrN = g->newModuleDecl("andrN", andrNType);
      ModuleDef* def = andrN->newModuleDef();

      Wireable* self = def->sel("self");
      Wireable* andr0 = def->addInstance("andr0", andr, {{"width", Const(n)}});
    
      def->connect(self->sel("in"), andr0->sel("in"));
      def->connect(andr0->sel("out"),self->sel("out"));

      andrN->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      SimulatorState state(andrN);

      SECTION("Bitvector that is all ones") {
	state.setValue("self.in", BitVec(n, "11111111111"));

	state.execute();

	REQUIRE(state.getBitVec("self.out") == BitVec(1, 1));
      }

      SECTION("Bitvector that is not all ones") {
	state.setValue("self.in", BitVec(n, "11011101111"));

	state.execute();

	REQUIRE(state.getBitVec("self.out") == BitVec(1, 0));
      }
      
    }

    SECTION("And 4") {
      cout << "32 bit and 4" << endl;

      uint n = 32;
  
      Generator* and2 = c->getGenerator("coreir.and");

      // Define And4 Module
      Type* and4Type = c->Record({
	  {"in0",c->Array(n,c->BitIn())},
	    {"in1",c->Array(n,c->BitIn())},
	      {"in2",c->Array(n,c->BitIn())},
		{"in3",c->Array(n,c->BitIn())},
		  {"out",c->Array(n,c->Bit())}
	});

      Module* and4_n = g->newModuleDecl("And4",and4Type);
      ModuleDef* def = and4_n->newModuleDef();
      Wireable* self = def->sel("self");
      Wireable* and_00 = def->addInstance("and00",and2,{{"width", Const(n)}});
      Wireable* and_01 = def->addInstance("and01",and2,{{"width", Const(n)}});
      Wireable* and_1 = def->addInstance("and1",and2,{{"width", Const(n)}});
    
      def->connect(self->sel("in0"), and_00->sel("in0"));
      def->connect(self->sel("in1"), and_00->sel("in1"));
      def->connect(self->sel("in2"), and_01->sel("in0"));
      def->connect(self->sel("in3"), and_01->sel("in1"));

      def->connect(and_00->sel("out"),and_1->sel("in0"));
      def->connect(and_01->sel("out"),and_1->sel("in1"));

      def->connect(and_1->sel("out"),self->sel("out"));
      and4_n->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      // How to initialize and track values in the interpreter?
      // I think the right way would be to set select values, but
      // that does not deal with registers and memory that need
      // intermediate values
      SimulatorState state(and4_n);
      state.setValue(self->sel("in0"), BitVec(n, 20));
      state.setValue(self->sel("in1"), BitVec(n, 0));
      state.setValue(self->sel("in2"), BitVec(n, 9));
      state.setValue(self->sel("in3"), BitVec(n, 31));

      state.execute();

      BitVec bv(n, 20 & 0 & 9 & 31);

      cout << "BV     = " << bv << endl;
      cout << "output = " << state.getBitVec(self->sel("out")) << endl;

      REQUIRE(state.getBitVec(self->sel("out")) == bv);
    }

    SECTION("Or 4") {
      cout << "23 bit or 4" << endl;

      uint n = 23;
  
      Generator* or2 = c->getGenerator("coreir.or");

      // Define Or4 Module
      Type* or4Type = c->Record({
	  {"in0",c->Array(n,c->BitIn())},
	    {"in1",c->Array(n,c->BitIn())},
	      {"in2",c->Array(n,c->BitIn())},
		{"in3",c->Array(n,c->BitIn())},
		  {"outval",c->Array(n,c->Bit())}
	});

      Module* or4_n = g->newModuleDecl("Or4",or4Type);
      ModuleDef* def = or4_n->newModuleDef();
      Wireable* self = def->sel("self");
      Wireable* or_00 = def->addInstance("or00",or2,{{"width", Const(n)}});
      Wireable* or_01 = def->addInstance("or01",or2,{{"width", Const(n)}});
      Wireable* or_1 = def->addInstance("or1",or2,{{"width", Const(n)}});
    
      def->connect(self->sel("in0"), or_00->sel("in0"));
      def->connect(self->sel("in1"), or_00->sel("in1"));
      def->connect(self->sel("in2"), or_01->sel("in0"));
      def->connect(self->sel("in3"), or_01->sel("in1"));

      def->connect(or_00->sel("out"), or_1->sel("in0"));
      def->connect(or_01->sel("out"), or_1->sel("in1"));

      def->connect(or_1->sel("out"), self->sel("outval"));
      or4_n->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      // How to initialize or track values in the interpreter?
      // I think the right way would be to set select values, but
      // that does not deal with registers or memory that need
      // intermediate values
      SimulatorState state(or4_n);
      //state.setValue(self->sel("in0"), BitVec(n, 20));
      state.setValue("self.in0", BitVec(n, 20));
      state.setValue(self->sel("in1"), BitVec(n, 0));
      state.setValue(self->sel("in2"), BitVec(n, 9));
      state.setValue(self->sel("in3"), BitVec(n, 31));

      state.execute();

      BitVec bv(n, 20 | 0 | 9 | 31);

      cout << "BV     = " << bv << endl;
      cout << "output = " << state.getBitVec(self->sel("outval")) << endl;

      REQUIRE(state.getBitVec(self->sel("outval")) == bv);
    }

    SECTION("Counter") {

      addCounter(c, g);

      uint pcWidth = 17;
      Type* counterTestType =
	c->Record({
	    {"en", c->BitIn()},
	      {"clk", c->Named("coreir.clkIn")},
		{"counterOut", c->Array(pcWidth, c->Bit())}});

      Module* counterTest = g->newModuleDecl("counterMod", counterTestType);
      ModuleDef* def = counterTest->newModuleDef();

      def->addInstance("counter", "global.counter", {{"width", Const(pcWidth)}});

      def->connect("self.en", "counter.en");
      def->connect("self.clk", "counter.clk");
      def->connect("counter.out", "self.counterOut");

      counterTest->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      // Inline increment
      inlineInstance(def->getInstances()["counter"]);

      SimulatorState state(counterTest);

      SECTION("Test regiser defaults") {
	REQUIRE(state.getBitVec("counter$ri.out") == BitVec(pcWidth, 0));
      }

      SECTION("Count from zero, enable set") {

	state.setValue("counter$ri.out", BitVec(pcWidth, 0));
	state.setValue("self.en", BitVec(1, 1));
	state.setClock("self.clk", 0, 1);

	state.execute();

	REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 0));

	state.execute();

	REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 1));

	state.execute();

	REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 2));

      }

      SECTION("Counting with clock changes, enable set") {

	state.setValue("counter$ri.out", BitVec(pcWidth, 400));
	state.setValue("self.en", BitVec(1, 1));
	state.setClock("self.clk", 0, 1);
  
	state.execute();

	cout << "Output = " << state.getBitVec("self.counterOut") << endl;

	SECTION("Value is 400 after first tick at 400") {
	  REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 400));
	}

	state.setClock("self.clk", 1, 0);

	state.execute();

	ClockValue* clkVal = toClock(state.getValue("self.clk"));

	cout << "last clock = " << (int) clkVal->lastValue() << endl;
	cout << "curr clock = " << (int) clkVal->value() << endl;

	cout << "Output = " << state.getBitVec("self.counterOut") << endl;

	SECTION("Value is 401 after second tick") {
	  REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 401));
	}

	state.setClock("self.clk", 0, 1);

	state.execute();

	SECTION("Value is still 401") {
	  REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 401));
	}

	state.setClock("self.clk", 1, 0);

	state.execute();

	SECTION("Value is now 402") {
	  REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 402));
	}
	
      }

      SECTION("Enable on") {
	state.setValue("counter$ri.out", BitVec(pcWidth, 400));
	state.setValue("self.en", BitVec(1, 1));
	state.setClock("self.clk", 1, 0);
  
	state.execute();

	cout << "Output = " << state.getBitVec("self.counterOut") << endl;

	REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 400));
      }

      SECTION("Setting watchpoint") {
	state.setValue("counter$ri.out", BitVec(pcWidth, 0));
	state.setClock("self.clk", 1, 0);
	state.setValue("self.en", BitVec(1, 1));

	state.setWatchPoint("self.counterOut", BitVec(pcWidth, 10));
	state.setMainClock("self.clk");

	// auto states = state.getCircStates();
	// for (auto& state : states) {
	//   cout << "State" << endl;
	//   for (auto& val : state.valMap) {
	//     cout << (val.first)->toString() << " --> " << (val.second) << endl;
	//   }
	// }
	
	state.run();

	// states = state.getCircStates();
	// for (auto& state : states) {
	//   cout << "State" << endl;
	//   for (auto& val : state.valMap) {
	//     cout << (val.first)->toString() << " --> " << (val.second) << endl;
	//   }
	// }
	
	SECTION("Stop at watchpoint") {
	  REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 10));
	}

	// Should rewind rewind 1 clock cycle or one half clock?
	SECTION("Rewinding state by 1 clock cycle") {

	  cout << "state index before rewind = " << state.getStateIndex() << endl;
	  state.rewind(2);
	  cout << "state index after rewind  = " << state.getStateIndex() << endl;

	  REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 9));
	}

	SECTION("Rewinding to before the start of the simulation is an error") {

	  bool rewind = state.rewind(22);

	  REQUIRE(!rewind);
	}
      }

    }

    SECTION("Test bit vector addition") {
      cout << "23 bit or 4" << endl;

      uint n = 76;
  
      Generator* add2 = c->getGenerator("coreir.add");

      // Define Add2 Module
      Type* add2Type = c->Record({
	  {"in0",c->Array(n,c->BitIn())},
	    {"in1",c->Array(n,c->BitIn())},
	      {"outval",c->Array(n,c->Bit())}
	});

      Module* add2_n = g->newModuleDecl("Add2",add2Type);
      ModuleDef* def = add2_n->newModuleDef();
      Wireable* self = def->sel("self");
      Wireable* or_00 = def->addInstance("or00",add2,{{"width", Const(n)}});

      def->connect(self->sel("in0"), or_00->sel("in0"));
      def->connect(self->sel("in1"), or_00->sel("in1"));
      def->connect(or_00->sel("out"), self->sel("outval"));

      add2_n->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      // How to initialize or track values in the interpreter?
      // I think the right way would be to set select values, but
      // that does not deal with registers or memory that need
      // intermediate values
      SimulatorState state(add2_n);
      //state.setValue(self->sel("in0"), BitVec(n, 20));
      state.setValue("self.in0", BitVec(n, 20));
      state.setValue("self.in1", BitVec(n, 1234));

      state.execute();

      BitVec bv(n, 20 + 1234);

      cout << "BV     = " << bv << endl;
      cout << "output = " << state.getBitVec(self->sel("outval")) << endl;

      REQUIRE(state.getBitVec(self->sel("outval")) == bv);
      
    }

    SECTION("Multiplexer") {
      uint width = 30;

      Type* muxType =
	c->Record({
	    {"in0", c->Array(width, c->BitIn())},
	      {"in1", c->Array(width, c->BitIn())},
		{"sel", c->BitIn()},
		  {"out", c->Array(width, c->Bit())}
	  });

      Module* muxTest = g->newModuleDecl("muxTest", muxType);
      ModuleDef* def = muxTest->newModuleDef();

      Wireable* mux = def->addInstance("m0", "coreir.mux", {{"width", Const(width)}});

      def->connect("self.in0", "m0.in0");
      def->connect("self.in1", "m0.in1");
      def->connect("self.sel", "m0.sel");
      def->connect("m0.out", "self.out");

      muxTest->setDef(def);

      SECTION("Select input 1") {
	SimulatorState state(muxTest);
	state.setValue("self.in0", BitVec(width, 1234123));
	state.setValue("self.in1", BitVec(width, 987));
	state.setValue("self.sel", BitVec(1, 1));

	state.execute();

	REQUIRE(state.getBitVec("self.out") == BitVec(width, 987));
      }

      SECTION("Select input 0") {
	SimulatorState state(muxTest);
	state.setValue("self.in0", BitVec(width, 1234123));
	state.setValue("self.in1", BitVec(width, 987));
	state.setValue("self.sel", BitVec(1, 0));

	state.execute();

	REQUIRE(state.getBitVec("self.out") == BitVec(width, 1234123));
      }
      
    }

    SECTION("Increment circuit") {
      uint width = 3;

      Type* incTestType =
	c->Record({{"incIn", c->Array(width, c->BitIn())},
	      {"incOut", c->Array(width, c->Bit())}});

      Module* incTest = g->newModuleDecl("incMod", incTestType);
      ModuleDef* def = incTest->newModuleDef();
      
      Args wArg({{"width", Const(width)}});
      def->addInstance("ai","coreir.add",wArg);
      def->addInstance("ci","coreir.const",wArg,{{"value", Const(1)}});
    
      def->connect("ci.out","ai.in0");
      def->connect("self.incIn","ai.in1");
      def->connect("ai.out","self.incOut");

      incTest->setDef(def);

      SimulatorState state(incTest);
      state.setValue("self.incIn", BitVec(width, 0));

      state.execute();

      REQUIRE(state.getBitVec("self.incOut") == BitVec(width, 1));
      
      
    }

    SECTION("Memory") {
      uint width = 20;
      uint depth = 4;
      uint index = 2;

      Type* memoryType = c->Record({
      	  {"clk", c->Named("coreir.clkIn")},
      	    {"write_data", c->BitIn()->Arr(width)},
      	      {"write_addr", c->BitIn()->Arr(index)},
      		{"write_en", c->BitIn()},
      		  {"read_data", c->Bit()->Arr(width)},
      		    {"read_addr", c->BitIn()->Arr(index)}
      	});

      
      Module* memory = c->getGlobal()->newModuleDecl("memory0", memoryType);
      ModuleDef* def = memory->newModuleDef();

      def->addInstance("m0",
      		       "coreir.mem",
      		       {{"width", Const(width)},{"depth", Const(depth)}},
      		       {{"init", Const("0")}});

      def->connect("self.clk", "m0.clk");
      def->connect("self.write_en", "m0.wen");
      def->connect("self.write_data", "m0.wdata");
      def->connect("self.write_addr", "m0.waddr");
      def->connect("self.read_data", "m0.rdata");
      def->connect("self.read_addr", "m0.raddr");

      memory->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(c->getGlobal());

      SimulatorState state(memory);

      SECTION("Memory default is zero") {
	REQUIRE(state.getMemory("m0", BitVec(index, 0)) == BitVec(width, 0));
      }

      SECTION("rdata default is zero") {
	REQUIRE(state.getBitVec("m0.rdata") == BitVec(width, 0));
      }
      
      state.setMemory("m0", BitVec(index, 0), BitVec(width, 0));
      state.setMemory("m0", BitVec(index, 1), BitVec(width, 1));
      state.setMemory("m0", BitVec(index, 2), BitVec(width, 2));
      state.setMemory("m0", BitVec(index, 3), BitVec(width, 3));

      SECTION("Setting memory manually") {
	REQUIRE(state.getMemory("m0", BitVec(index, 2)) == BitVec(width, 2));
      }

      SECTION("Re-setting memory manually") {
	state.setMemory("m0", BitVec(index, 3), BitVec(width, 23));

	REQUIRE(state.getMemory("m0", BitVec(index, 3)) == BitVec(width, 23));
      }

      SECTION("Write to address zero") {
      	state.setClock("self.clk", 0, 1);
      	state.setValue("self.write_en", BitVec(1, 1));
      	state.setValue("self.write_addr", BitVec(index, 0));
      	state.setValue("self.write_data", BitVec(width, 23));
      	state.setValue("self.read_addr", BitVec(index, 0));

      	state.execute();

      	REQUIRE(state.getBitVec("self.read_data") == BitVec(width, 0));

      	state.execute();

      	REQUIRE(state.getBitVec("self.read_data") == BitVec(width, 23));

      	state.execute();

      	cout << "read data later = " << state.getBitVec("self.read_data") << endl;

	REQUIRE(state.getBitVec("self.read_data") == BitVec(width, 23));
	
      }
      
    }

    SECTION("Slice") {
      uint inLen = 7;
      uint lo = 2;
      uint hi = 5;
      uint outLen = hi - lo;

      Type* sliceType = c->Record({
	  {"in", c->Array(inLen, c->BitIn())},
	    {"out", c->Array(outLen, c->Bit())}
	});

      Module* sliceM = c->getGlobal()->newModuleDecl("sliceM", sliceType);
      ModuleDef* def = sliceM->newModuleDef();

      def->addInstance("sl",
		       "coreir.slice",
		       {{"width", Const(inLen)},
			   {"lo", Const(lo)},
			     {"hi", Const(hi)}});

      def->connect("self.in", "sl.in");
      def->connect("sl.out", "self.out");

      sliceM->setDef(def);

      SimulatorState state(sliceM);
      state.setValue("self.in", BitVec(inLen, "1011010"));

      state.execute();

      REQUIRE(state.getBitVec("self.out") == BitVec(outLen, "110"));
    }

    SECTION("Concat") {
      uint inLen0 = 3;
      uint inLen1 = 5;
      uint outLen = inLen0 + inLen1;

      Type* concatType = c->Record({
	  {"in0", c->BitIn()->Arr(inLen0)},
	    {"in1", c->BitIn()->Arr(inLen1)},
	      {"out", c->Bit()->Arr(outLen)}
	});

      Module* concatM = c->getGlobal()->newModuleDecl("concatM", concatType);
      ModuleDef* def = concatM->newModuleDef();

      def->addInstance("cm",
		       "coreir.concat",
		       {{"width0", Const(inLen0)},
			   {"width1", Const(inLen1)}});

      def->connect("self.in0", "cm.in0");
      def->connect("self.in1", "cm.in1");
      def->connect("cm.out", "self.out");

      concatM->setDef(def);

      SimulatorState state(concatM);
      state.setValue("self.in0", BitVec(3, "111"));
      state.setValue("self.in1", BitVec(5, "00000"));

      state.execute();

      REQUIRE(state.getBitVec("self.out") == BitVec(8, "00000111"));
    }

    deleteContext(c);
  }

}
