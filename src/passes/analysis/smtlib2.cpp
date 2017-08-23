#include "coreir.h"
#include "coreir-passes/analysis/smtmodule.hpp"
#include "coreir-passes/analysis/smtlib2.h"

using namespace CoreIR;

namespace {

string NEXT_PF = "_N";
  
string getNext(string var) {return var + NEXT_PF; }
  
string unary_op(string op, string in, string out) {
  return "(assert (= (" + op + " " + in + ") " + out + "))";
}
  
string binary_op(string op, string in1, string in2, string out) {
  return "(assert (= (" + op + " " + in1 + " " + in2 + ") " + out + "))";
}
  
string SmtBVVarDec(SmtBVVar w) { return "(declare-fun " + w.getName() + " () (_ BitVec " + w.dimstr() + "))"; }

string SMTAssign(Connection con) {
  Wireable* left = con.first->getType()->getDir()==Type::DK_In ? con.first : con.second;
  Wireable* right = left==con.first ? con.second : con.first;
  SmtBVVar vleft(left);
  SmtBVVar vright(right);
  return "  (= " + vleft.getName() + " " + vright.getName() + ")";
}
 
string SMTAnd(SmtBVVar in1, SmtBVVar in2, SmtBVVar out) {
  // (in1 & in2 = out) & (in1' & in2' = out')
  string op = "bvand";
  string current = binary_op(op, in1.getName(), in2.getName(), out.getName());
  string next = binary_op(op, getNext(in1.getName()), getNext(in2.getName()), getNext(out.getName()));
  return current + next;
}

string SMTOr(SmtBVVar in1, SmtBVVar in2, SmtBVVar out) {
  // (in1 | in2 = out) & (in1' | in2' = out')
  string op = "bvor";
  string current = binary_op(op, in1.getName(), in2.getName(), out.getName());
  string next = binary_op(op, getNext(in1.getName()), getNext(in2.getName()), getNext(out.getName()));
  return current + next;
}

string SMTNot(SmtBVVar in, SmtBVVar out) {
  // (!in = out) & (!in' = out')
  string op = "bvnot";
  string current = unary_op("op", in.getName(), out.getName());
  string next = unary_op("op", getNext(in.getName()), getNext(out.getName()));
  return current + next;
}

string SMTReg(SmtBVVar in, SmtBVVar clk, SmtBVVar out) {
  // (!clk & clk') -> (out' = in)
  return "(assert (=> ((bvand (bvnot " + clk.getName() + ") " + getNext(clk.getName()) + ")) (= " + getNext(out.getName()) + " " + in.getName() + ")))";
}


  
}

std::string Passes::SmtLib2::ID = "smtlib2";
bool Passes::SmtLib2::runOnInstanceGraphNode(InstanceGraphNode& node) {
  
  //Create a new SMTmodule for this node
  Instantiable* i = node.getInstantiable();
  if (auto g = dyn_cast<Generator>(i)) {
    this->modMap[i] = new SMTModule(g);
    this->external.insert(i);
    return false;
  }
  Module* m = cast<Module>(i);
  SMTModule* smod = new SMTModule(m);
  modMap[i] = smod;
  if (!m->hasDef()) {
    this->external.insert(i);
    return false;
  }

  ModuleDef* def = m->getDef();
  
  string tab = "  ";
  for (auto imap : def->getInstances()) {
    string iname = imap.first;
    Instance* inst = imap.second;
    Instantiable* iref = imap.second->getInstantiableRef();
    smod->addStmt("  ; Wire declarations for instance '" + imap.first + "' (Module "+ iref->getName() + ")");
    for (auto rmap : cast<RecordType>(imap.second->getType())->getRecord()) {
      smod->addStmt(SmtBVVarDec(SmtBVVar(iname+"_"+rmap.first,rmap.second)));
    }
    ASSERT(modMap.count(iref),"DEBUG ME: Missing iref");
    smod->addStmt(modMap[iref]->toInstanceString(inst));
  }

  smod->addStmt("  ; All the connections");
  for (auto con : def->getConnections()) {
    smod->addStmt(SMTAssign(con));
  }
  
  return false;
}

void Passes::SmtLib2::writeToStream(std::ostream& os) {
  
  for (auto ext : external) {
    os << modMap[ext]->toCommentString() << endl;
  }
  os << endl;
  for (auto mmap : modMap) {
    if (external.count(mmap.first)==0) { 
      os << mmap.second->toString() << endl;
    }
  }


}
