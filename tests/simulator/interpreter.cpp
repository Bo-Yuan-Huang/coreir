#define CATCH_CONFIG_MAIN

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

    TypeGen* counterTypeGen = global->newTypeGen(
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
      state.setValue("counter$ri.out", BitVec(pcWidth, 400));
      state.setValue("self.en", BitVec(1, 1));
      state.setClock("self.clk", 0, 1);

      state.execute();

      REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 401));

      state.setValue("counter$ri.out", BitVec(pcWidth, 400));
      state.setValue("self.en", BitVec(1, 1));
      state.setClock("self.clk", 0, 1);
  
      state.execute();

      cout << "Output = " << state.getBitVec("self.counterOut") << endl;

      REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 24));

      state.setValue("counter$ri.out", BitVec(pcWidth, 400));
      state.setValue("self.en", BitVec(1, 1));
      state.setClock("self.clk", 1, 0);
  
      state.execute();

      cout << "Output = " << state.getBitVec("self.counterOut") << endl;

      REQUIRE(state.getBitVec("self.counterOut") == BitVec(pcWidth, 400));

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

  }

}
