#include "catch.hpp"

#include "coreir.h"
#include "coreir-passes/analysis/pass_sim.h"
#include "coreir/passes/transform/rungenerators.h"

#include "fuzzing.hpp"

#include "../src/simulator/output.hpp"
#include "../src/simulator/sim.hpp"
#include "../src/simulator/utils.hpp"

#include <iostream>

using namespace CoreIR;
using namespace CoreIR::Passes;
using namespace std;

namespace CoreIR {

  TEST_CASE("Combinational logic simulation") {

    // New context
    Context* c = newContext();
  
    Namespace* g = c->getGlobal();
    
    SECTION("32 bit add 4") {
      uint n = 32;
  
      Generator* add2 = c->getGenerator("coreir.add");

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
    
      def->connect(self->sel("in")->sel(0),add_00->sel("in0"));
      def->connect(self->sel("in")->sel(1),add_00->sel("in1"));
      def->connect(self->sel("in")->sel(2),add_01->sel("in0"));
      def->connect(self->sel("in")->sel(3),add_01->sel("in1"));

      def->connect(add_00->sel("out"),add_1->sel("in0"));
      def->connect(add_01->sel("out"),add_1->sel("in1"));

      def->connect(add_1->sel("out"),self->sel("out"));
      add4_n->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph gr;
      buildOrderedGraph(add4_n, gr);
      deque<vdisc> topoOrder = topologicalSort(gr);

      SECTION("Checking graph size") {
	REQUIRE(numVertices(gr) == 5);
      }

      SECTION("Checking mask elimination") {
	eliminateMasks(topoOrder, gr);

	REQUIRE(numMasksNeeded(gr) == 0);
      }

      SECTION("Sorting and compiling code") {
	auto str = printCode(topoOrder, gr, add4_n);
	int s = compileCode(str, "./gencode/add4.cpp");

	cout << "Command result = " << s << endl;

	saveToFile(g, "add4.json");

	REQUIRE(s == 0);

	// Building verilog example
	s = buildVerilator(add4_n, g);

	REQUIRE(s == 0);
      }
      
    }

    SECTION("6 bit signed remainder 3 operations") {
      uint n = 6;
  
      Generator* srem2 = c->getGenerator("coreir.srem");

      // Define Srem4 Module
      Type* srem4Type = c->Record({
	  {"in",c->Array(3, c->Array(n,c->BitIn()))},
	    {"out",c->Array(n, c->Bit())}
	});

      Module* srem4_n = g->newModuleDecl("Srem4", srem4Type);
      ModuleDef* def = srem4_n->newModuleDef();
      Wireable* self = def->sel("self");
      Wireable* srem_00 = def->addInstance("srem00", srem2, {{"width",c->argInt(n)}});
      Wireable* srem_01 = def->addInstance("srem01", srem2, {{"width",c->argInt(n)}});
      Wireable* srem_1 = def->addInstance("srem1", srem2, {{"width",c->argInt(n)}});

      def->connect(self->sel("in")->sel(0),srem_00->sel("in0"));
      def->connect(self->sel("in")->sel(1),srem_00->sel("in1"));

      def->connect(self->sel("in")->sel(2),srem_01->sel("in0"));
      def->connect(srem_00->sel("out"), srem_01->sel("in1"));

      def->connect(srem_00->sel("out"),srem_1->sel("in0"));
      def->connect(srem_01->sel("out"),srem_1->sel("in1"));

      def->connect(srem_1->sel("out"),self->sel("out"));
      srem4_n->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph gr;
      buildOrderedGraph(srem4_n, gr);
      
      deque<vdisc> topoOrder = topologicalSort(gr);

      //auto str = printCode(topoOrder, gr, srem4_n);
      int s = compileCodeAndRun(topoOrder,
				gr,
				srem4_n,
				"./gencode/srem4",
				"./gencode/test_srem4.cpp");

      REQUIRE(s == 0);
      
    }
    
    SECTION("64 bit subtract") {
      uint n = 64;
  
      Generator* sub2 = c->getGenerator("coreir.sub");

      Type* sub4Type = c->Record({
	  {"in",c->Array(4,c->Array(n,c->BitIn()))},
	    {"out",c->Array(n,c->Bit())}
	});

      Module* sub4_n = g->newModuleDecl("Sub4",sub4Type);
      ModuleDef* def = sub4_n->newModuleDef();
      Wireable* self = def->sel("self");
      Wireable* sub_00 = def->addInstance("sub00",sub2,{{"width",c->argInt(n)}});
      Wireable* sub_01 = def->addInstance("sub01",sub2,{{"width",c->argInt(n)}});
      Wireable* sub_1 = def->addInstance("sub1",sub2,{{"width",c->argInt(n)}});
    
      def->connect(self->sel("in")->sel(0),sub_00->sel("in0"));
      def->connect(self->sel("in")->sel(1),sub_00->sel("in1"));
      def->connect(self->sel("in")->sel(2),sub_01->sel("in0"));
      def->connect(self->sel("in")->sel(3),sub_01->sel("in1"));

      def->connect(sub_00->sel("out"),sub_1->sel("in0"));
      def->connect(sub_01->sel("out"),sub_1->sel("in1"));

      def->connect(sub_1->sel("out"),self->sel("out"));
      sub4_n->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(sub4_n, g);

      SECTION("Checking graph size") {
	REQUIRE(numVertices(g) == 5);
      }
      
      deque<vdisc> topoOrder = topologicalSort(g);

      auto str = printCode(topoOrder, g, sub4_n);
      cout << "CODE STRING" << endl;
      cout << str << endl;

      string outFile = "./gencode/sub4.cpp";      
      int s = compileCode(str, outFile);

      REQUIRE(s == 0);
      
    }

    SECTION("Multiply 8 bits") {

      uint n = 8;
  
      Generator* mul2 = c->getGenerator("coreir.mul");

      Type* mul2Type = c->Record({
	  {"in",c->Array(2,c->Array(n,c->BitIn()))},
	    {"out",c->Array(n,c->Bit())}
	});

      Module* mul_n = g->newModuleDecl("Mul4", mul2Type);
      ModuleDef* def = mul_n->newModuleDef();

      Wireable* self = def->sel("self");
      Wireable* mul = def->addInstance("mul1", mul2, {{"width", c->argInt(n)}});
    
      def->connect(self->sel("in")->sel(0), mul->sel("in0"));
      def->connect(self->sel("in")->sel(1), mul->sel("in1"));

      def->connect(mul->sel("out"), self->sel("out"));
      mul_n->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(mul_n, g);

      SECTION("Checking graph size") {
	REQUIRE(numVertices(g) == 3);
      }
      
      deque<vdisc> topoOrder = topologicalSort(g);

      auto str = printCode(topoOrder, g, mul_n);
      cout << "CODE STRING" << endl;
      cout << str << endl;
      int s = compileCode(str, "./gencode/mul2.cpp");

      cout << "Command result = " << s << endl;

      REQUIRE(s == 0);

    }

    SECTION("One 37 bit logical and") {
      uint n = 37;

      Generator* andG = c->getGenerator("coreir.and");

      Type* andType = c->Record({
	  {"input", c->Array(2, c->Array(n, c->BitIn()))},
	    {"output", c->Array(n, c->Bit())}
	});

      Module* andM = g->newModuleDecl("and37", andType);

      ModuleDef* def = andM->newModuleDef();

      Wireable* self = def->sel("self");
      Wireable* and0 = def->addInstance("and0", andG, {{"width", c->argInt(n)}});

      def->connect("self.input.0", "and0.in0");
      def->connect("self.input.1", "and0.in1");
      def->connect("and0.out", "self.output");

      andM->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(andM, g);

      SECTION("Checking graph size") {
      	REQUIRE(numVertices(g) == 3);
      }

      deque<vdisc> topoOrder = topologicalSort(g);

      auto str = printCode(topoOrder, g, andM);
      cout << "CODE STRING" << endl;
      cout << str << endl;

      string outFile = "./gencode/and37.cpp";

      int s = compileCodeAndRun(topoOrder,
				g,
				andM,
				"./gencode/and37",
				"./gencode/test_and37.cpp");

      cout << "Test result = " << s << endl;

      REQUIRE(s == 0);
      
    }

    SECTION("One 63 bit addition") {
      uint n = 63;

      Generator* addG = c->getGenerator("coreir.add");

      Type* addType = c->Record({
	  {"input", c->Array(2, c->Array(n, c->BitIn()))},
	    {"output", c->Array(n, c->Bit())}
	});

      Module* addM = g->newModuleDecl("add63", addType);

      ModuleDef* def = addM->newModuleDef();

      Wireable* self = def->sel("self");
      Wireable* add0 = def->addInstance("add0", addG, {{"width", c->argInt(n)}});

      def->connect("self.input.0", "add0.in0");
      def->connect("self.input.1", "add0.in1");
      def->connect("add0.out", "self.output");

      addM->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(addM, g);

      SECTION("Checking graph size") {
      	REQUIRE(numVertices(g) == 3);
      }

      deque<vdisc> topoOrder = topologicalSort(g);

      auto str = printCode(topoOrder, g, addM);
      cout << "CODE STRING" << endl;
      cout << str << endl;

      string outFile = "./gencode/add63";
      int s = compileCodeAndRun(topoOrder,
				g,
				addM,
				outFile,
				"./gencode/test_add63.cpp");

      REQUIRE(s == 0);

    }

    SECTION("One 2 bit not") {
      uint n = 2;
  
      Generator* neg = c->getGenerator("coreir.not");

      Type* neg2Type = c->Record({
	  {"A",    c->Array(n,c->BitIn())},
	    {"res", c->Array(n,c->Bit())}
	});

      Module* neg_n = g->newModuleDecl("neg_16", neg2Type);

      ModuleDef* def = neg_n->newModuleDef();

      Wireable* self = def->sel("self");
      Wireable* neg0 = def->addInstance("neg0", neg, {{"width", c->argInt(n)}});

      def->connect(self->sel("A"), neg0->sel("in"));

      def->connect(neg0->sel("out"), self->sel("res"));

      neg_n->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      Type* t = neg_n->getType();
      cout << "Module type = " << t->toString() << endl;
      
      NGraph gr;
      buildOrderedGraph(neg_n, gr);
      deque<vdisc> topoOrder = topologicalSort(gr);

      SECTION("Checking mask elimination") {
	eliminateMasks(topoOrder, gr);

	REQUIRE(numMasksNeeded(gr) == 1);
      }

      SECTION("Compiling code") {
	auto str = printCode(topoOrder, gr, neg_n);
	cout << "CODE STRING" << endl;
	cout << str << endl;

	string outFile = "./gencode/neg2";
	int s = compileCodeAndRun(topoOrder,
				  gr,
				  neg_n,
				  outFile,
				  "./gencode/test_neg2.cpp");

	REQUIRE(s == 0);

      }

      
    }
    
    SECTION("One 16 bit not") {
      uint n = 16;
  
      Generator* neg = c->getGenerator("coreir.not");

      Type* neg2Type = c->Record({
	  {"A",    c->Array(n,c->BitIn())},
	    {"res", c->Array(n,c->Bit())}
	});

      Module* neg_n = g->newModuleDecl("neg_16", neg2Type);

      ModuleDef* def = neg_n->newModuleDef();

      Wireable* self = def->sel("self");
      Wireable* neg0 = def->addInstance("neg0", neg, {{"width", c->argInt(n)}});

      def->connect(self->sel("A"), neg0->sel("in"));

      def->connect(neg0->sel("out"), self->sel("res"));

      neg_n->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      Type* t = neg_n->getType();
      cout << "Module type = " << t->toString() << endl;
      
      NGraph g;
      buildOrderedGraph(neg_n, g);

      SECTION("Checking graph size") {
      	REQUIRE(numVertices(g) == 3);
      }

      deque<vdisc> topoOrder = topologicalSort(g);

      auto str = printCode(topoOrder, g, neg_n);
      cout << "CODE STRING" << endl;
      cout << str << endl;

      int s = compileCode(str, "./gencode/neg16.cpp");

      cout << "Command result = " << s << endl;

      REQUIRE(s == 0);
    }
    
    SECTION("Two 16 bit nots") {
      uint n = 16;
  
      Generator* neg = c->getGenerator("coreir.not");

      Type* neg2Type = c->Record({
	  {"in",    c->Array(2, c->Array(n,c->BitIn()))},
	    {"out", c->Array(2, c->Array(n,c->Bit()))}
	});

      Module* neg_n = g->newModuleDecl("two_negs", neg2Type);

      ModuleDef* def = neg_n->newModuleDef();

      Wireable* self = def->sel("self");
      Wireable* neg0 = def->addInstance("neg0", neg, {{"width", c->argInt(n)}});
      Wireable* neg1 = def->addInstance("neg1", neg, {{"width", c->argInt(n)}});

      def->connect(self->sel("in")->sel(0), neg0->sel("in"));
      def->connect(self->sel("in")->sel(1), neg1->sel("in"));

      def->connect(neg0->sel("out"), self->sel("out")->sel(0));
      def->connect(neg1->sel("out"), self->sel("out")->sel(1));

      neg_n->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      Type* t = neg_n->getType();
      cout << "Module type = " << t->toString() << endl;
      
      NGraph g;
      buildOrderedGraph(neg_n, g);

      SECTION("Checking graph size") {
	REQUIRE(numVertices(g) == 4);
      }

      deque<vdisc> topoOrder = topologicalSort(g);

      auto str = printCode(topoOrder, g, neg_n);
      cout << "CODE STRING" << endl;
      cout << str << endl;

      int s = compileCode(str, "./gencode/two_negs.cpp");

      cout << "Command result = " << s << endl;

      REQUIRE(s == 0);
      
    }

    SECTION("Add 2 by 3 64 bit matrices") {
      uint n = 64;
  
      Generator* add = c->getGenerator("coreir.add");

      Type* addMatsType = c->Record({
	  {"A",    c->Array(2, c->Array(3, c->Array(n,c->BitIn()))) },
	    {"B",    c->Array(2, c->Array(3, c->Array(n,c->BitIn()))) },
	      {"out", c->Array(2, c->Array(3, c->Array(n,c->Bit()))) }
	});

      Module* addM = g->newModuleDecl("two_negs", addMatsType);

      ModuleDef* def = addM->newModuleDef();

      Wireable* self = def->sel("self");
      for (int i = 0; i < 2; i++) {
	for (int j = 0; j < 3; j++) {
	  string str = "add" + to_string(i) + "_" + to_string(j);
	  Wireable* addInst = def->addInstance(str, add, {{"width", c->argInt(n)}});

	  def->connect(self->sel("A")->sel(i)->sel(j), addInst->sel("in0"));
	  def->connect(self->sel("B")->sel(i)->sel(j), addInst->sel("in1"));

	  def->connect(addInst->sel("out"), self->sel("out")->sel(i)->sel(j));
	}
      }

      addM->setDef(def);
      
	      
      RunGenerators rg;
      rg.runOnNamespace(g);

      Type* t = addM->getType();
      cout << "Module type = " << t->toString() << endl;
      
      NGraph g;
      buildOrderedGraph(addM, g);

      deque<vdisc> topoOrder = topologicalSort(g);

      auto str = printCode(topoOrder, g, addM);
      cout << "CODE STRING" << endl;
      cout << str << endl;

      string outFile = "./gencode/mat2_3_add.cpp";
      int s = compileCodeAndRun(topoOrder,
				g,
				addM,
				"./gencode/mat2_3_add",
				"./gencode/test_mat2_3_add.cpp");

      REQUIRE(s == 0);
    }

    SECTION("Equality comparison on 54 bits") {
      uint n = 54;
  
      Generator* eq = c->getGenerator("coreir.eq");

      Type* eqType = c->Record({
	  {"A",    c->Array(2, c->Array(n, c->BitIn())) },
	    {"out", c->Bit() }
	});

      Module* eqM = g->newModuleDecl("equality_test", eqType);

      ModuleDef* def = eqM->newModuleDef();

      Wireable* self = def->sel("self");
      Wireable* eq0 = def->addInstance("eq0", eq, {{"width", c->argInt(n)}});

      def->connect("self.A.0", "eq0.in0");
      def->connect("self.A.1", "eq0.in1");
      def->connect(eq0->sel("out"), self->sel("out"));

      eqM->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(eqM, g);

      deque<vdisc> topoOrder = topologicalSort(g);


      auto str = printCode(topoOrder, g, eqM);
      cout << "CODE STRING" << endl;
      cout << str << endl;

      compileCode(str, "./gencode/eq54.cpp");
    }

    SECTION("sle on 7 bits") {
      uint n = 7;
  
      Generator* sle = c->getGenerator("coreir.sle");

      Type* sleType = c->Record({
	  {"A",    c->Array(2, c->Array(n, c->BitIn())) },
	    {"out", c->Bit() }
	});

      Module* sleM = g->newModuleDecl("sle_test", sleType);

      ModuleDef* def = sleM->newModuleDef();

      Wireable* self = def->sel("self");
      Wireable* sle0 = def->addInstance("sle0", sle, {{"width", c->argInt(n)}});

      def->connect("self.A.0", "sle0.in0");
      def->connect("self.A.1", "sle0.in1");
      def->connect(sle0->sel("out"), self->sel("out"));

      sleM->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(sleM, g);

      deque<vdisc> topoOrder = topologicalSort(g);


      auto str = printCode(topoOrder, g, sleM);
      cout << "CODE STRING" << endl;
      cout << str << endl;

      int s = compileCodeAndRun(topoOrder,
				g,
				sleM,
				"./gencode/sle7", "./gencode/test_sle7.cpp");
      REQUIRE(s == 0);

    }

    SECTION("Multiplexer test") {
      uint n = 8;
  
      Type* muxType = c->Record({
	  {"A",    c->Array(2, c->Array(n, c->BitIn())) },
	    {"sel", c->BitIn()},
	    {"out", c->Array(n, c->Bit()) }
	});

      Module* muxM = g->newModuleDecl("muxM", muxType);
      ModuleDef* def = muxM->newModuleDef();

      Generator* mux = c->getGenerator("coreir.mux");

      Wireable* self = def->sel("self");
      Wireable* mux0 = def->addInstance("mux0", mux, {{"width", c->argInt(n)}});

      def->connect("self.A.0", "mux0.in0");
      def->connect("self.A.1", "mux0.in1");
      def->connect("self.sel", "mux0.sel");
      def->connect(mux0->sel("out"), self->sel("out"));

      muxM->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(muxM, g);

      deque<vdisc> topoOrder = topologicalSort(g);

      string outFile = "gencode/mux8";

      int s = compileCodeAndRun(topoOrder,
				g,
				muxM,
				outFile,
				"gencode/test_mux8.cpp");

      REQUIRE(s == 0);
      
    }

    SECTION("32 bit dshl test") {
      uint n = 32;
  
      Type* dshlType = c->Record({
	  {"A",    c->Array(2, c->Array(n, c->BitIn())) },
	    {"out", c->Array(n, c->Bit()) }
	});

      Module* dshlM = g->newModuleDecl("dshlM", dshlType);
      ModuleDef* def = dshlM->newModuleDef();

      Generator* dshl = c->getGenerator("coreir.dshl");

      Wireable* self = def->sel("self");
      Wireable* dshl0 = def->addInstance("dshl0", dshl, {{"width", c->argInt(n)}});

      def->connect("self.A.0", "dshl0.in0");
      def->connect("self.A.1", "dshl0.in1");
      def->connect(dshl0->sel("out"), self->sel("out"));

      dshlM->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(dshlM, g);

      deque<vdisc> topoOrder = topologicalSort(g);

      string outFile = "gencode/dshl32";

      int s = compileCodeAndRun(topoOrder,
				g,
				dshlM,
				outFile,
				"gencode/test_dshl32.cpp");

      REQUIRE(s == 0);
      
    }

    SECTION("60 bit dashr test") {
      uint n = 60;
  
      Type* dashrType = c->Record({
	  {"A",    c->Array(2, c->Array(n, c->BitIn())) },
	    {"out", c->Array(n, c->Bit()) }
	});

      Module* dashrM = g->newModuleDecl("dashrM", dashrType);
      ModuleDef* def = dashrM->newModuleDef();

      Generator* dashr = c->getGenerator("coreir.dashr");

      Wireable* self = def->sel("self");
      Wireable* dashr0 = def->addInstance("dashr0", dashr, {{"width", c->argInt(n)}});

      def->connect("self.A.0", "dashr0.in0");
      def->connect("self.A.1", "dashr0.in1");
      def->connect(dashr0->sel("out"), self->sel("out"));

      dashrM->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(dashrM, g);

      deque<vdisc> topoOrder = topologicalSort(g);

      string outFile = "gencode/dashr60";

      int s = compileCodeAndRun(topoOrder,
				g,
				dashrM,
				outFile,
				"gencode/test_dashr60.cpp");

      REQUIRE(s == 0);
      
    }

    SECTION("5 bit dlshr test") {
      uint n = 5;
  
      Type* dlshrType = c->Record({
	  {"A",    c->Array(2, c->Array(n, c->BitIn())) },
	    {"out", c->Array(n, c->Bit()) }
	});

      Module* dlshrM = g->newModuleDecl("dlshrM", dlshrType);
      ModuleDef* def = dlshrM->newModuleDef();

      Generator* dlshr = c->getGenerator("coreir.dlshr");

      Wireable* self = def->sel("self");
      Wireable* dlshr0 = def->addInstance("dlshr0", dlshr, {{"width", c->argInt(n)}});

      def->connect("self.A.0", "dlshr0.in0");
      def->connect("self.A.1", "dlshr0.in1");
      def->connect(dlshr0->sel("out"), self->sel("out"));

      dlshrM->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(dlshrM, g);

      deque<vdisc> topoOrder = topologicalSort(g);

      string outFile = "gencode/dlshr5";

      int s = compileCodeAndRun(topoOrder,
				g,
				dlshrM,
				outFile,
				"gencode/test_dlshr5.cpp");

      REQUIRE(s == 0);
    }

    SECTION("Unsigned 27 bit division") {
      uint n = 27;
  
      Type* udivType = c->Record({
	  {"A",    c->Array(2, c->Array(n, c->BitIn())) },
	    {"out", c->Array(n, c->Bit()) }
	});

      Module* udivM = g->newModuleDecl("udivM", udivType);
      ModuleDef* def = udivM->newModuleDef();

      Generator* udiv = c->getGenerator("coreir.udiv");

      Wireable* self = def->sel("self");
      Wireable* udiv0 = def->addInstance("udiv0", udiv, {{"width", c->argInt(n)}});

      def->connect("self.A.0", "udiv0.in0");
      def->connect("self.A.1", "udiv0.in1");
      def->connect(udiv0->sel("out"), self->sel("out"));

      udivM->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(udivM, g);

      deque<vdisc> topoOrder = topologicalSort(g);

      string outFile = "gencode/udiv27";

      int s = compileCodeAndRun(topoOrder,
				g,
				udivM,
				outFile,
				"gencode/test_udiv27.cpp");

      REQUIRE(s == 0);
      
    }

    SECTION("Unsigned 13 bit remainder") {
      uint n = 13;
  
      Type* uremType = c->Record({
	  {"A",    c->Array(2, c->Array(n, c->BitIn())) },
	    {"out", c->Array(n, c->Bit()) }
	});

      Module* uremM = g->newModuleDecl("uremM", uremType);
      ModuleDef* def = uremM->newModuleDef();

      Generator* urem = c->getGenerator("coreir.urem");

      Wireable* self = def->sel("self");
      Wireable* urem0 = def->addInstance("urem0", urem, {{"width", c->argInt(n)}});

      def->connect("self.A.0", "urem0.in0");
      def->connect("self.A.1", "urem0.in1");
      def->connect(urem0->sel("out"), self->sel("out"));

      uremM->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(uremM, g);

      deque<vdisc> topoOrder = topologicalSort(g);

      string outFile = "gencode/urem13";

      int s = compileCodeAndRun(topoOrder,
				g,
				uremM,
				outFile,
				"gencode/test_urem13.cpp");

      REQUIRE(s == 0);
      
    }

    SECTION("Signed 5 bit remainder") {
      uint n = 5;
  
      Type* sremType = c->Record({
	  {"A",    c->Array(2, c->Array(n, c->BitIn())) },
	    {"out", c->Array(n, c->Bit()) }
	});

      Module* sremM = g->newModuleDecl("sremM", sremType);
      ModuleDef* def = sremM->newModuleDef();

      Generator* srem = c->getGenerator("coreir.srem");

      Wireable* self = def->sel("self");
      Wireable* srem0 = def->addInstance("srem0", srem, {{"width", c->argInt(n)}});

      def->connect("self.A.0", "srem0.in0");
      def->connect("self.A.1", "srem0.in1");
      def->connect(srem0->sel("out"), self->sel("out"));

      sremM->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(sremM, g);

      deque<vdisc> topoOrder = topologicalSort(g);

      string outFile = "gencode/srem5";

      int s = compileCodeAndRun(topoOrder,
				g,
				sremM,
				outFile,
				"gencode/test_srem5.cpp");

      REQUIRE(s == 0);
      
    }

    SECTION("Signed 5 bit division") {
      uint n = 5;
  
      Type* sdivType = c->Record({
	  {"A",    c->Array(2, c->Array(n, c->BitIn())) },
	    {"out", c->Array(n, c->Bit()) }
	});

      Module* sdivM = g->newModuleDecl("sdivM", sdivType);
      ModuleDef* def = sdivM->newModuleDef();

      Generator* sdiv = c->getGenerator("coreir.sdiv");

      Wireable* self = def->sel("self");
      Wireable* sdiv0 = def->addInstance("sdiv0", sdiv, {{"width", c->argInt(n)}});

      def->connect("self.A.0", "sdiv0.in0");
      def->connect("self.A.1", "sdiv0.in1");
      def->connect(sdiv0->sel("out"), self->sel("out"));

      sdivM->setDef(def);

      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(sdivM, g);

      deque<vdisc> topoOrder = topologicalSort(g);

      string outFile = "gencode/sdiv5";

      int s = compileCodeAndRun(topoOrder,
				g,
				sdivM,
				outFile,
				"gencode/test_sdiv5.cpp");

      REQUIRE(s == 0);
      
    }
    
    SECTION("Circuit with module references") {

      cout << "loading" << endl;
      if (!loadFromFile(c, "main.json")) {
	cout << "Could not Load from json!!" << endl;
	c->die();
      }

      cout << "Loaded" << endl;

      Module* mainMod = c->getModule("global.main");

      
      RunGenerators rg;
      rg.runOnNamespace(g);

      NGraph g;
      buildOrderedGraph(mainMod, g);

      deque<vdisc> topoOrder = topologicalSort(g);

      auto str = printCode(topoOrder, g, mainMod);
      int s = compileCode(str, "./gencode/mainMod.cpp");

      cout << "Command result = " << s << endl;

      REQUIRE(s == 0);
      
    }

    deleteContext(c);

  }


}
