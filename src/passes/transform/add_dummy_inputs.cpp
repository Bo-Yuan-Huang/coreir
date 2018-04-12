#include "coreir.h"
#include "coreir/passes/transform/add_dummy_inputs.h"

using namespace std;
using namespace CoreIR;

string Passes::AddDummyInputs::ID = "add-dummy-inputs";

void connectToDummy(//CoreIR::Instance* const next,
                    //const std::string& field,
                    const std::string& constName,
                    CoreIR::Select* const sel,
                    CoreIR::ModuleDef* const def,
                    CoreIR::Context* const c) {

  if (isBitArray(*(sel->getType()))) {
    ArrayType* arrTp = cast<ArrayType>(sel->getType());
    int width = arrTp->getLen();

    auto replaceConst =
      def->addInstance(constName,
                       "coreir.const",
                       {{"width", Const::make(c, width)}},
                       {{"value", Const::make(c, BitVector(width, 0))}});

    def->connect(replaceConst->sel("out"), sel);
  } else {
    if (!isBitType(*(sel->getType()))) {
      cout << "ERROR: " << sel->toString() << " has type " << sel->getType()->toString() << endl;
    }
    assert(isBitType(*(sel->getType())));


    auto replaceConst =
      def->addInstance(constName,
                       "corebit.const",
                       {{"value", Const::make(c, false)}});

    def->connect(replaceConst->sel("out"), sel);
            
  }

}

bool Passes::AddDummyInputs::runOnModule(Module* m) {
  if (!m->hasDef()) {
    return false;
  }

  auto def = m->getDef();
  auto c = def->getContext();

  bool addedDummy = false;

  auto self = def->sel("self");
  for (auto field : def->getType()->getFields()) {
    Select* sel = self->sel(field);

    if (sel->getType()->getDir() == Type::DirKind::DK_In) {

      if (getSourceSelects(sel).size() == 0) {

        string constName = "self_" + field;
        connectToDummy(constName, sel, def, c);
          
      }
        
    }

  }
  
  auto instances = def->getInstances();
  set<Instance*> toCheck;
  for (auto instR : instances) {
    toCheck.insert(instR.second);
  }

  //cout << "Running on module " << m->toString() << endl;

  while (toCheck.size() > 0) {
    Instance* next = *begin(toCheck);
    Module* mr = next->getModuleRef();
    RecordType* tp = mr->getType();
    //cout << "\tChecking instance " << next->toString() << endl;
    for (auto field : tp->getFields()) {
      Select* sel = next->sel(field);

      if (sel->getType()->getDir() == Type::DirKind::DK_In) {

        if (getSourceSelects(sel).size() == 0) {
          //cout << "\t\t" << sel->toString() << endl;
          //assert(isBitArray(*(sel->getType())) || isBitType(*(sel->getType())));

          string constName = next->toString() + "_" + field + "_const_in";
          connectToDummy(constName, sel, def, c);
          
        }
        
      }
    }

    toCheck.erase(next);
  }
  
  return addedDummy;
}
